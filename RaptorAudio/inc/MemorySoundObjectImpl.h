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

#include "WaveoutDevice.h"
#include "SoundObject.h"

#include <iostream>

namespace Raptor
{
	namespace Audio
	{
		class MemorySoundObject;

		class MemorySoundObjectImpl
		{
		protected:
			MemorySoundObjectImpl( const char* filePath, WaveoutDevice* wvOut, MemorySoundObject* parent )
				:
			m_FilePath( filePath )
			{}

			virtual ~MemorySoundObjectImpl( void ){};
			friend class MemorySoundObject;

		protected:
			virtual short GetCurrentSample( unsigned int num ) = 0;
			virtual const short* GetChannelPtr( unsigned int num ) = 0;

		protected:
			short** m_BufferChannels;
			unsigned int m_NumChannels;
			const char* m_FilePath;
		};
	};
};