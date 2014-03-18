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

#include "WaveoutDevice.h"

#include <stdio.h>

#include "SoundMixer.h"

using namespace Raptor;
using namespace Raptor::Audio;

namespace Raptor
{
	unsigned int g_NumBlockSamples;
	WaveoutDevice* g_Waveout;

	namespace Audio
	{
		void CALLBACK WaveoutCallback( HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2 )
		{
			if ( uMsg == WOM_DONE )
			{
				waveOutWrite( g_Waveout->m_Waveout, &g_Waveout->m_BlockBufferHeader[ ( g_Waveout->m_CurrentBlock + 1 ) % 2 ], sizeof( WAVEHDR ) );

				g_Waveout->m_CurrentBlock = ( g_Waveout->m_CurrentBlock + 1 ) % 2;
				SoundMixer::GetMixer()->m_BlockPlaybackBuffer = g_Waveout->m_BlockBuffer[ g_Waveout->m_CurrentBlock ];			
				
				SoundMixer::GetMixer()->m_BlockPlaybackBuffer->ResetParameters();

				SetEvent( g_Waveout->m_WaveoutBufferSwapEvent );
			}
		}

		DWORD WINAPI WaveoutBlockBufferThreadFunction( WaveoutDevice* wvOut )
		{
			HWAVEOUT waveOut;
			WAVEFORMATEX waveFormat;

			waveFormat.wFormatTag = WAVE_FORMAT_PCM;
			waveFormat.nChannels = 2;
			waveFormat.wBitsPerSample = 16;
			waveFormat.nSamplesPerSec = wvOut->GetSampleRate();
			waveFormat.nBlockAlign = (WORD) ( waveFormat.nChannels * waveFormat.wBitsPerSample / 8 );
			waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
			waveFormat.cbSize = 0;

			if ( waveOutOpen( &waveOut, 0, &waveFormat, (DWORD_PTR) WaveoutCallback, 0, CALLBACK_FUNCTION ) != MMSYSERR_NOERROR )
			{
				printf( "ERROR: Could not open sound device!\n" );
			}

			for ( unsigned int i = 0; i < 2; i++ )
			{
				wvOut->m_BlockBufferHeader[i].lpData = (char*) wvOut->m_BlockBuffer[i]->GetDataPtr();
				wvOut->m_BlockBufferHeader[i].dwBufferLength = g_NumBlockSamples * sizeof( short ) * 2;
				wvOut->m_BlockBufferHeader[i].dwFlags = 0;
				wvOut->m_BlockBufferHeader[i].dwLoops = 0;
			}

			wvOut->m_Waveout = waveOut;
			g_Waveout = wvOut;

			waveOutPrepareHeader( waveOut, &wvOut->m_BlockBufferHeader[0], sizeof( wvOut->m_BlockBufferHeader[0] ) );
			waveOutPrepareHeader( waveOut, &wvOut->m_BlockBufferHeader[1], sizeof( wvOut->m_BlockBufferHeader[1] ) );

			SoundMixer::GetMixer()->m_BlockPlaybackBuffer = g_Waveout->m_BlockBuffer[ g_Waveout->m_CurrentBlock ];			

			waveOutWrite( waveOut, &wvOut->m_BlockBufferHeader[ 1 ], sizeof( WAVEHDR ) );
			waveOutWrite( waveOut, &wvOut->m_BlockBufferHeader[ 0 ], sizeof( WAVEHDR ) );

			SetEvent( wvOut->m_WaveoutReadyEvent );

			if ( WaitForSingleObject( wvOut->m_WaveoutStopEvent, INFINITE ) == WAIT_OBJECT_0 )
			{
				WaitForSingleObject( wvOut->m_WaveoutWriteStopEvent, INFINITE );
			}

			waveOutReset( waveOut );

			waveOutUnprepareHeader( waveOut, &wvOut->m_BlockBufferHeader[0], sizeof( wvOut->m_BlockBufferHeader[0] ) );
			waveOutUnprepareHeader( waveOut, &wvOut->m_BlockBufferHeader[1], sizeof( wvOut->m_BlockBufferHeader[1] ) );
		
			waveOutClose( waveOut );

			delete wvOut->m_BlockBuffer[0];
			delete wvOut->m_BlockBuffer[1];

			return 1;
		}
	};
};

DWORD WINAPI WaveoutBlockBufferThreadFunctionStart( void* ptr ) 
{ 
	return WaveoutBlockBufferThreadFunction( (WaveoutDevice*) ptr ); 
}

WaveoutDevice::WaveoutDevice( unsigned int sampleRate, unsigned int sampleAmount )
	:
m_SampleRate( sampleRate ),
m_Waveout( 0 ),
m_CurrentBlock( 0 )
{
	m_WaveoutStopEvent = CreateEvent( 0, true, 0, 0 );
	ResetEvent( m_WaveoutStopEvent );

	g_NumBlockSamples = sampleAmount;
	g_Waveout = this;

	m_WaveoutReadyEvent = CreateEvent( 0, true, 0, 0 );
	m_WaveoutBufferSwapEvent = CreateEvent( 0, false, 0, 0 );

	m_BlockBuffer[0] = new BlockBuffer( sampleAmount );
	m_BlockBuffer[1] = new BlockBuffer( sampleAmount );
	m_WaveoutThreadHandle = CreateThread( 0, 0, WaveoutBlockBufferThreadFunctionStart, this, 0, 0 );
}

WaveoutDevice::~WaveoutDevice( void )
{
	StopDevice();
}


unsigned int WaveoutDevice::GetSampleRate( void )
{
	return m_SampleRate;
}

void WaveoutDevice::StopDevice( void )
{
	SetEvent( m_WaveoutStopEvent );
	WaitForSingleObject( m_WaveoutThreadHandle, INFINITE );

	CloseHandle( m_WaveoutStopEvent );
	CloseHandle( m_WaveoutThreadHandle );
	CloseHandle( m_WaveoutReadyEvent );
	CloseHandle( m_WaveoutBufferSwapEvent );
}