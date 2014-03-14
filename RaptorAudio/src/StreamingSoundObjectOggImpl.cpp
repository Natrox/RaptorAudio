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

using namespace Raptor;
using namespace Raptor::Audio;

StreamingSoundObjectOggImpl::StreamingSoundObjectOggImpl( const char* filePath, WaveoutDevice* wvOut, StreamingSoundObject* parent )
		:
StreamingSoundObjectImpl( filePath, 0, 0 ) 
{
	m_GlobalPosition = 0;
	m_File = fopen( filePath, "rb" );

	int error = 0;

	m_OggHandle = stb_vorbis_open_file( m_File, 0, &error, 0 );
	m_NumChannels = (unsigned int) stb_vorbis_get_info( m_OggHandle ).channels;
	m_BufferSize = (unsigned int) stb_vorbis_stream_length_in_samples( m_OggHandle );
	m_TotalSize = m_BufferSize;

	if ( m_BufferSize > wvOut->GetSampleRate() * 10 ) m_BufferSize = wvOut->GetSampleRate() * 10;
	if ( m_NumChannels > 2 ) m_NumChannels = 2;

	m_BufferChannels = (short**) malloc( sizeof( short* ) * m_NumChannels );
	m_BufferChannelsInterleaved = (short*) malloc( sizeof( short ) * m_BufferSize * 2 );

	for ( unsigned int i = 0; i < m_NumChannels; i++ )
	{
		m_BufferChannels[i] = (short*) malloc( sizeof( short ) * m_BufferSize );
	}

	m_Parent = parent;

	parent->SetAdvanceAmount( (double) stb_vorbis_get_info( m_OggHandle ).sample_rate / (double) wvOut->GetSampleRate() ); 
	parent->SetBufferSize( m_BufferSize );

	UpdateBuffer();
}

StreamingSoundObjectOggImpl::~StreamingSoundObjectOggImpl( void )
{
	stb_vorbis_close( m_OggHandle );

	free( m_BufferChannelsInterleaved );

	for ( unsigned int b = 0; b < m_NumChannels; b++ )
		free( m_BufferChannels[b] );

	fclose( m_File );
}

short StreamingSoundObjectOggImpl::GetCurrentSample( unsigned int num )
{
	unsigned int origNum = num;
	if ( num >= m_NumChannels ) num = m_NumChannels - 1;

	double position = m_GlobalPosition;

	double frac = position - (double) ( (int) position );

	short val1 = m_BufferChannels[num][ ( (int) position ) % m_BufferSize ];

	int nextPos = (int) ( position + 1 ) % m_BufferSize;

	short val2 = m_BufferChannels[num][ nextPos ];

	return (short) ( ( ( ( 1.0 - frac ) * val1 ) + (short) ( frac * val2 )  ) * m_Parent->GetVolume() );
}

const short* StreamingSoundObjectOggImpl::GetChannelPtr( unsigned int num )
{
	if ( num >= m_NumChannels ) num = m_NumChannels - 1;

	return m_BufferChannels[num];
}

bool StreamingSoundObjectOggImpl::UpdateBuffer( void )
{
	int samplesGotten = stb_vorbis_get_samples_short_interleaved( m_OggHandle, m_NumChannels, m_BufferChannelsInterleaved, m_BufferSize * m_NumChannels ); 

	if ( samplesGotten == 0 && !m_Parent->GetLooping() )
	{
		return false;
	}

	if ( samplesGotten == 0 && m_Parent->GetLooping() )
	{
		stb_vorbis_seek_start( m_OggHandle );
		stb_vorbis_get_samples_short_interleaved( m_OggHandle, m_NumChannels, m_BufferChannelsInterleaved, m_BufferSize * m_NumChannels ); 
	}

	for ( unsigned int i = 0; i < m_BufferSize; i++ )
	{
		m_BufferChannels[0][i] = m_BufferChannelsInterleaved[i*m_NumChannels+0];
		if ( m_NumChannels > 1 ) m_BufferChannels[1][i] = m_BufferChannelsInterleaved[i*m_NumChannels+1];
	}

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
		remain = UpdateBuffer();
	}

	if ( m_Parent->GetPosition() >= (double) m_TotalSize )
	{
		m_Parent->SetPosition( m_Parent->GetPosition() - (double) m_TotalSize );
	}

	if ( remain ) return SoundObjectResults::SOUND_OBJECT_PLAYING;
	else return SoundObjectResults::SOUND_OBJECT_DONE;
}
