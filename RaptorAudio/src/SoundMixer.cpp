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

#include "SoundMixer.h"
#include "DSPChain.h"

#include <Windows.h>
#include "OggDecoder.h"

using namespace Raptor;
using namespace Raptor::Audio;

SoundMixer* SoundMixer::m_Mixer = 0;

namespace Raptor
{
	namespace Audio
	{
		DWORD WINAPI BlockBufferMixerThreadEntry( void* ptr )
		{
			SoundMixer* sMix = (SoundMixer*) ptr;

			while ( WaitForSingleObject( sMix->m_MixerStopEvent, 0 ) != WAIT_OBJECT_0 )
			{
				EnterCriticalSection( &sMix->m_SoundRemoveCSec );
				EnterCriticalSection( &sMix->m_SoundListCSec );
				sMix->MixSoundList();
				LeaveCriticalSection( &sMix->m_SoundListCSec );
				LeaveCriticalSection( &sMix->m_SoundRemoveCSec );

				WaitForSingleObject( sMix->GetWaveOut()->m_WaveoutBufferSwapEvent, INFINITE );
			}

			return 1;
		}

		DWORD WINAPI SoundAdditionThreadEntry( void* ptr )
		{
			SoundMixer* sMix = (SoundMixer*) ptr;

			while ( WaitForSingleObject( sMix->m_MixerStopEvent, 5 ) != WAIT_OBJECT_0 )
			{
				while ( WaitForSingleObject( sMix->m_SoundAdditionSemaphore, 0 ) == WAIT_OBJECT_0 )
				{
					EnterCriticalSection( &sMix->m_SoundRemoveCSec );
					EnterCriticalSection( &sMix->m_TempSoundListCSec );

					if ( sMix->m_TempSounds.size() == 0 )
					{
						LeaveCriticalSection( &sMix->m_SoundListCSec );
						LeaveCriticalSection( &sMix->m_TempSoundListCSec );
						continue;
					}

					SoundObjectPropertiesInternal sound = sMix->m_TempSounds.front();
					sMix->m_TempSounds.pop_front();

					LeaveCriticalSection( &sMix->m_TempSoundListCSec );

					if ( sound->sop_Shared != 0 )
					{
						if ( sound->sop_Shared->sp_DSPChain != 0 )
						{
							if ( sound->sop_EchoVolume == 1.0 )
								sound->sop_Shared->sp_DSPChain->PerformRandomizePlay();

							sound->sop_Shared->sp_DSPChain->PerformPerPlayProcessing( sound );
						}
					}

					EnterCriticalSection( &sMix->m_SoundListCSec );
					sMix->m_Sounds.push_back( sound );
					LeaveCriticalSection( &sMix->m_SoundListCSec );
					LeaveCriticalSection( &sMix->m_SoundRemoveCSec );
				}
			}

			return 1;
		}
	};
};

void SoundMixer::StopAllSounds( void )
{
	EnterCriticalSection( &m_SoundListCSec );
	EnterCriticalSection( &m_TempSoundListCSec );
	EnterCriticalSection( &m_SoundRemoveCSec );

	for ( unsigned int i = 0; i < m_Sounds.size(); i++ )
	{
		if ( m_Sounds[i]->sop_Object->GetType() == SoundObject::SOUND_STREAMED )
		{
			delete m_Sounds[i]->sop_Object;
		}
	}

	for ( std::list< SoundObjectPropertiesInternal >::iterator i = m_TempSounds.begin(); i != m_TempSounds.end(); i++ )
	{
		if ( (*i)->sop_Object->GetType() == SoundObject::SOUND_STREAMED )
		{
			delete (*i)->sop_Object;
		}
	}
	
	m_Groups.clear();
	m_Sounds.clear();
	m_TempSounds.clear();
	
	LeaveCriticalSection( &m_SoundRemoveCSec );
	LeaveCriticalSection( &m_TempSoundListCSec );
	LeaveCriticalSection( &m_SoundListCSec );
}

unsigned int SoundMixer::GetAmountOfSounds( void )
{
	return m_Sounds.size() + m_TempSounds.size();
}

