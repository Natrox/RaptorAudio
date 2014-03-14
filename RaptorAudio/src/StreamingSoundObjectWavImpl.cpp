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

#include "StreamingSoundObjectWavImpl.h"
#include "StreamingSoundObject.h"

#include "DSPChain.h"

#include <iomanip> 
#include <string>
#include <sstream>

using namespace Raptor;
using namespace Raptor::Audio;

#define NON_COMPATIBLE \
	printf( "Not a compatible .WAV file!\n" ); \
	parent->m_BadFile = true; \
	return; \

StreamingSoundObjectWavImpl::StreamingSoundObjectWavImpl( StreamingSoundObject* parent )
		:
StreamingSoundObjectImpl( 0 ) 
{
	m_GlobalPosition = 0;

	m_UpdateTime = 0;

	WaveFile* wvFile = new WaveFile;

	parent->GetWaveFileHeader() = wvFile;

	wvFile->wf_Data = (char*) malloc( SIZE_OF_WAV_HEADER );

	unsigned int size = 0;

	parent->m_AudioSource->Seek( 0, SeekOrigins::SEEK_ORIGIN_END );
	size = parent->m_AudioSource->Tell();

	m_FileSize = size;

	if ( size <= SIZE_OF_WAV_HEADER ) 
	{
		NON_COMPATIBLE
	}

	parent->m_AudioSource->Seek( 0, SeekOrigins::SEEK_ORIGIN_SET );
	parent->m_AudioSource->Read( wvFile->wf_Data, SIZE_OF_WAV_HEADER, 1 );

	parent->SetAdvanceAmount( (double) wvFile->wf_WaveHeaders->wh_FmtHeader.fh_SampleRate / (double) SoundMixer::GetMixer()->GetWaveOut()->GetSampleRate() ); 

	unsigned int blockAlign = wvFile->wf_WaveHeaders->wh_FmtHeader.fh_NumChannels * 
		wvFile->wf_WaveHeaders->wh_FmtHeader.fh_BitsPerSample / 8;

	unsigned int byteRate = wvFile->wf_WaveHeaders->wh_FmtHeader.fh_SampleRate * 
		wvFile->wf_WaveHeaders->wh_FmtHeader.fh_NumChannels * 
		wvFile->wf_WaveHeaders->wh_FmtHeader.fh_BitsPerSample / 8;

	if ( strncmp( wvFile->wf_WaveHeaders->wh_FmtHeader.fh_Subchunk1ID, "fmt ", 4 ) != 0 ||
		 strncmp( wvFile->wf_WaveHeaders->wh_RiffHeader.rh_ChunkID, "RIFF", 4 ) != 0 ||
		 strncmp( wvFile->wf_WaveHeaders->wh_DataHeader.dh_Subchunk2ID, "data", 4 ) != 0 ||
		 wvFile->wf_WaveHeaders->wh_FmtHeader.fh_Subchunk1Size != 16 ||
		 wvFile->wf_WaveHeaders->wh_DataHeader.dh_Subchunk2Size > size ||
		 wvFile->wf_WaveHeaders->wh_FmtHeader.fh_BlockAlign != blockAlign ||
		 wvFile->wf_WaveHeaders->wh_FmtHeader.fh_ByteRate != byteRate ||
		 wvFile->wf_WaveHeaders->wh_FmtHeader.fh_AudioFormat != WAVE_FORMAT_PCM )
	{
		NON_COMPATIBLE
	}

	parent->SetBufferSize( ( wvFile->wf_WaveHeaders->wh_DataHeader.dh_Subchunk2Size / 
				( wvFile->wf_WaveHeaders->wh_FmtHeader.fh_BitsPerSample / 8 ) ) /
				wvFile->wf_WaveHeaders->wh_FmtHeader.fh_NumChannels );

	m_NumChannels = wvFile->wf_WaveHeaders->wh_FmtHeader.fh_NumChannels;

	m_BeginDataSeg = parent->m_AudioSource->Tell();
	m_StreamPos = 0;

	m_BufferChannels = (RingBuffer**) malloc( sizeof( RingBuffer* ) * m_NumChannels );

	for ( unsigned int i = 0; i < wvFile->wf_WaveHeaders->wh_FmtHeader.fh_NumChannels; i++ )
	{
		m_BufferChannels[i] = new RingBuffer( BufferSizes::BUF_SIZE_16384 );
		m_BufferChannels[i]->GetWritePosition() = 1;
	}

	m_Parent = parent;

	parent->m_AudioSource->Seek( ( parent->GetWaveFileHeader()->wf_WaveHeaders->wh_FmtHeader.fh_BitsPerSample / 8 ) * m_StreamPos + m_BeginDataSeg, SEEK_SET );
}

