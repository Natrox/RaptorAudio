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

#include "AudioSourceFile.h"

using namespace Raptor::Audio;

static int g_SeekTranslationTable[3][2] =
{
	{ SeekOrigins::SEEK_ORIGIN_SET, SEEK_SET },
	{ SeekOrigins::SEEK_ORIGIN_CUR, SEEK_CUR },
	{ SeekOrigins::SEEK_ORIGIN_END, SEEK_END }
};

AudioSourceFile::AudioSourceFile( const char* fileNameOrPtr )
	:
AudioSource( fileNameOrPtr, AudioOrigins::AUDIO_ORIGIN_FILE ),
m_File( 0 )
{
	m_File = fopen( fileNameOrPtr, "rb" );
}

AudioSourceFile::~AudioSourceFile( void )
{
	if ( m_File != 0 ) fclose( m_File );
}

size_t AudioSourceFile::Read( void* dstBuf, size_t elementSize, size_t count )
{
	return fread( dstBuf, elementSize, count, m_File );
}

int AudioSourceFile::Seek( long int offset, int seekOrigin )
{
	return fseek( m_File, offset, g_SeekTranslationTable[ seekOrigin ][1] );
}

long AudioSourceFile::Tell( void )
{
	return ftell( m_File );
}

int AudioSourceFile::Error( void )
{
	return ferror( m_File );
}

bool AudioSourceFile::CheckIfLoaded( void )
{
	return m_File != 0;
}

void AudioSourceFile::Close( void )
{
	fclose( m_File );
	m_File = 0;
}

void* AudioSourceFile::GetPtrData( void )
{
	return m_File;
}
