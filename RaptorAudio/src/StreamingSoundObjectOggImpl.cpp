/*
	Copyright (c) 2014 Sam Hardeman

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/

#include "StreamingSoundObjectOggImpl.h"
#include "StreamingSoundObject.h"

#include "DSPChain.h"

#include <iomanip> 
#include <string>
#include <sstream>
#include "OggDecoder.h"

using namespace Raptor;
using namespace Raptor::Audio;

StreamingSoundObjectOggImpl::StreamingSoundObjectOggImpl( StreamingSoundObject* parent )
		:
StreamingSoundObjectImpl( 0 ) 
{
	m_GlobalPosition = 0;
	m_CurrentBuffer = 0;
	m_LoopDone = false;

	int error = 0;

	switch ( parent->m_AudioSource->GetType() )
	{
	case AudioOrigins::AUDIO_ORIGIN_FILE:
		m_OggHandle = stb_vorbis_open_file( (FILE*) parent->m_AudioSource->GetPtrData(), 0, &error, 0 );
		break;

	case AudioOrigins::AUDIO_ORIGIN_OPENMEMORY_POINT:
	case AudioOrigins::AUDIO_ORIGIN_OPENMEMORY:
		m_OggHandle = stb_vorbis_open_memory( (unsigned char*) parent->m_AudioSource->GetPtrData(), (int) parent->m_AudioSource->GetLength(), &error, 0 );
		break;
	};

	m_NumChannels = (unsigned int) stb_vorbis_get_info( m_OggHandle ).channels;
	m_BufferSize = (unsigned int) stb_vorbis_stream_length_in_samples( m_OggHandle );
	m_TotalSize = m_BufferSize;

	WaveoutDevice* wvOut = SoundMixer::GetMixer()->GetWaveOut();

	if ( m_BufferSize > wvOut->GetSampleRate() * 10 ) m_BufferSize = wvOut->GetSampleRate() * 10;
	if ( m_NumChannels > 2 ) m_NumChannels = 2;

	m_BufferSize /= 2;

	for ( unsigned int i = 0; i < m_NumChannels; i++ )
	{
		m_OggBuffers[0].ob_BufferPtrs[i] = new short[ m_BufferSize + 2 ];
		m_OggBuffers[1].ob_BufferPtrs[i] = new short[ m_BufferSize + 2 ];
	}

	m_OggBuffers[0].ob_BufferSize = m_BufferSize;
	m_OggBuffers[1].ob_BufferSize = m_BufferSize;

	m_Parent = parent;

	parent->SetAdvanceAmount( (double) stb_vorbis_get_info( m_OggHandle ).sample_rate / (double) wvOut->GetSampleRate() ); 
	parent->SetBufferSize( m_BufferSize );

	for ( unsigned int i = 0; i < 2; i++ )
	{
		OggDecoderRequest request;

		request.odr_BufferSize = m_BufferSize + 1;
		request.odr_DstBuffer1 = m_OggBuffers[i].ob_BufferPtrs[0];
		request.odr_DstBuffer2 = m_OggBuffers[i].ob_BufferPtrs[1];

		request.odr_Vorbis = m_OggHandle;
		request.odr_LoopDone = &m_LoopDone;

		OggDecoder::GetSingleton()->DecodeOggNow( request );
	}
}

StreamingSoundObjectOggImpl::~StreamingSoundObjectOggImpl( void )
{
	stb_vorbis_close( m_OggHandle );
}

short StreamingSoundObjectOggImpl::GetCurrentSample( unsigned int num )
{
	unsigned int origNum = num;
	if ( num >= m_NumChannels ) num = m_NumChannels - 1;

	double position = m_GlobalPosition;

	double frac = position - (double) ( (int) position );

	short val1 = m_OggBuffers[m_CurrentBuffer].ob_BufferPtrs[num][ ( (int) position ) % m_BufferSize ];

	int nextPos = (int) ( position + 1 );
	if ( nextPos >= (int) m_BufferSize ) return val1;

	short val2 = m_OggBuffers[m_CurrentBuffer].ob_BufferPtrs[num][ nextPos ];

	return (short) ( ( ( ( 1.0 - frac ) * val1 ) + (short) ( frac * val2 )  ) * m_Parent->GetVolume() );
}

const short* StreamingSoundObjectOggImpl::GetChannelPtr( unsigned int num )
{
	if ( num >= m_NumChannels ) num = m_NumChannels - 1;

	return m_OggBuffers[m_CurrentBuffer].ob_BufferPtrs[num];
}

bool StreamingSoundObjectOggImpl::UpdateBuffer( void )
{
	if ( m_LoopDone == true && m_Parent->GetLooping() )
	{
		m_LoopDone = false;
	}

	if ( m_LoopDone == false )
	{
		OggDecoderRequest request;

		request.odr_BufferSize = m_BufferSize + 1;
		request.odr_DstBuffer1 = m_OggBuffers[m_CurrentBuffer].ob_BufferPtrs[0];
		request.odr_DstBuffer2 = m_OggBuffers[m_CurrentBuffer].ob_BufferPtrs[1];

		request.odr_Vorbis = m_OggHandle;
		request.odr_LoopDone = &m_LoopDone;

		OggDecoder::GetSingleton()->DecodeOgg( request );
	}

	m_CurrentBuffer += 1;
	m_CurrentBuffer &= 1;

	return true;
}

SoundObjectResults::SoundObjectResult StreamingSoundObjectOggImpl::AdvancePosition( void )
{
	m_Parent->SetPosition( m_Parent->GetPosition() + m_Parent->GetAdvanceAmount() * m_Parent->GetPitchShift() );
	m_GlobalPosition += m_Parent->GetAdvanceAmount() * m_Parent->GetPitchShift();

	bool remain = true;

	if ( m_GlobalPosition >= (double) m_BufferSize )
	{
		m_GlobalPosition -= (double) m_BufferSize;
		UpdateBuffer();
	}

	if ( m_Parent->GetPosition() >= (double) m_TotalSize )
	{
		m_Parent->SetPosition( m_Parent->GetPosition() - (double) m_TotalSize );

		if ( !m_Parent->GetLooping() )
		{
			remain = false;
		}

		if ( remain && m_Parent->m_Properties->sop_Shared != 0 && m_Parent->GetLooping() )
		{
			if ( m_Parent->m_Properties->sop_Shared->sp_DSPChain != 0 )
			{
				m_Parent->m_Properties->sop_Shared->sp_DSPChain->PerformRandomizePlay();
				m_Parent->m_Properties->sop_Shared->sp_DSPChain->PerformPerPlayProcessing( m_Parent->m_Properties );
			}
		}
	}

	if ( remain ) return SoundObjectResults::SOUND_OBJECT_PLAYING;
	else return SoundObjectResults::SOUND_OBJECT_DONE;
}
