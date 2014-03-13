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

namespace Raptor
{
	namespace Audio
	{
		namespace BufferSizes
		{
			enum BufferSize
			{
				BUF_SIZE_POLL_DEVICE = 0,
				BUF_SIZE_512 = 512,
				BUF_SIZE_1024 = 1024,
				BUF_SIZE_2048 = 2048,
				BUF_SIZE_4096 = 4096,
				BUF_SIZE_8192 = 8192,
				BUF_SIZE_16384 = 16384,
				BUF_SIZE_32768 = 32768,
				BUF_SIZE_65536 = 65536,
				BUF_SIZE_131072 = 131072
			};
		};

		namespace BufferResults
		{
			enum BufferResult
			{
				BUFFER_FREE,
				BUFFER_FULL
			};
		};

		class RingBuffer
		{
		public:
			RingBuffer( BufferSizes::BufferSize bufferSize );
			RingBuffer( unsigned int bufferSize );
			~RingBuffer( void );

		public:
			short* GetBuffer( void );

		public:
			unsigned int& GetReadPosition( void );
			unsigned int& GetWritePosition( void );
			unsigned int GetBufferSize( void );

			BufferResults::BufferResult CheckStatus( void );
			BufferResults::BufferResult WriteBuffer2( short value1, short value2 );
			BufferResults::BufferResult WriteBuffer( short value );

		private:
			short* m_Buffer;
			unsigned int m_NumSamples;

			unsigned int m_ReadPos;
			unsigned int m_WritePos;

#ifdef _DEBUG
			friend class WaveoutDevice;
#endif
		};
	};
};