StreamingSoundObjectWavImpl::~StreamingSoundObjectWavImpl( void )
{
	free( m_Parent->GetWaveFileHeader()->wf_Data );
		delete m_Parent->GetWaveFileHeader();

	for ( unsigned int b = 0; b < m_NumChannels; b++ )
		delete m_BufferChannels[b];

	free( m_BufferChannels );
}

short StreamingSoundObjectWavImpl::GetCurrentSample( unsigned int num )
{
	unsigned int origNum = num;
	if ( num >= m_NumChannels ) num = m_NumChannels - 1;

	double position = m_Parent->GetPosition();

	double frac = position - (double) ( (int) position );

	short val1 = m_BufferChannels[num]->GetBuffer()[ (int) position ];

	int nextPos = (int) ( position + 1 ) & ( m_BufferChannels[num]->GetBufferSize() - 1 );

	short val2 = m_BufferChannels[num]->GetBuffer()[ nextPos ];

	return (short) ( ( ( ( 1.0 - frac ) * val1 ) + (short) ( frac * val2 )  ) * m_Parent->GetVolume() );
}

const short* StreamingSoundObjectWavImpl::GetChannelPtr( unsigned int num )
{
	if ( num >= m_NumChannels ) num = m_NumChannels - 1;

	return m_BufferChannels[num]->GetBuffer();
}

bool StreamingSoundObjectWavImpl::UpdateBuffer( void )
{
	int sampleI;
	unsigned char sampleC;
	float sampleF;
	short sampleT;
		
	short val1 = 0;
	short val2 = 0;
		
	switch ( m_Parent->GetWaveFileHeader()->wf_WaveHeaders->wh_FmtHeader.fh_BitsPerSample )
	{
	case 8:
		m_Parent->m_AudioSource->Read( &sampleC, sizeof( char ), 1 );
		sampleT = ( ( (int) sampleC - 128 ) << 7 );
		val1 = sampleT;
		if ( m_NumChannels <= 1 ) break;
		m_Parent->m_AudioSource->Read( &sampleC, sizeof( char ), 1 );
		sampleT = ( ( (int) sampleC - 128 ) << 7 );
		val2 = sampleT;
		break;
	case 16:
		m_Parent->m_AudioSource->Read( &val1, sizeof( short ), 1 );
		if ( m_NumChannels <= 1 ) break;
		m_Parent->m_AudioSource->Read( &val2, sizeof( short ), 1 );
		break;
	case 24:
		m_Parent->m_AudioSource->Read( &sampleI, 3, 1 );
		sampleT = (short) ( sampleI >> 8 );
		val1 = sampleT;
		if ( m_NumChannels <= 1 ) break;
		m_Parent->m_AudioSource->Read( &sampleI, 3, 1 );
		sampleT = (short) ( sampleI >> 8 );
		val2 = sampleT;
		break;
	case 32:
		m_Parent->m_AudioSource->Read( &sampleF, sizeof( float ), 1 );
		sampleT = (short) ( sampleF * SHRT_MAX );
		val1 = sampleT;
		if ( m_NumChannels <= 1 ) break;
		m_Parent->m_AudioSource->Read( &sampleF, sizeof( float ), 1 );
		sampleT = (short) ( sampleF * SHRT_MAX );
		val2 = sampleT;
		break;
	default:
		break;
	}

	int error = m_Parent->m_AudioSource->Error();

	if ( error != 0 )
	{
		printf( "%s : An error has occurred while streaming. Playback has been halted.\n", m_FilePath, error );

		char pBuffer[256];
		sprintf( pBuffer, "%s : (%d) ", m_FilePath, error );

		perror( pBuffer );

		m_Parent->m_BadFile = true;
		m_Parent->SetLooping( false );
		m_GlobalPosition = m_Parent->GetBufferSize();

		return false;
	}

	BufferResults::BufferResult b = BufferResults::BUFFER_FREE;

	b = m_BufferChannels[0]->WriteBuffer( val1 );
	if ( m_NumChannels > 1 ) b = m_BufferChannels[1]->WriteBuffer( val2 );

	if ( b == BufferResults::BUFFER_FULL )
	{
		m_Parent->m_AudioSource->Seek( -( m_Parent->GetWaveFileHeader()->wf_WaveHeaders->wh_FmtHeader.fh_BitsPerSample / 8 ) * m_NumChannels, SEEK_CUR );
		return false;
	}

	m_StreamPos++;

	if ( m_StreamPos >= m_Parent->GetBufferSize() )
	{
		m_StreamPos = 0;
		m_Parent->m_AudioSource->Seek( ( m_Parent->GetWaveFileHeader()->wf_WaveHeaders->wh_FmtHeader.fh_BitsPerSample / 8 ) * m_StreamPos + m_BeginDataSeg, SEEK_SET );
	}

	return true;
}

