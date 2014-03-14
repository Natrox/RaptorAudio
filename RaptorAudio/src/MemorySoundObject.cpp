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

#include "MemorySoundObject.h"
#include "MemorySoundObjectWavImpl.h"

#include <iomanip> 
#include <string>
#include <sstream>

using namespace Raptor;
using namespace Raptor::Audio;

MemorySoundObject::MemorySoundObject( const char* filePath, WaveoutDevice* wvOut )
	:
SoundObject(),
m_SoundObjectImpl( 0 )
{
	FILE* tempRead = fopen( filePath, "rb" );

	m_Type = SoundObject::SOUND_MEMORY;

	if ( tempRead == 0 )
	{
		printf( "%s : File does not exist!\n", filePath );
		m_BadFile = true;
		return;
	}

	char magic[4];

	fread( magic, 4, 1, tempRead );
	fclose( tempRead );
	
	if ( strncmp( magic, "RIFF", 4 ) == 0 )
	{
		m_SoundObjectImpl = new MemorySoundObjectWavImpl( filePath, wvOut, this );
		m_NumChannels = m_SoundObjectImpl->m_NumChannels;
		return;
	}

	else if ( strncmp( magic, "OggS", 4 ) == 0 )
	{
		// TODO
	}

	m_SoundObjectImpl = 0;
	m_BadFile = true;

	printf( "%s : Not a compatible file!\n", filePath );
	return;
}

MemorySoundObject::~MemorySoundObject( void )
{
	delete m_SoundObjectImpl;
}

short MemorySoundObject::GetCurrentSample( unsigned int num )
{
	return m_SoundObjectImpl->GetCurrentSample( num );
}

const short* MemorySoundObject::GetChannelPtr( unsigned int num )
{
	return m_SoundObjectImpl->GetChannelPtr( num );
}