bool SoundMixer::IsDone( SoundObjectProperties& sound )
{
	SoundObjectPropertiesInternal soundInternal = sound._Convert<_SoundObjectPropertiesInternal>();
	return soundInternal->sop_IsDone;
}

bool SoundMixer::IsPlaying( SoundObjectProperties& sound )
{
	SoundObjectPropertiesInternal soundInternal = sound._Convert<_SoundObjectPropertiesInternal>();
	return soundInternal->sop_IsPlaying && !soundInternal->sop_IsDone;
}

void SoundMixer::Resume( SoundObjectProperties& sound )
{
	SoundObjectPropertiesInternal soundInternal = sound._Convert<_SoundObjectPropertiesInternal>();
	soundInternal->sop_IsPlaying = true && !soundInternal->sop_IsDone;
}

void SoundMixer::Pause( SoundObjectProperties& sound )
{
	SoundObjectPropertiesInternal soundInternal = sound._Convert<_SoundObjectPropertiesInternal>();
	soundInternal->sop_IsPlaying = false;
}

void SoundMixer::Stop( SoundObjectProperties& sound )
{
	SoundObjectPropertiesInternal soundInternal = sound._Convert<_SoundObjectPropertiesInternal>();
	soundInternal->sop_StopSound = true;
}

void SoundMixer::SetProfile( SoundMixerProfiles::SoundMixerProfile profile )
{
	m_Profile = profile;
}

void SoundMixer::SetListenerAttributes( vec3 position, vec3 forward, vec3 up )
{
	if ( TryEnterCriticalSection( &m_CameraSettingsCSec ) )
	{
		m_Forward = forward;
		m_Up = up;
		m_Position = position;

		LeaveCriticalSection( &m_CameraSettingsCSec );
	}
}

void SoundMixer::AddGroup( HistoryBufferObject* historyObject )
{
	EnterCriticalSection( &m_SoundListCSec );
	m_Groups.push_back( historyObject );
	LeaveCriticalSection( &m_SoundListCSec );
}

SoundMixer::SoundMixer( unsigned int sampleRate, unsigned int bufferSize, SoundMixerProfiles::SoundMixerProfile profile )
{
	m_Mixer = this;
	m_AttenuationFactor = 1.0f;
	m_Compression = 1.0f;
	m_Profile = profile;

	m_WaveOutDevice = new WaveoutDevice( sampleRate, bufferSize );
	WaitForSingleObject( m_WaveOutDevice->m_WaveoutReadyEvent, INFINITE );

	InitializeCriticalSection( &m_SoundListCSec );
	InitializeCriticalSection( &m_TempSoundListCSec );
	InitializeCriticalSection( &m_SoundRemoveCSec );
	InitializeCriticalSection( &m_CameraSettingsCSec );

	m_MixerStopEvent = CreateEvent( 0, true, 0, 0 );
	ResetEvent( m_MixerStopEvent );

	m_SoundAdditionSemaphore = CreateSemaphore( 0, 0, 2048, 0 );
	m_MixerThreadHandle = CreateThread( 0, 0, BlockBufferMixerThreadEntry, this, 0, 0 );

	m_SoundAdditionThreadHandle = CreateThread( 0, 0, SoundAdditionThreadEntry, this, 0, 0 );
}

SoundMixer::~SoundMixer( void )
{
	SetEvent( m_MixerStopEvent );

	WaitForSingleObject( m_MixerThreadHandle, INFINITE );
	WaitForSingleObject( m_SoundAdditionThreadHandle, INFINITE );

	CloseHandle( m_MixerThreadHandle );
	CloseHandle( m_SoundAdditionThreadHandle );
	CloseHandle( m_MixerStopEvent );
	CloseHandle( m_SoundAdditionSemaphore );

	DeleteCriticalSection( &m_SoundListCSec );
	DeleteCriticalSection( &m_TempSoundListCSec );
	DeleteCriticalSection( &m_SoundRemoveCSec );
	DeleteCriticalSection( &m_CameraSettingsCSec );

	delete m_WaveOutDevice;

	for ( unsigned int i = 0; i < m_Sounds.size(); i++ )
	{
		if ( m_Sounds[i]->sop_Object->GetType() == SoundObject::SOUND_STREAMED )
		{
			delete m_Sounds[i]->sop_Object;
		}
	}

	for ( std::list< SoundObjectPropertiesInternal >::iterator i = m_TempSounds.begin(); i != m_TempSounds.end(); i++ )
	{
		if ( (*i)->sop_Object->GetType() == SoundObject::SOUND_STREAMED )
		{
			delete (*i)->sop_Object;
		}
	}
}

