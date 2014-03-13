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

#include "HistoryBufferObject.h"
#include "SoundMixer.h"
#include "DSPChain.h"

#include <Windows.h>
#include <memory>

#define PI 3.14159265359

using namespace Raptor;
using namespace Raptor::Audio;

HistoryBufferObject::HistoryBufferObject( unsigned int maxSamples )
	:
SoundObject(),
m_NumEchoes( 0 ),
m_EchoMarkers( 0 ),
m_EchoEnabled( false ),
m_WritePosition( 1 ),
m_AccumL( 0 ),
m_AccumR( 0 )
{
	m_ChannelRingBuffers[0] = new RingBuffer( maxSamples );
	m_ChannelRingBuffers[1] = new RingBuffer( maxSamples );

	m_BufferSize = maxSamples;
	m_AdvanceAmount = 1.0;
	m_BadFile = false;
	m_Type = SoundObject::SOUND_HISTORY;
}

HistoryBufferObject::~HistoryBufferObject( void )
{
	delete m_ChannelRingBuffers[0];
	delete m_ChannelRingBuffers[1];

	if ( m_EchoMarkers != 0 )
	{
		delete [] m_EchoMarkers;
	}
}

void HistoryBufferObject::InitializeEchoes( SoundObjectProperties propertiesSource, unsigned int numberOfEchoes, double secondsBetween, double volumeReductionPower )
{
	if ( m_EchoMarkers != 0 ) return;

	propertiesSource->SetLooping( true );
	m_Properties = propertiesSource._Convert<_SoundObjectPropertiesInternal>();
	m_NumEchoes = numberOfEchoes;
	m_EchoEnabled = true;

	m_Properties->sop_Position = 0.0;
	m_Properties->sop_Object = this;

	m_EchoMarkers = new SoundObjectPropertiesInternal[ numberOfEchoes ];

	double level = 1.0;

	double incrementer = 1.0 / ( (double) numberOfEchoes + 1.0 );
	double currentPos = PI + incrementer;
	
	for ( unsigned int i = 0; i < m_NumEchoes; i++ )
	{
		m_EchoMarkers[i] = new _SoundObjectPropertiesInternal();

		double position = -SECONDS_TO_SAMPLES( SoundMixer::GetMixer()->GetWaveOut()->GetSampleRate(),
											   secondsBetween * (double) ( i + 1 ) );

		//level -= levelReducer;

		level = pow( ( sin( currentPos ) + 1 ), volumeReductionPower );
		currentPos += incrementer * PI / 2.0;

 		m_EchoMarkers[i]->sop_Position = position;
		m_EchoMarkers[i]->sop_EchoVolume = level;
		m_EchoMarkers[i]->sop_EchoTime = position;
	}
}

void HistoryBufferObject::SetEchoEnabled( bool playing )
{
	m_EchoEnabled = playing;
}

ChannelSamplePair HistoryBufferObject::GetCurrentSamples( void )
{
	int valL = (int) ( (double) m_ChannelRingBuffers[0]->GetBuffer()[ (unsigned int) GetPosition() ] * GetVolume() );
	int valR = (int) ( (double) m_ChannelRingBuffers[1]->GetBuffer()[ (unsigned int) GetPosition() ] * GetVolume() );

	DSPChain* chainMain = 0;

	SoundObjectPropertiesInternal oldProperties = m_Properties;

	if ( m_Properties->sop_Shared != 0 )
		chainMain = m_Properties->sop_Shared->sp_DSPChain;

	if ( chainMain != 0 )
	{
		chainMain->GetSignalL() = valL;
		chainMain->GetSignalR() = valR;

		chainMain->PerformProcessing( this );
				
		valL = chainMain->GetSignalL();
		valR = chainMain->GetSignalR();
	}

	if ( m_EchoMarkers != 0 && m_EchoEnabled == true )
	{
		for ( unsigned int i = 0; i < m_NumEchoes; i++ )
		{
			if ( m_EchoMarkers[i]->sop_Position < 0 ) continue;

			m_Properties = m_EchoMarkers[i];

			int echoL = m_ChannelRingBuffers[0]->GetBuffer()[ (unsigned int) m_EchoMarkers[i]->sop_Position ];
			int echoR = m_ChannelRingBuffers[1]->GetBuffer()[ (unsigned int) m_EchoMarkers[i]->sop_Position ];
			
			echoL = (int) ( (double) echoL * m_EchoMarkers[i]->sop_EchoVolume * GetVolume() );
			echoR = (int) ( (double) echoR * m_EchoMarkers[i]->sop_EchoVolume * GetVolume() );

			if ( chainMain != 0 )
			{
				chainMain->GetSignalL() = echoL;
				chainMain->GetSignalR() = echoR;

				chainMain->PerformProcessing( this );
				
				echoL = chainMain->GetSignalL();
				echoR = chainMain->GetSignalR();
			}

			valL = ClippedMix( echoL, valL );
			valR = ClippedMix( echoR, valR );
		}
	}

	m_Properties = oldProperties;

	ChannelSamplePair pair;
	
	pair.L = valL;
	pair.R = valR;

	return pair;
}

const short* HistoryBufferObject::GetChannelPtr( unsigned int num )
{
	return m_ChannelRingBuffers[num]->GetBuffer();
}

void HistoryBufferObject::WriteLR( short L, short R )
{
	m_ChannelRingBuffers[0]->GetBuffer()[ (unsigned int) m_WritePosition ] = L;
	m_ChannelRingBuffers[1]->GetBuffer()[ (unsigned int) m_WritePosition ] = R;
}

SoundObjectResults::SoundObjectResult HistoryBufferObject::AdvancePosition( void )
{
	m_Properties->sop_Position += 1.0;

	if ( (unsigned int) m_Properties->sop_Position >= m_BufferSize )
	{
		m_Properties->sop_Position = 0;
	}

	for ( unsigned int i = 0; i < m_NumEchoes; i++ )
	{
		m_EchoMarkers[i]->sop_Position += 1.0;

		if ( (int) m_EchoMarkers[i]->sop_Position >= (int) m_BufferSize )
		{
			m_EchoMarkers[i]->sop_Position = 0;
		}
	}

	m_WritePosition += 1.0;

	if ( m_WritePosition >= m_BufferSize )
	{
		m_WritePosition = 0;
	}

	m_ChannelRingBuffers[0]->GetBuffer()[ (unsigned int) m_WritePosition ] = 0;
	m_ChannelRingBuffers[1]->GetBuffer()[ (unsigned int) m_WritePosition ] = 0;

	return SoundObjectResults::SOUND_OBJECT_PLAYING;
}

void HistoryBufferObject::AccumulateSampleL( int val )
{
	m_AccumL = ClippedMix( m_AccumL, val );
}

void HistoryBufferObject::AccumulateSampleR( int val )
{
	m_AccumR = ClippedMix( m_AccumR, val );
}

void HistoryBufferObject::WriteAccumulation( void )
{
	WriteLR( m_AccumL, m_AccumR );

	m_AccumL = 0;
	m_AccumR = 0;
}
