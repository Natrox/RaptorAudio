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

#include "StreamingSoundObject.h"
#include "StreamingSoundObjectWavImpl.h"
#include "StreamingSoundObjectOggImpl.h"

#include <iomanip> 
#include <string>
#include <sstream>
#include "AudioSourceFile.h"
#include "AudioSourceMemory.h"

using namespace Raptor;
using namespace Raptor::Audio;

StreamingSoundObject::StreamingSoundObject( const char* fileNameOrPtr, AudioOrigins::AudioOrigin origin, size_t length )
		:
SoundObject(),
m_SoundObjectImpl( 0 )
{
	const char* ptrChoice;

	switch ( origin )
	{
	case AudioOrigins::AUDIO_ORIGIN_FILE:
		ptrChoice = fileNameOrPtr;
		m_AudioSource = new AudioSourceFile( fileNameOrPtr );
		break;

	case AudioOrigins::AUDIO_ORIGIN_OPENMEMORY:
		m_AudioSource = new AudioSourceMemory( fileNameOrPtr, length, origin );
		ptrChoice = (const char*) m_AudioSource->GetPtrData();
		break;

	case AudioOrigins::AUDIO_ORIGIN_OPENMEMORY_POINT:
		ptrChoice = fileNameOrPtr;
		m_AudioSource = new AudioSourceMemory( fileNameOrPtr, length, origin );
		break;
	};

	if ( !m_AudioSource->CheckIfLoaded() )
	{
		printf( "%s : File does not exist!\n", fileNameOrPtr );
		m_BadFile = true;
		return;
	}

	m_Type = SoundObject::SOUND_STREAMED;

	char magic[4];

	m_AudioSource->Read( magic, 4, 1 );
	m_AudioSource->Seek( 0, SeekOrigins::SEEK_ORIGIN_SET );

	if ( strncmp( magic, "RIFF", 4 ) == 0 )
	{
		m_SoundObjectImpl = new StreamingSoundObjectWavImpl( this );
		m_SoundObjectImpl->m_FilePath = ptrChoice;
		m_NumChannels = m_SoundObjectImpl->m_NumChannels;
		return;
	}

	else if ( strncmp( magic, "OggS", 4 ) == 0 )
	{
		m_SoundObjectImpl = new StreamingSoundObjectOggImpl( this );
		m_SoundObjectImpl->m_FilePath = ptrChoice;
		m_NumChannels = m_SoundObjectImpl->m_NumChannels;
		return;
	}

	m_SoundObjectImpl = 0;

	m_BadFile = true;

	printf( "%s : Not a compatible file!\n", fileNameOrPtr );
	return;
}

StreamingSoundObject::~StreamingSoundObject( void )
{
	delete m_SoundObjectImpl;
}

short StreamingSoundObject::GetCurrentSample( unsigned int num )
{
	return m_SoundObjectImpl->GetCurrentSample( num );
}

const short* StreamingSoundObject::GetChannelPtr( unsigned int num )
{
	return m_SoundObjectImpl->GetChannelPtr( num );
}

bool StreamingSoundObject::UpdateBuffer( void )
{
	return m_SoundObjectImpl->UpdateBuffer();
}

void StreamingSoundObject::Update( void )
{
	m_SoundObjectImpl->Update();
}

SoundObjectResults::SoundObjectResult StreamingSoundObject::AdvancePosition( void )
{
	return m_SoundObjectImpl->AdvancePosition();
}