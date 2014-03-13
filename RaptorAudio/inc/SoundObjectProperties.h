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

#include <memory>
#include "glm\glm.hpp"

#include "SharedPointer.h"

using namespace glm;

namespace Raptor
{
	namespace Audio
	{
		class SoundObject;
		class SoundMixer;
		class HistoryBufferObject;
		class DSPChain;

		struct _Sound3DDescription;
		struct _SharedProperties;
		struct _SoundObjectPropertiesInternal;
		struct _SoundObjectProperties;

		typedef Utility::SharedPointer< _Sound3DDescription > Sound3DDescription;
		typedef Utility::SharedPointer< _SharedProperties > SharedProperties;
		typedef Utility::SharedPointer< _SoundObjectPropertiesInternal > SoundObjectPropertiesInternal;
		typedef Utility::SharedPointer< _SoundObjectProperties > SoundObjectProperties;

		struct _Sound3DDescription
		{
			_Sound3DDescription( void )
				:
			s3d_LeftVolume( 1.0f ),
			s3d_RightVolume( 1.0f ),
			s3d_StereoInnerDistance( 5.0f ),
			s3d_StereoOuterDistance( 10.0f ),
			s3d_AttenuationMin( 0.0f ),
			s3d_AttenuationMax( 50.0f )
			{}

			vec3 s3d_Position;

			float s3d_StereoInnerDistance;
			float s3d_StereoOuterDistance;

			float s3d_AttenuationMin;
			float s3d_AttenuationMax;

		private:
			bool s3d_StereoEnabled;
			float s3d_Factor;

			float s3d_LeftVolume;
			float s3d_RightVolume;
			float s3d_LeftVolumeRaw;
			float s3d_RightVolumeRaw;

			float s3d_Distance;

			friend class SoundMixer;
			friend struct _SoundObjectPropertiesInternal;
			friend struct _SharedProperties;
		};

		struct _SharedProperties
		{
			_SharedProperties( void )
			{
				sp_PitchShift = 1.0;
				sp_Volume = 1.0f;
				sp_DSPChain = 0;
			}

			DSPChain* sp_DSPChain;

			float sp_Volume;
			double sp_PitchShift;
		};

		struct _SoundObjectProperties
		{
			_SoundObjectProperties( void )
				:
			sop_Shared( 0 ),
			sop_Sound3D( 0 ),
			sop_IsDone( false ),
			sop_IsPlaying( true ),
			sop_StopSound( false ),
			sop_AlreadyPlayed( false )
			{
				sop_EchoTime = 0;
				sop_EchoVolume = 1.0;
				sop_HistoryObject = 0;
				sop_Looping = false;

				memset( sop_SampleStore, 0, sizeof( int ) * 16 );
			}

			void Set3DInformation( Sound3DDescription sound3D )
			{
				sop_Sound3D = sound3D;
			}

			void SetHistoryBufferObject( HistoryBufferObject* hbo )
			{
				sop_HistoryObject = hbo;
			}

			void SetLooping( bool looping )
			{
				sop_Looping = looping;
			}

			bool GetLooping( void )
			{
				return sop_Looping;
			}

			SharedProperties GetSharedProperties( void )
			{
				return sop_Shared;
			}

			void SetSharedProperties( SharedProperties& sprop )
			{
				sop_Shared.Release();
				sop_Shared = sprop;
			}

		private:
			bool sop_IsDone;
			bool sop_IsPlaying;
			bool sop_StopSound;

			double sop_Position;
			double sop_EchoTime;
			double sop_EchoVolume;

			unsigned int sop_AudioID;
			bool sop_AlreadyPlayed;

			int sop_SampleStore[16];
			bool sop_Looping;

			Sound3DDescription sop_Sound3D;
			SharedProperties sop_Shared;
			SoundObject* sop_Object;
			HistoryBufferObject* sop_HistoryObject;
		};

		struct _SoundObjectPropertiesInternal
		{
			enum StoreOwner
			{
				STORE_L_LOWPASS = 0,
				STORE_R_LOWPASS,
				STORE_L_LOWPASSECHO,
				STORE_R_LOWPASSECHO
			};

			_SoundObjectPropertiesInternal( void )
				:
			sop_Shared( 0 ),
			sop_Sound3D( 0 ),
			sop_IsDone( false ),
			sop_IsPlaying( true ),
			sop_StopSound( false ),
			sop_AlreadyPlayed( false )
			{
				sop_EchoTime = 0;
				sop_EchoVolume = 1.0;
				sop_HistoryObject = 0;
				sop_Looping = false;

				memset( sop_SampleStore, 0, sizeof( int ) * 16 );
			}

			_SoundObjectPropertiesInternal( _SoundObjectPropertiesInternal* sop )
			{
				sop_Shared = sop->sop_Shared;
				sop_Object = 0;
				sop_Sound3D = sop->sop_Sound3D;
				sop_Looping = sop->sop_Looping;

				sop_IsDone = sop->sop_IsDone;
				sop_IsPlaying = sop->sop_IsPlaying;
				sop_StopSound = sop->sop_StopSound;
				sop_AlreadyPlayed = sop->sop_AlreadyPlayed;

				sop_AudioID = sop->sop_AudioID;
				sop_HistoryObject = sop->sop_HistoryObject;

				memset( sop_SampleStore, 0, sizeof( int ) * 16 );
			}

			float GetLeftVolume( void )
			{
				if ( sop_Sound3D == 0 ) return 1.0f;
				else return sop_Sound3D->s3d_LeftVolume;
			}

			float GetRightVolume( void )
			{
				if ( sop_Sound3D == 0 ) return 1.0f;
				else return sop_Sound3D->s3d_RightVolume;
			}

			bool sop_IsDone;
			bool sop_IsPlaying;
			bool sop_StopSound;

			double sop_Position;
			double sop_EchoTime;
			double sop_EchoVolume;

			unsigned int sop_AudioID;
			bool sop_AlreadyPlayed;

			int sop_SampleStore[16];
			bool sop_Looping;

			Sound3DDescription sop_Sound3D;
			SharedProperties sop_Shared;
			SoundObject* sop_Object;
			HistoryBufferObject* sop_HistoryObject;
		};
	};
};

