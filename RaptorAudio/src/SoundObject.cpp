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

#include "SoundObject.h"
#include "DSPChain.h"

#include <memory>

using namespace Raptor;
using namespace Raptor::Audio;

SoundObject::SoundObject( void )
	:
m_BadFile( false )
{

}

SoundObject::~SoundObject( void )
{
	delete m_AudioSource;
}

SoundObjectResults::SoundObjectResult SoundObject::AdvancePosition( void )
{
	m_Properties->sop_Position += m_AdvanceAmount * GetPitchShift();

	if ( m_Properties->sop_Position >= (double) m_BufferSize && !GetLooping() )
	{
		m_Properties->sop_Position = 0;
		return SoundObjectResults::SOUND_OBJECT_DONE;
	}

	while ( m_Properties->sop_Position >= (double) m_BufferSize )
	{
		m_Properties->sop_Position -= (double) m_BufferSize;

		if ( m_Properties->sop_Shared != 0 )
		{
			if ( m_Properties->sop_Shared->sp_DSPChain != 0 )
			{
				m_Properties->sop_Shared->sp_DSPChain->PerformRandomizePlay();
				m_Properties->sop_Shared->sp_DSPChain->PerformPerPlayProcessing( m_Properties );
			}
		}
	}

	return SoundObjectResults::SOUND_OBJECT_PLAYING;
}

const double& SoundObject::GetAdvanceAmount( void )
{
	return m_AdvanceAmount;
}

const unsigned int& SoundObject::GetBufferSize( void )
{
	return m_BufferSize;
}

const double& SoundObject::GetPosition( void )
{
	return m_Properties->sop_Position;
}

const double& SoundObject::GetPitchShift( void )
{
	if ( m_Properties->sop_Shared != 0  )
	{
		return m_Properties->sop_Shared->sp_PitchShift;
	}

	static double failPitch = 1.0;

	return failPitch;
}

SoundObject::SoundType SoundObject::GetType( void )
{
	return m_Type;
}

void SoundObject::SetVolume( float volume )
{
	if ( volume > 1.0f ) volume = 1.0f;
	else if ( volume < 0.0f ) volume = 0.0f;
	
	if ( m_Properties->sop_Shared != 0 )
	{
		m_Properties->sop_Shared->sp_Volume = volume;
	}
}

float SoundObject::GetVolume( void )
{
	if ( m_Properties->sop_Shared != 0 )
	{
		return m_Properties->sop_Shared->sp_Volume;
	}

	return 1.0f;
}

WaveFile*& SoundObject::GetWaveFileHeader( void )
{
	return m_WaveFile;
}

void SoundObject::SetAdvanceAmount( double amount )
{
	m_AdvanceAmount = amount;
}

void SoundObject::SetBufferSize( unsigned int size )
{
	m_BufferSize = size;
}

void SoundObject::SetPosition( double position )
{
	m_Properties->sop_Position = position;
}

const unsigned int& Raptor::Audio::SoundObject::GetNumberOfChannels( void )
{
	return m_NumChannels;
}

void Raptor::Audio::SoundObject::SetLooping( bool looping )
{
	m_Properties->sop_Looping = looping;
}

const bool& Raptor::Audio::SoundObject::GetLooping( void )
{
	return m_Properties->sop_Looping;
}
