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

#pragma once

#include "SoundObject.h"
#include "RingBuffer.h"

#define SECONDS_TO_SAMPLES( sampleRate, secondsTotal ) ( (double) sampleRate * (double) secondsTotal )
#define ECHO_PARAMETERS_TO_SAMPLES( sampleRate, numberOfEchoes, secondsBetween ) SECONDS_TO_SAMPLES( sampleRate, (double) secondsBetween * (double) numberOfEchoes )

namespace Raptor
{
	namespace Audio
	{
		struct ChannelSamplePair
		{
			short L;
			short R;
		};

		class HistoryBufferObject : public SoundObject
		{
		public:
			HistoryBufferObject( unsigned int maxSamples );
			~HistoryBufferObject( void );

		public:
			void InitializeEchoes( SoundObjectProperties propertiesSource, unsigned int numberOfEchoes, double secondsBetween, double volumeReductionPower = 1.0 );
			void SetEchoEnabled( bool playing );

		public:
			ChannelSamplePair GetCurrentSamples( void );
			SoundObjectResults::SoundObjectResult AdvancePosition( void );

			const short* GetChannelPtr( unsigned int num );

			void WriteLR( short L, short R );
			void WriteAccumulation( void );

			void SetAdvanceAmount( double amount ) {}
			void SetBufferSize( unsigned int size ) {}
			void SetPosition( double position ) {}

			void AccumulateSampleL( int val );
			void AccumulateSampleR( int val );

		private:
			short m_AccumL;
			short m_AccumR;

		private:
			RingBuffer* m_ChannelRingBuffers[2];

			SoundObjectPropertiesInternal* m_EchoMarkers;
			unsigned int m_NumEchoes;

			double m_WritePosition;
			bool m_EchoEnabled;
		};
	};
};