void StreamingSoundObjectWavImpl::Update( void )
{
	m_BufferChannels[0]->GetReadPosition() = (unsigned int) m_Parent->GetPosition();
	if ( m_NumChannels > 1 ) m_BufferChannels[1]->GetReadPosition() = (unsigned int) m_Parent->GetPosition();

	double cAA = ( m_Parent->GetAdvanceAmount() * m_Parent->GetPitchShift() );
	m_UpdateTime -= ( cAA < 1.0 ? 1.0 / cAA : cAA );

	if ( m_UpdateTime <= 0 )
	{
		for ( unsigned int i = 0; i < m_NumChannels; i++ )
			m_BufferChannels[i]->GetReadPosition() = (int) m_Parent->GetPosition();

		while ( UpdateBuffer() );
		m_UpdateTime = floor( (double) m_BufferChannels[0]->GetBufferSize() * 0.75 * ( 1.0 / ( cAA ) ) );
	}
}

SoundObjectResults::SoundObjectResult StreamingSoundObjectWavImpl::AdvancePosition( void )
{
	Update();

	m_Parent->SetPosition( m_Parent->GetPosition() + m_Parent->GetAdvanceAmount() * m_Parent->GetPitchShift() );
	m_GlobalPosition += m_Parent->GetAdvanceAmount() * m_Parent->GetPitchShift();

	while ( m_Parent->GetPosition() >= m_BufferChannels[0]->GetBufferSize() )
	{
		m_Parent->SetPosition( m_Parent->GetPosition() - (double) m_BufferChannels[0]->GetBufferSize() );
	}

	if ( m_GlobalPosition >= m_Parent->GetBufferSize() )
	{
		if ( m_Parent->GetLooping() )
		{
			m_UpdateTime = 0;
			m_GlobalPosition -= m_Parent->GetBufferSize();

			if ( m_Parent->m_Properties->sop_Shared != 0 )
			{
				if ( m_Parent->m_Properties->sop_Shared->sp_DSPChain != 0 )
				{
					m_Parent->m_Properties->sop_Shared->sp_DSPChain->PerformRandomizePlay();
					m_Parent->m_Properties->sop_Shared->sp_DSPChain->PerformPerPlayProcessing( m_Parent->m_Properties );
				}
			}

			return SoundObjectResults::SOUND_OBJECT_PLAYING;
		}

		return SoundObjectResults::SOUND_OBJECT_DONE;
	}

	return SoundObjectResults::SOUND_OBJECT_PLAYING;
}

const char* StreamingSoundObject::GetFilePath( void )
{
	return m_SoundObjectImpl->m_FilePath;
}