void SoundMixer::InitializeMixer( unsigned int sampleRate, unsigned int bufferSize, SoundMixerProfiles::SoundMixerProfile profile )
{
	if ( m_Mixer == 0 )
	{
		m_Mixer = new SoundMixer( sampleRate, bufferSize, profile );
	}
}

SoundMixer* SoundMixer::GetMixer( void )
{
	return m_Mixer;
}

void SoundMixer::DeinitializeMixer( void )
{
	OggDecoder::DeleteSingleton();

	delete m_Mixer;
	m_Mixer = 0;
}

unsigned int g_ID = 0;

void SoundMixer::PlaySoundObject( SoundObject* soundToPlay, SharedProperties constProp )
{
	if ( soundToPlay->GetType() == SoundObject::SOUND_HISTORY ) return;

	SoundObjectPropertiesInternal newProp = new _SoundObjectPropertiesInternal();

	if ( soundToPlay->GetType() == SoundObject::SOUND_STREAMED )
	{
		StreamingSoundObject* streaming = (StreamingSoundObject*) soundToPlay;
		soundToPlay = new StreamingSoundObject( streaming->GetFilePath(), streaming->m_AudioSource->GetType(), streaming->m_AudioSource->GetLength() );
	}

	newProp->sop_Object = soundToPlay;
	newProp->sop_Position = 0.0f;
	newProp->sop_Shared = constProp;
	newProp->sop_AudioID = g_ID++;

	if ( soundToPlay->m_BadFile == false )
	{
		EnterCriticalSection( &m_TempSoundListCSec );
		newProp->sop_AlreadyPlayed = true;
		m_TempSounds.push_back( newProp );
		LeaveCriticalSection( &m_TempSoundListCSec );

		ReleaseSemaphore( m_SoundAdditionSemaphore, 1, 0 );
	}
}

void SoundMixer::PlaySoundObject( SoundObjectProperties& soundToPlay )
{
	SoundObjectPropertiesInternal soundToPlayConv = soundToPlay._Convert<_SoundObjectPropertiesInternal>();
	if ( soundToPlayConv->sop_AlreadyPlayed == true ) return;

	if ( soundToPlayConv->sop_Object->m_BadFile == false )
	{
		if ( soundToPlayConv->sop_Sound3D != 0 )
		{
			if ( soundToPlayConv->sop_Object->GetNumberOfChannels() == 2 )
			{
				soundToPlayConv->sop_Sound3D->s3d_StereoEnabled = true;
			}
		}

		EnterCriticalSection( &m_TempSoundListCSec );
		soundToPlayConv->sop_AlreadyPlayed = true;
		m_TempSounds.push_back( soundToPlayConv );
		LeaveCriticalSection( &m_TempSoundListCSec );

		ReleaseSemaphore( m_SoundAdditionSemaphore, 1, 0 );
	}
}

SoundObjectProperties SoundMixer::CreateProperties( SoundObject* soundToUse, SharedProperties constProp )
{
	if ( soundToUse->GetType() == SoundObject::SOUND_HISTORY ) return 0;

	SoundObjectPropertiesInternal newProp = new _SoundObjectPropertiesInternal();

	if ( soundToUse->GetType() == SoundObject::SOUND_STREAMED )
	{
		StreamingSoundObject* streaming = (StreamingSoundObject*) soundToUse;
		soundToUse = new StreamingSoundObject( streaming->GetFilePath(), streaming->m_AudioSource->GetType(), streaming->m_AudioSource->GetLength() );
	}

	newProp->sop_Object = soundToUse;
	newProp->sop_Position = 0.0f;
	newProp->sop_Shared = constProp;
	newProp->sop_AudioID = g_ID++;

	return newProp._Convert<_SoundObjectProperties>();
}

WaveoutDevice* SoundMixer::GetWaveOut( void )
{
	return m_WaveOutDevice;
}

