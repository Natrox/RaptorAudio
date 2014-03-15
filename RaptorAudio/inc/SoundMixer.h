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

#include "SoundObject.h"
#include "StreamingSoundObject.h"
#include "MemorySoundObject.h"

#include "WaveoutDevice.h"

#include <vector>
#include <list>

#include "BlockBuffer.h"
#include "HistoryBufferObject.h"

namespace Raptor
{
	namespace Audio
	{
		namespace SoundMixerBufferingModes
		{
			enum SoundMixerBufferingMode
			{
				BUFFERING_RING,
				BUFFERING_BLOCKS
			};
		};

		namespace SoundMixerProfiles
		{
			enum SoundMixerProfile
			{
				SOUND_MIXER_SPEAKERS,
				SOUND_MIXER_HEADPHONES
			};
		};

		class SoundMixer
		{
		public:
			SoundMixer( unsigned int sampleRate, unsigned int bufferSize, SoundMixerBufferingModes::SoundMixerBufferingMode bufferMode, SoundMixerProfiles::SoundMixerProfile profile = SoundMixerProfiles::SOUND_MIXER_SPEAKERS );

		private:
			~SoundMixer( void );

		public:
			static void InitializeMixer( unsigned int sampleRate, unsigned int bufferSize, SoundMixerBufferingModes::SoundMixerBufferingMode bufferMode, SoundMixerProfiles::SoundMixerProfile profile = SoundMixerProfiles::SOUND_MIXER_SPEAKERS );
			static SoundMixer* GetMixer( void );
			static void DeinitializeMixer( void );

		public:
			void StopAllSounds( void );

		public:
			void AddGroup( HistoryBufferObject* historyObject );
			void SetListenerAttributes( vec3 position, vec3 forward, vec3 up );
			void SetProfile( SoundMixerProfiles::SoundMixerProfile profile );
			unsigned int GetAmountOfSounds( void );

		public:
			bool IsDone( SoundObjectProperties& sound );
			bool IsPlaying( SoundObjectProperties& sound );

		public:
			void Resume( SoundObjectProperties& sound );
			void Pause( SoundObjectProperties& sound );
			void Stop( SoundObjectProperties& sound );

		public:
			void PlaySoundObject( SoundObject* soundToPlay, SharedProperties constProp = SharedProperties() );
			void PlaySoundObject( SoundObjectProperties& soundToPlay );

			void SetBlockBuffer( BlockBuffer* blockBuffer );

			SoundObjectProperties CreateProperties( SoundObject* soundToUse, SharedProperties constProp = SharedProperties() );

		public:
			WaveoutDevice* GetWaveOut( void );

		private:
			CRITICAL_SECTION m_SoundListCSec;
			CRITICAL_SECTION m_TempSoundListCSec;
			CRITICAL_SECTION m_SoundRemoveCSec;
			CRITICAL_SECTION m_CameraSettingsCSec;
			void MixSoundList( void );

		private:
			LARGE_INTEGER m_TickRate;

		private:
			HANDLE m_MixerThreadHandle;
			HANDLE m_SoundAdditionThreadHandle;
			HANDLE m_MixerStopEvent;
			HANDLE m_SoundAdditionSemaphore;

			friend DWORD WINAPI RingBufferMixerThreadEntry( void* ptr );
			friend DWORD WINAPI BlockBufferMixerThreadEntry( void* ptr );
			friend DWORD WINAPI SoundAdditionThreadEntry( void* ptr );

		private:
			WaveoutDevice* m_WaveOutDevice;
			RingBuffer* m_RingPlaybackBuffer;
			SoundMixerProfiles::SoundMixerProfile m_Profile;

		public:
			BlockBuffer* m_BlockPlaybackBuffer;
			unsigned int m_BufferSize;
		private:
			SoundMixerBufferingModes::SoundMixerBufferingMode m_BufferType;

			std::vector< HistoryBufferObject* > m_Groups;
			std::vector< SoundObjectPropertiesInternal > m_Sounds;
			std::list< SoundObjectPropertiesInternal > m_TempSounds;

		private:
			vec3 m_TempPosition;
			vec3 m_Forward;
			vec3 m_Up;
			vec3 m_Position;

			float m_AttenuationFactor;

		private:
			static SoundMixer* m_Mixer;
		};
	};
};

