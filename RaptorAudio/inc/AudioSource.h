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

#include <stdio.h>

namespace Raptor
{
	namespace Audio
	{
		namespace AudioOrigins
		{
			enum AudioOrigin
			{
				AUDIO_ORIGIN_FILE,
				AUDIO_ORIGIN_OPENMEMORY,
				AUDIO_ORIGIN_OPENMEMORY_POINT
			};
		};
		
		namespace SeekOrigins
		{
			enum SeekOrigin
			{
				SEEK_ORIGIN_SET,
				SEEK_ORIGIN_CUR,
				SEEK_ORIGIN_END
			};
		};

		class AudioSource
		{
		public:
			AudioSource( const char* fileNameOrPtr, AudioOrigins::AudioOrigin type = AudioOrigins::AUDIO_ORIGIN_FILE );
			virtual ~AudioSource( void );

		public:
			AudioOrigins::AudioOrigin GetType( void );
			virtual void* GetPtrData( void ) = 0;
			virtual size_t GetLength( void );

		public:
			virtual size_t Read( void* dstBuf, size_t elementSize, size_t count ) = 0;
			virtual int Seek( long int offset, int seekOrigin ) = 0;
			virtual long Tell( void ) = 0;
			virtual int Error( void );

		public:
			virtual bool CheckIfLoaded( void );
			virtual void Close( void );

		protected:
			AudioOrigins::AudioOrigin m_Type;
		};
	};
};