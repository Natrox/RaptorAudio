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

#include "AudioSource.h"

namespace Raptor
{
	namespace Audio
	{
		class AudioSourceMemory : public AudioSource
		{
		public:
			AudioSourceMemory( const char* fileNameOrPtr, size_t length, AudioOrigins::AudioOrigin memoryOrigin = AudioOrigins::AUDIO_ORIGIN_OPENMEMORY );
			~AudioSourceMemory( void );

		public:
			void* GetPtrData( void );
			size_t GetLength( void );

		public:
			size_t Read( void* dstBuf, size_t elementSize, size_t count );
			long Tell( void );
			int Seek( long int offset, int seekOrigin );

		protected:
			char* m_Address;
			size_t m_ReadPos;
			size_t m_Length;
		};
	};
};