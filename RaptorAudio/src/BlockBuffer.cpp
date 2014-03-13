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

#include "BlockBuffer.h"

#include <memory>

using namespace Raptor;
using namespace Raptor::Audio;

BlockBuffer::BlockBuffer( unsigned int amountOfSamples )
	:
m_NumSamples( amountOfSamples * 2 ),
m_BufferPosition( 0 )
{
	m_DataPtr = (short*) malloc( sizeof( short ) * m_NumSamples );
	memset( m_DataPtr, 0, sizeof( short ) * m_NumSamples );
}

BlockBuffer::~BlockBuffer( void )
{
	free( m_DataPtr );
}

bool BlockBuffer::WriteBuffer2( short lVal, short rVal )
{
	if ( m_BufferPosition + 2 > m_NumSamples )
	{
		//m_BufferPosition = 0;

		return false;
	}

	m_DataPtr[ m_BufferPosition ] = lVal;
	m_DataPtr[ m_BufferPosition + 1 ] = rVal;

	m_BufferPosition += 2;

	return true;
}

short* BlockBuffer::GetDataPtr( void )
{
	return m_DataPtr;
}

bool BlockBuffer::CheckStatus( void )
{
	if ( m_BufferPosition + 2 > m_NumSamples )
	{
		return false;
	}

	return true;
}