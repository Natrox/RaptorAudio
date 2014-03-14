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

#include "SoundObjectProperties.h"
#include "WaveFile.h"
#include "AudioSource.h"

namespace Raptor
{
	namespace Audio
	{
		namespace SoundObjectResults
		{
			enum SoundObjectResult
			{
				SOUND_OBJECT_PLAYING,
				SOUND_OBJECT_DONE
			};
		};

		class SoundObject
		{
		public:
			enum SoundType
			{
				SOUND_MEMORY,
				SOUND_STREAMED,
				SOUND_HISTORY
			};

		public:
			SoundObject( void );
			virtual ~SoundObject( void );

		public:
			virtual short GetCurrentSample( unsigned int num ) { return 0; };
			virtual const short* GetChannelPtr( unsigned int num ) { return 0; };

		public:
			virtual void Update( void ) {};
			virtual SoundObjectResults::SoundObjectResult AdvancePosition( void );

			const double& GetAdvanceAmount( void );
			const unsigned int& GetBufferSize( void );
			const double& GetPosition( void );
			const double& GetPitchShift( void );
			const unsigned int& GetNumberOfChannels( void );
			const bool& GetLooping( void );
			SoundType GetType( void );

			virtual void SetAdvanceAmount( double amount );
			virtual void SetBufferSize( unsigned int size );
			virtual void SetPosition( double position );
			void SetLooping( bool looping );

			void SetVolume( float volume );
			float GetVolume( void );

		public:
			WaveFile*& GetWaveFileHeader( void );
			SoundObjectPropertiesInternal m_Properties;
			AudioSource* m_AudioSource;

			bool m_BadFile;

		protected:
			WaveFile* m_WaveFile;
			unsigned int m_BufferSize;
			unsigned int m_NumChannels;
			SoundType m_Type;

			double m_AdvanceAmount;
		};
	};
};

