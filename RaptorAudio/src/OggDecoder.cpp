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

#include "OggDecoder.h"
#include "WaveoutDevice.h"
#include "SoundMixer.h"

using namespace Raptor::Audio;

namespace Raptor
{
	namespace Audio
	{
		DWORD WINAPI OggRequestThread( void* ptr )
		{
			OggDecoder* oggDecoder = (OggDecoder*) ptr;

			const HANDLE handles[] =
			{
				oggDecoder->m_RequestQueueSemaphore,
				oggDecoder->m_OggDecoderStopEvent
			};

			while ( true )
			{
				DWORD result = WaitForMultipleObjects( 2, handles, false, INFINITE );

				if ( result == WAIT_OBJECT_0 + 1 )
				{
					break;
				}

				EnterCriticalSection( &oggDecoder->m_RequestQueueCSec );
				OggDecoderRequest request = oggDecoder->m_RequestQueue.front();
				oggDecoder->m_RequestQueue.pop();
				LeaveCriticalSection( &oggDecoder->m_RequestQueueCSec );

				oggDecoder->UpdateOggBuffer( oggDecoder->m_ConcurrentBuffer, request );
			}

			return 1;
		}
	};
};

OggDecoder::OggDecoder( void )
{
	InitializeCriticalSection( &m_RequestQueueCSec );
	m_RequestQueueSemaphore = CreateSemaphore( 0, 0, 0xFFFFFF, 0 );
	m_OggDecoderStopEvent = CreateEvent( 0, true, 0, 0 );

	m_NonConcurrentBuffer = (short*) malloc( sizeof( short ) * SoundMixer::GetMixer()->GetWaveOut()->GetSampleRate() * 10 );
	m_ConcurrentBuffer = (short*) malloc( sizeof( short ) *  SoundMixer::GetMixer()->GetWaveOut()->GetSampleRate() * 10 );

	m_RequestThread = CreateThread( 0, 0, OggRequestThread, this, 0, 0 );
}

OggDecoder::~OggDecoder( void )
{
	DeleteCriticalSection( &m_RequestQueueCSec );
	CloseHandle( m_RequestQueueSemaphore );
	CloseHandle( m_OggDecoderStopEvent );
	CloseHandle( m_RequestThread );

	free( m_ConcurrentBuffer );
	free( m_NonConcurrentBuffer );
}

OggDecoder* OggDecoder::GetSingleton( void )
{
	if ( m_Singleton == 0 )
	{
		m_Singleton = new OggDecoder();
	}

	return m_Singleton;
}

void OggDecoder::DecodeOggNow( OggDecoderRequest request )
{
	UpdateOggBuffer( m_NonConcurrentBuffer, request );
}

void OggDecoder::DecodeOgg( OggDecoderRequest request )
{
	EnterCriticalSection( &m_RequestQueueCSec );
	m_RequestQueue.push( request );
	ReleaseSemaphore( m_RequestQueueSemaphore, 1, 0 );
	LeaveCriticalSection( &m_RequestQueueCSec );
}

void OggDecoder::UpdateOggBuffer( short* tempStorage, OggDecoderRequest decoderRequest )
{
	if ( decoderRequest.odr_DstBuffer1 == 0 ) return;

	unsigned int channels = 1;
	if ( decoderRequest.odr_DstBuffer2 != 0 ) channels += 1;
	
	int samplesGotten = 0;

	samplesGotten = stb_vorbis_get_samples_short_interleaved( decoderRequest.odr_Vorbis, channels, tempStorage, decoderRequest.odr_BufferSize * channels ); 

	if ( samplesGotten != decoderRequest.odr_BufferSize )
	{
		int remainder = ( decoderRequest.odr_BufferSize * channels - samplesGotten * channels );
		*decoderRequest.odr_LoopDone = true;

		stb_vorbis_seek_start( decoderRequest.odr_Vorbis );

		short* ptr = tempStorage + samplesGotten * channels;

		stb_vorbis_get_samples_short_interleaved( decoderRequest.odr_Vorbis, channels, ptr, remainder ); 
	}

	for ( unsigned int i = 0; i < decoderRequest.odr_BufferSize; i++ )
	{
		decoderRequest.odr_DstBuffer1[i] = tempStorage[i*channels+0];
		if ( channels > 1 ) decoderRequest.odr_DstBuffer2[i] = tempStorage[i*channels+1];
	}
}

void OggDecoder::DeleteSingleton( void )
{
	if ( m_Singleton != 0 )
	{
		SetEvent( m_Singleton->m_OggDecoderStopEvent );

		WaitForSingleObject( m_Singleton->m_RequestThread, INFINITE );

		delete m_Singleton;
		m_Singleton = 0;
	}
}

OggDecoder* OggDecoder::m_Singleton = 0;
