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

#include "AudioSourceMemory.h"

#include <memory>

using namespace Raptor::Audio;

AudioSourceMemory::AudioSourceMemory( const char* fileNameOrPtr, size_t length, AudioOrigins::AudioOrigin memoryOrigin )
	:
AudioSource( fileNameOrPtr, memoryOrigin ),
m_ReadPos( 0 ),
m_Length( length )
{
	if ( m_Type == AudioOrigins::AUDIO_ORIGIN_OPENMEMORY )
	{
		m_Address = (char*) malloc( length ); 
		memcpy( m_Address, fileNameOrPtr, length );
	}

	else
	{
		m_Address = (char*) fileNameOrPtr;
	}
}

AudioSourceMemory::~AudioSourceMemory( void )
{
	if ( m_Type == AudioOrigins::AUDIO_ORIGIN_OPENMEMORY )
	{
		free( m_Address );
	}
}

size_t AudioSourceMemory::Read( void* dstBuf, size_t elementSize, size_t count )
{
	memcpy( dstBuf, m_Address + m_ReadPos, elementSize * count );
	m_ReadPos += elementSize * count;
	return count;
}

long AudioSourceMemory::Tell( void )
{
	return (long) m_ReadPos;
}

int AudioSourceMemory::Seek( long int offset, int seekOrigin )
{
	switch ( seekOrigin )
	{
		case SeekOrigins::SEEK_ORIGIN_CUR:
			m_ReadPos += offset;
			break;

		case SeekOrigins::SEEK_ORIGIN_SET:
			m_ReadPos = offset;
			break;

		case SeekOrigins::SEEK_ORIGIN_END:
			m_ReadPos = m_Length - offset;
			break;
	}

	return 0;
}

void* AudioSourceMemory::GetPtrData( void )
{
	return m_Address;
}

size_t AudioSourceMemory::GetLength( void )
{
	return m_Length;
}
