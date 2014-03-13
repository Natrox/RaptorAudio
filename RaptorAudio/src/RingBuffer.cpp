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

#include "RingBuffer.h"

#include <memory>

using namespace Raptor;
using namespace Raptor::Audio;

RingBuffer::RingBuffer( BufferSizes::BufferSize bufferSize )
	:
m_NumSamples( (unsigned int) bufferSize ),
m_WritePos( 1 ),
m_ReadPos( 0 )
{
	m_Buffer = (short*) malloc( sizeof( short ) * m_NumSamples );
	memset( m_Buffer, 0, sizeof( short ) * m_NumSamples );
}

RingBuffer::RingBuffer( unsigned int bufferSize )
	:
m_NumSamples( (unsigned int) bufferSize ),
m_WritePos( 1 ),
m_ReadPos( 0 )
{
	m_Buffer = (short*) malloc( sizeof( short ) * m_NumSamples );
	memset( m_Buffer, 0, sizeof( short ) * m_NumSamples );
}

RingBuffer::~RingBuffer( void )
{
	free( m_Buffer );
}

short* RingBuffer::GetBuffer( void )
{
	return m_Buffer;
}

unsigned int& RingBuffer::GetReadPosition( void )
{
	return m_ReadPos;
}

unsigned int& RingBuffer::GetWritePosition( void )
{
	return m_WritePos;
}

unsigned int RingBuffer::GetBufferSize( void )
{
	return m_NumSamples;
}

BufferResults::BufferResult RingBuffer::WriteBuffer2( short value1, short value2 )
{
	if ( ( ( m_WritePos + 1 ) & ( m_NumSamples - 1 ) ) == m_ReadPos ||
		 ( ( m_WritePos + 2 ) & ( m_NumSamples - 1 ) ) == m_ReadPos )
	{
		return BufferResults::BUFFER_FULL;
	}

	m_WritePos++;
	m_WritePos &= ( m_NumSamples - 1 );

	m_Buffer[ m_WritePos ] = value1;

	m_WritePos++;
	m_WritePos &= ( m_NumSamples - 1 );

	m_Buffer[ m_WritePos ] = value2;

	return BufferResults::BUFFER_FREE;
}

BufferResults::BufferResult RingBuffer::WriteBuffer( short value )
{
	if ( ( ( m_WritePos + 1 ) & ( m_NumSamples - 1 ) ) == m_ReadPos )
	{
		return BufferResults::BUFFER_FULL;
	}

	m_WritePos++;
	m_WritePos &= ( m_NumSamples - 1 );

	m_Buffer[ m_WritePos ] = value;

	return BufferResults::BUFFER_FREE;
}

BufferResults::BufferResult RingBuffer::CheckStatus( void )
{
	if ( ( ( m_WritePos + 1 ) & ( m_NumSamples - 1 ) ) == m_ReadPos ||
		 ( ( m_WritePos + 2 ) & ( m_NumSamples - 1 ) ) == m_ReadPos )
	{
		return BufferResults::BUFFER_FULL;
	}

	return BufferResults::BUFFER_FREE;
}