void SoundMixer::Compress( int sample, double minimal )
{
	if ( sample > SHRT_MAX || sample < SHRT_MIN )
	{
		if ( m_Compression > minimal ) m_Compression -= 0.01;
	}

	else if ( m_Compression < 1.0 )
	{
		m_Compression += minimal * 0.0001;
		m_Compression = min( 1.0, m_Compression );
	}
}

void SoundMixer::MixSoundList( void )
{
	int amplL = 0;
	int amplR = 0;

	int count = 0;

	int maxCount = (int) m_BlockPlaybackBuffer->GetNumSamples() / 2;
	size_t soundSize = m_Sounds.size();

	EnterCriticalSection( &m_CameraSettingsCSec );
	vec3 right = cross( m_Forward, m_Up );
	m_TempPosition = m_Position;
	LeaveCriticalSection( &m_CameraSettingsCSec );

	float minVal = 0.25f;

	if ( m_Profile == SoundMixerProfiles::SOUND_MIXER_SPEAKERS )
	{
		minVal = 0.0f;
	}

	for ( unsigned int i = 0; i < soundSize; i++ )
	{
		SoundObjectPropertiesInternal properties = m_Sounds[i];

		if ( properties->sop_IsPlaying == false ) continue;

		if ( properties->sop_Sound3D != 0 )
		{
			if ( properties->sop_Sound3D->s3d_Position == m_TempPosition )
			{
				properties->sop_Sound3D->s3d_LeftVolume = 1.0f;
				properties->sop_Sound3D->s3d_RightVolume = 1.0f;

				properties->sop_Sound3D->s3d_Factor = 1.0f;

				continue;
			}

			vec3 direction = properties->sop_Sound3D->s3d_Position - m_TempPosition;
			vec3 nDirection = length( direction ) <= 1.0 ? direction : normalize( direction );
			float lDirection = length( direction );

			float dotVal = dot( nDirection, right );
			dotVal = ( dotVal + 1.0f ) / 2.0f;
			
			float attenuation = glm::smoothstep( properties->sop_Sound3D->s3d_AttenuationMax, properties->sop_Sound3D->s3d_AttenuationMin, lDirection );

			properties->sop_Sound3D->s3d_Distance = attenuation;

			properties->sop_Sound3D->s3d_Factor = smoothstep( properties->sop_Sound3D->s3d_StereoOuterDistance,
																properties->sop_Sound3D->s3d_StereoInnerDistance,
																lDirection );

			properties->sop_Sound3D->s3d_LeftVolumeRaw = 1.0f - dotVal;
			properties->sop_Sound3D->s3d_RightVolumeRaw = dotVal;

			properties->sop_Sound3D->s3d_LeftVolume = max( minVal * ( 1.5f - properties->sop_Sound3D->s3d_RightVolumeRaw ), properties->sop_Sound3D->s3d_LeftVolumeRaw ) * attenuation;
			properties->sop_Sound3D->s3d_RightVolume = max( minVal * ( 1.5f - properties->sop_Sound3D->s3d_LeftVolumeRaw ), properties->sop_Sound3D->s3d_RightVolumeRaw ) * attenuation;
		}
	}

	while ( count < maxCount )
	{
		if ( m_BlockPlaybackBuffer->CheckStatus() == false )
		{
			break;
		}

		for ( unsigned int i = 0; i < m_Sounds.size(); i++ )
		{
			m_Sounds[i]->sop_Object->m_Properties = m_Sounds[i];

			if ( m_Sounds[i]->sop_Object->m_Properties->sop_IsPlaying == false )
			{
				continue;
			}

			DSPChain* chain = 0;

			if ( m_Sounds[i]->sop_Shared != 0 )
				chain = m_Sounds[i]->sop_Shared->sp_DSPChain;

			int val1 = (int) ( (double) m_Sounds[i]->sop_Object->GetCurrentSample( 0 ) * m_Sounds[i]->sop_EchoVolume );
			int val2 = (int) ( (double) m_Sounds[i]->sop_Object->GetCurrentSample( 1 ) * m_Sounds[i]->sop_EchoVolume );

			if ( chain != 0 )
			{
				chain->GetSignalL() = val1;
				chain->GetSignalR() = val2;

				chain->PerformProcessing( m_Sounds[i]->sop_Object );
				
				val1 = chain->GetSignalL();
				val2 = chain->GetSignalR();
			}

			if ( m_Sounds[i]->sop_Sound3D != 0 && m_Sounds[i]->sop_Sound3D->s3d_StereoEnabled == true )
			{
				int M = ( val1 + val2 ) / 2;

				val1 = (int) ( (double) ( 1.0f - m_Sounds[i]->sop_Sound3D->s3d_Factor ) * (double) M * (double) m_Sounds[i]->GetLeftVolume() ) +
						(int) ( (double) ( m_Sounds[i]->sop_Sound3D->s3d_Factor ) * (double) val1 * (double) m_Sounds[i]->sop_Sound3D->s3d_Distance );

				val2 = (int) ( (double) ( 1.0f - m_Sounds[i]->sop_Sound3D->s3d_Factor ) * (double) M * (double) m_Sounds[i]->GetRightVolume() ) +
						(int) ( (double) ( m_Sounds[i]->sop_Sound3D->s3d_Factor ) * (double) val2 * (double) m_Sounds[i]->sop_Sound3D->s3d_Distance );

				int l = 0;
			}

			else
			{
				val1 = (int) ( (double) val1 * (double) m_Sounds[i]->GetLeftVolume() );
				val2 = (int) ( (double) val2 * (double) m_Sounds[i]->GetRightVolume() );
			}

			if ( m_Sounds[i]->sop_HistoryObject != 0 )
			{
				m_Sounds[i]->sop_HistoryObject->AccumulateSampleL( val1 );
				m_Sounds[i]->sop_HistoryObject->AccumulateSampleR( val2 );
			
				continue;
			}

			amplL += val1;
			amplR += val2;
		}

		for ( unsigned int i = 0; i < m_Groups.size(); i++ )
		{
			ChannelSamplePair pair = m_Groups[i]->GetCurrentSamples();

			amplL += pair.L;
			amplR += pair.R;

			m_Groups[i]->WriteAccumulation();
			m_Groups[i]->AdvancePosition();
		}

		amplL = (int) ( m_Compression * (double) amplL );
		amplR = (int) ( m_Compression * (double) amplR );

		Compress( amplL, 1.0 / (double) ( soundSize + 4.0 ) );
		Compress( amplR, 1.0 / (double) ( soundSize + 4.0 ) );

		if ( amplL > SHRT_MAX ) amplL = SHRT_MAX;
		if ( amplL < SHRT_MIN ) amplL = SHRT_MIN;

		if ( amplR > SHRT_MAX ) amplR = SHRT_MAX;
		if ( amplR < SHRT_MIN ) amplR = SHRT_MIN;

		m_BlockPlaybackBuffer->WriteBuffer2( amplL, amplR );

		count++;

		for ( int i = 0; i < (int) m_Sounds.size(); i++ )
		{
			m_Sounds[i]->sop_Object->m_Properties = m_Sounds[i];

			if ( m_Sounds[i]->sop_Object->m_Properties->sop_IsPlaying == false ) 
			{
				continue;
			}
			
			if ( m_Sounds[i]->sop_EchoTime > 0 )
			{
				m_Sounds[i]->sop_EchoTime--;
				continue;
			}

			SoundObjectResults::SoundObjectResult result = m_Sounds[i]->sop_Object->AdvancePosition();

			if ( result == SoundObjectResults::SOUND_OBJECT_DONE || m_Sounds[i]->sop_StopSound )
			{
				if ( m_Sounds[i]->sop_Object->GetType() == SoundObject::SOUND_STREAMED )
				{
					delete m_Sounds[i]->sop_Object;
				}

				m_Sounds[i]->sop_IsDone = true;
				m_Sounds[i]->sop_IsPlaying = false;

				m_Sounds[i] = m_Sounds.back();
				m_Sounds.pop_back();

				i--;
			}
		}

		amplL = 0;
		amplR = 0;
	}
}

void SoundMixer::SetBlockBuffer( BlockBuffer* blockBuffer )	
{
	m_BlockPlaybackBuffer = blockBuffer;
}