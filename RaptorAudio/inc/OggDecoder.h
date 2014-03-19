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

#include "stb_vorbis.h"

#include <process.h>
#include <Windows.h>
#include <queue>

namespace Raptor
{
	namespace Audio
	{
		struct OggDecoderRequest
		{
			short* odr_DstBuffer1;
			short* odr_DstBuffer2;

			stb_vorbis* odr_Vorbis;
			bool* odr_LoopDone;

			unsigned int odr_BufferSize;
		};

		class SoundMixer;

		class OggDecoder
		{
		public:
			OggDecoder( void );
			~OggDecoder( void );

		public:
			static OggDecoder* GetSingleton( void );
			static void DeleteSingleton( void );
		
		public:
			void DecodeOggNow( OggDecoderRequest request );
			void DecodeOgg( OggDecoderRequest request );

		private:
			void UpdateOggBuffer( short* tempStorage, OggDecoderRequest decoderRequest );

		private:
			short* m_NonConcurrentBuffer;
			short* m_ConcurrentBuffer;

		private:
			CRITICAL_SECTION m_RequestQueueCSec;
			HANDLE m_OggDecoderStopEvent;
			HANDLE m_RequestQueueSemaphore;
			HANDLE m_RequestThread;

		private:
			std::queue< OggDecoderRequest > m_RequestQueue;

		private:
			static OggDecoder* m_Singleton;
			friend DWORD WINAPI OggRequestThread( void* ptr );
			friend class SoundMixer;
		};
	};
};
