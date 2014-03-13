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

#include "MemorySoundObjectWavImpl.h"
#include "MemorySoundObject.h"

#include <iomanip> 
#include <string>
#include <sstream>

using namespace Raptor;
using namespace Raptor::Audio;

#define NON_COMPATIBLE \
	printf( "%s : Not a compatible .WAV file!\n", filePath ); \
	parent->m_BadFile = true; \
	fclose( audioFile ); \
	return; \

MemorySoundObjectWavImpl::MemorySoundObjectWavImpl( const char* filePath, WaveoutDevice* wvOut, MemorySoundObject* parent )
	:
MemorySoundObjectImpl( filePath, 0, 0 )
{
	FILE* audioFile = fopen( filePath, "rb" );

	WaveFile* wvFile = new WaveFile;

	parent->GetWaveFileHeader() = wvFile;

	wvFile->wf_Data = (char*) malloc( SIZE_OF_WAV_HEADER );

	unsigned int size = 0;

	fseek( audioFile, 0, SEEK_END );
	size = ftell( audioFile );

	if ( size <= SIZE_OF_WAV_HEADER ) 
	{
		NON_COMPATIBLE
	}

	fseek( audioFile, 0, SEEK_SET );
	fread( wvFile->wf_Data, SIZE_OF_WAV_HEADER, 1, audioFile );

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

	m_BufferChannels = (short**) malloc( sizeof( short* ) * 2 );

	parent->SetBufferSize( ( wvFile->wf_WaveHeaders->wh_DataHeader.dh_Subchunk2Size / 
				( wvFile->wf_WaveHeaders->wh_FmtHeader.fh_BitsPerSample / 8 ) ) /
				wvFile->wf_WaveHeaders->wh_FmtHeader.fh_NumChannels );

	for ( unsigned int i = 0; i < wvFile->wf_WaveHeaders->wh_FmtHeader.fh_NumChannels; i++ )
	{
		m_BufferChannels[i] = (short*) malloc( sizeof( short ) * parent->GetBufferSize() );
	}

	unsigned char sampleC;
	float sampleF;
	short sampleT;

	for ( unsigned int i = 0; i < parent->GetBufferSize(); i++ )
	{
		for ( unsigned int b = 0; b < wvFile->wf_WaveHeaders->wh_FmtHeader.fh_NumChannels; b++ )
		{
			switch ( wvFile->wf_WaveHeaders->wh_FmtHeader.fh_BitsPerSample )
			{
			case 8:
				sampleC = 0;
				fread( &sampleC, sizeof( char ), 1, audioFile );
				sampleT = ( ( (int) sampleC - 128 ) << 7 );
				m_BufferChannels[b][i] = sampleT;
				break;
			case 16:
				fread( &m_BufferChannels[b][i], sizeof( short ), 1, audioFile );
				break;
			case 32:
				sampleF = 0;
				fread( &sampleF, sizeof( float ), 1, audioFile );
				sampleT = (short) ( sampleF * SHRT_MAX );
				m_BufferChannels[b][i] = sampleT;
				break;
			default:
				printf( "%s : Unsupported bitrate!\n", filePath );
				return;
			}
		}
	}

	m_NumChannels = wvFile->wf_WaveHeaders->wh_FmtHeader.fh_NumChannels;
	wvFile->wf_WaveHeaders->wh_FmtHeader.fh_BitsPerSample = 16;
	parent->SetAdvanceAmount( (double) wvFile->wf_WaveHeaders->wh_FmtHeader.fh_SampleRate / (double) wvOut->GetSampleRate() );

	m_Parent = parent;

	fclose( audioFile );
}

MemorySoundObjectWavImpl::~MemorySoundObjectWavImpl( void )
{
	free( m_Parent->GetWaveFileHeader()->wf_Data );
		delete m_Parent->GetWaveFileHeader();

	for ( unsigned int b = 0; b < m_NumChannels; b++ )
		free( m_BufferChannels[b] );

	free( m_BufferChannels );
}

short MemorySoundObjectWavImpl::GetCurrentSample( unsigned int num )
{
	unsigned int origNum = num;
	if ( num >= m_NumChannels ) num = m_NumChannels - 1;

	double position = m_Parent->GetPosition();

	double frac = position - (double) ( (int) position );

	short val1 = m_BufferChannels[num][ (unsigned int) position ];

	int nextPos = (int) ( position + 1 );
	if ( nextPos >= (int) m_Parent->GetBufferSize() ) nextPos -= (int) m_Parent->GetBufferSize();

	short val2 = m_BufferChannels[num][ nextPos ];

	return (short) ( ( ( ( 1.0 - frac ) * val1 ) + (short) ( frac * val2 )  ) * m_Parent->GetVolume() );
}

const short* MemorySoundObjectWavImpl::GetChannelPtr( unsigned int num )
{
	if ( num >= m_NumChannels ) num = m_NumChannels - 1;

	return m_BufferChannels[num];
}