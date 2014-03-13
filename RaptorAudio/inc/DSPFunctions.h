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

#include "DSPChain.h"
#include "WaveoutDevice.h"

using namespace Raptor;
using namespace Raptor::Audio;

static void Amplify( unsigned int num, DSPChainEntry* entry, DSPChain* chain, void* objV )
{
	SoundObject* obj = (SoundObject*) objV;

	int L = chain->GetCurrentSignalL();
	int R = chain->GetCurrentSignalR();

	DSPVariables::CalculateValueSample( entry->dce_VariableSet.dvs_Val1 );

	double amplifyLevel = DSPVariables::GetComposedValue( entry->dce_VariableSet.dvs_Val1 ) * DSPVariables::GetComposedValue( entry->dce_VariableSet.dvs_DryWet );

	chain->OutputSignal( (int) ( L * amplifyLevel ), (int) ( R * amplifyLevel ), num );
}

static void ShiftPitch( unsigned int num, DSPChainEntry* entry, DSPChain* chain, void* objV )
{
	_SoundObjectPropertiesInternal* obj = (_SoundObjectPropertiesInternal*) objV;

	DSPVariables::CalculateValueSample( entry->dce_VariableSet.dvs_Val1 );

	double pitch = pow( 2.0, DSPVariables::GetComposedValue( entry->dce_VariableSet.dvs_Val1 ) / 12.0 );

	if ( obj->sop_Shared != 0 ) obj->sop_Shared->sp_PitchShift = pitch;
}

static void CrudeLowPass( unsigned int num, DSPChainEntry* entry, DSPChain* chain, void* objV )
{
	SoundObject* obj = (SoundObject*) objV;

	int L = 0;
	int R = 0;

	DSPVariables::CalculateValueSample( entry->dce_VariableSet.dvs_Val1 );

	if ( DSPVariables::GetComposedValue( entry->dce_VariableSet.dvs_Val1 ) < 1 )
	{
		entry->dce_VariableSet.dvs_Val1->dv_ComposedValue = 1;
	}

	unsigned int lowPassAmount = (unsigned int) DSPVariables::GetComposedValue( entry->dce_VariableSet.dvs_Val1 ); 

	if ( lowPassAmount <= 2 ) 
	{
		chain->OutputSignal( (int) ( (double) chain->GetCurrentSignalL() * DSPVariables::GetComposedValue( entry->dce_VariableSet.dvs_DryWet ) ), (int) ( (double) chain->GetCurrentSignalR() * DSPVariables::GetComposedValue( entry->dce_VariableSet.dvs_DryWet ) ), num );
		return;
	}

	if ( lowPassAmount > 32 ) lowPassAmount = 32;

	int position = (int) obj->GetPosition();
	int maxSize = obj->GetBufferSize();

	if ( obj->GetType() == SoundObject::SOUND_STREAMED )
	{
		maxSize = 16384;
	}

	for ( unsigned int i = 0; i < lowPassAmount; i++ )
	{
		position += i;

		if ( position >= maxSize )
		{
			position -= maxSize;
		}

		L += obj->GetChannelPtr( 0 )[ position ];
		R += obj->GetChannelPtr( 1 )[ position ];
	}

	L /= (int) ( lowPassAmount );
	R /= (int) ( lowPassAmount );

	chain->OutputSignal( (int) ( L * obj->m_Properties->sop_Shared->sp_Volume * DSPVariables::GetComposedValue( entry->dce_VariableSet.dvs_DryWet ) ), (int) ( R * obj->m_Properties->sop_Shared->sp_Volume * DSPVariables::GetComposedValue( entry->dce_VariableSet.dvs_DryWet ) ), num );
}

static void Decimate( unsigned int num, DSPChainEntry* entry, DSPChain* chain, void* objV )
{
	int L = chain->GetCurrentSignalL();
	int R = chain->GetCurrentSignalR();

	chain->OutputSignal( (int) ( abs( (double) L ) * DSPVariables::GetComposedValue( entry->dce_VariableSet.dvs_DryWet ) ), (int) ( abs( (double) R ) * DSPVariables::GetComposedValue( entry->dce_VariableSet.dvs_DryWet ) ), num );
}

static void CrudeLowPassEchoVolume( unsigned int num, DSPChainEntry* entry, DSPChain* chain, void* objV )
{
	SoundObject* obj = (SoundObject*) objV;

	int L = 0;
	int R = 0;

	DSPVariables::CalculateValueSample( entry->dce_VariableSet.dvs_Val1 );

	unsigned int lowPassAmount = (unsigned int) ( ( 1.0 - obj->m_Properties->sop_EchoVolume ) * DSPVariables::GetComposedValue( entry->dce_VariableSet.dvs_Val1 ) );  

	if ( lowPassAmount <= 2 ) 
	{
		chain->OutputSignal( (int) ( (double) chain->GetCurrentSignalL() * obj->m_Properties->sop_EchoVolume * DSPVariables::GetComposedValue( entry->dce_VariableSet.dvs_DryWet ) ), (int) ( (double) chain->GetCurrentSignalR() * obj->m_Properties->sop_EchoVolume * DSPVariables::GetComposedValue( entry->dce_VariableSet.dvs_DryWet ) ), num );
		return;
	}

	if ( lowPassAmount > 32 ) lowPassAmount = 32;

	int position = (int) obj->GetPosition();
	int maxSize = obj->GetBufferSize();

	if ( obj->GetType() == SoundObject::SOUND_STREAMED )
	{
		maxSize = 16384;
	}

	for ( unsigned int i = 0; i < lowPassAmount; i++ )
	{
		position += 1;

		if ( position >= maxSize )
		{
			position -= maxSize;
		}

		L += obj->GetChannelPtr( 0 )[ ( position + ( (int) ( obj->GetAdvanceAmount() * SoundMixer::GetMixer()->GetWaveOut()->GetSampleRate() ) / ( 200 + (int) ( obj->m_Properties->sop_EchoVolume * 800.0 ) ) ) ) % maxSize ]; // Haas effect
		R += obj->GetChannelPtr( 1 )[ position ];
	}

	L /= (int) ( lowPassAmount );
	R /= (int) ( lowPassAmount );

	L = (int) ( L * obj->m_Properties->sop_Shared->sp_Volume * obj->m_Properties->sop_EchoVolume * DSPVariables::GetComposedValue( entry->dce_VariableSet.dvs_DryWet ) );
	R = (int) ( R * obj->m_Properties->sop_Shared->sp_Volume * obj->m_Properties->sop_EchoVolume * DSPVariables::GetComposedValue( entry->dce_VariableSet.dvs_DryWet ) );

	chain->OutputSignal( (int) ( L + (int) ( R * 0.05 ) ), (int) ( R + (int) ( L * 0.05 ) ), num );
}

// Haas effect upon the echo
static void EchoPass( unsigned int num, DSPChainEntry* entry, DSPChain* chain, void* objV )
{
	SoundObject* obj = (SoundObject*) objV;

	int L = chain->GetCurrentSignalL();
	int R = chain->GetCurrentSignalR();

	int R2 = (int) ( obj->GetChannelPtr( 1 )[ (int) ( obj->GetPosition() + (int) ( (double) ( obj->GetAdvanceAmount() * SoundMixer::GetMixer()->GetWaveOut()->GetSampleRate() ) / 200.0 ) ) % obj->GetBufferSize() ] * obj->m_Properties->sop_EchoVolume * obj->m_Properties->sop_Shared->sp_Volume );

	R = (int) ( R * obj->m_Properties->sop_EchoVolume ) + (int) ( R2 * ( 1.0 - obj->m_Properties->sop_EchoVolume ) );

	chain->OutputSignal( (int) ( ( L + (int) ( R * 0.1 ) ) * obj->m_Properties->sop_EchoVolume ), (int) ( ( R + (int) ( L * 0.1 ) ) * obj->m_Properties->sop_EchoVolume ), num );
}

static void StereoExpand( unsigned int num, DSPChainEntry* entry, DSPChain* chain, void* objV )
{
	SoundObject* obj = (SoundObject*) objV;

	DSPVariables::CalculateValueSample( entry->dce_VariableSet.dvs_Val1 );

	int L = chain->GetCurrentSignalL();
	int R = chain->GetCurrentSignalR();

	int S = ( L - R ) / 2;
	S = (int) ( (double) S * DSPVariables::GetComposedValue( entry->dce_VariableSet.dvs_Val1 ) );

	chain->OutputSignal( L + S, R - S, num );
}

static void MonoToStereo( unsigned int num, DSPChainEntry* entry, DSPChain* chain, void* objV )
{
	SoundObject* obj = (SoundObject*) objV;

	DSPVariables::CalculateValueSample( entry->dce_VariableSet.dvs_Val1 );

	int maxSize = obj->GetBufferSize();

	if ( obj->GetType() == SoundObject::SOUND_STREAMED )
	{
		maxSize = 16384;
	}

	double position = obj->GetPosition();
	double offset = 0;

	int L = chain->GetCurrentSignalL();
	int R = L;

	offset = ( 0.00012 * DSPVariables::GetComposedValue( entry->dce_VariableSet.dvs_Val1 ) );
	offset *= ( double ) SoundMixer::GetMixer()->GetWaveOut()->GetSampleRate();
	offset *= obj->GetAdvanceAmount();

	position += offset;

	if ( DSPVariables::GetComposedValue( entry->dce_VariableSet.dvs_Val1 ) != 0.0 )
	{
		R = obj->GetChannelPtr( 1 )[ (int) position % maxSize ];
	}

	else
	{
		R = chain->GetCurrentSignalR();
	}

	int S = ( L - R ) / 2;

	chain->OutputSignal( L + S, R - S, num );
}

// Thanks to Marijn for this algorithm.
static void BetterLowPass( unsigned int num, DSPChainEntry* entry, DSPChain* chain, void* objV )
{
	SoundObject* obj = (SoundObject*) objV;

	DSPVariables::CalculateValueSample( entry->dce_VariableSet.dvs_Val1 );

	int L = chain->GetCurrentSignalL();
	int R = chain->GetCurrentSignalR();

	double amount = DSPVariables::GetComposedValue( entry->dce_VariableSet.dvs_Val1 ) / 100.0;

	obj->m_Properties->sop_SampleStore[ _SoundObjectPropertiesInternal::STORE_L_LOWPASS ] = (int) ( (double) obj->m_Properties->sop_SampleStore[ _SoundObjectPropertiesInternal::STORE_L_LOWPASS ] * ( amount ) + ( (double) L * ( 1.0 - amount ) ) ); 
	obj->m_Properties->sop_SampleStore[ _SoundObjectPropertiesInternal::STORE_R_LOWPASS ] = (int) ( (double) obj->m_Properties->sop_SampleStore[ _SoundObjectPropertiesInternal::STORE_R_LOWPASS ] * ( amount ) + ( (double) R * ( 1.0 - amount ) ) ); 

	chain->OutputSignal( obj->m_Properties->sop_SampleStore[ _SoundObjectPropertiesInternal::STORE_L_LOWPASS ], obj->m_Properties->sop_SampleStore[ _SoundObjectPropertiesInternal::STORE_R_LOWPASS ], num );
}

static void BetterLowPassEchoVolume( unsigned int num, DSPChainEntry* entry, DSPChain* chain, void* objV )
{
	SoundObject* obj = (SoundObject*) objV;

	int L = 0;
	int R = chain->GetCurrentSignalR();

	double position = obj->m_Properties->sop_Position;
	position = (double) ( position - ( 1.0 - obj->m_Properties->sop_EchoVolume ) * ( 0.01 * (double) SoundMixer::GetMixer()->GetWaveOut()->GetSampleRate() )  * obj->GetAdvanceAmount() ); // Haas effect

	while ( position < 0 )
	{
		position += obj->GetBufferSize();
	}

	L = obj->GetChannelPtr(0)[ (unsigned int) position ];

	double amount = 0.0;

	if ( obj->m_Properties->sop_EchoVolume < 1.0 )
	{
		amount = ( 1.00 - obj->m_Properties->sop_EchoVolume ) + 0.05;
		amount = min( 0.98, amount );
		amount = max( 0.0, amount );
	}

	obj->m_Properties->sop_SampleStore[ _SoundObjectPropertiesInternal::STORE_L_LOWPASSECHO ] = (int) ( (double) obj->m_Properties->sop_SampleStore[ _SoundObjectPropertiesInternal::STORE_L_LOWPASSECHO ] * ( amount ) + ( (double) L * ( 1.0 - amount ) ) ); 
	obj->m_Properties->sop_SampleStore[ _SoundObjectPropertiesInternal::STORE_R_LOWPASSECHO ] = (int) ( (double) obj->m_Properties->sop_SampleStore[ _SoundObjectPropertiesInternal::STORE_R_LOWPASSECHO ] * ( amount ) + ( (double) R * ( 1.0 - amount ) ) ); 

	chain->OutputSignal( obj->m_Properties->sop_SampleStore[ _SoundObjectPropertiesInternal::STORE_L_LOWPASSECHO ], obj->m_Properties->sop_SampleStore[ _SoundObjectPropertiesInternal::STORE_R_LOWPASSECHO ], num );
}

static void StereoExpandEchoVolume( unsigned int num, DSPChainEntry* entry, DSPChain* chain, void* objV )
{
	SoundObject* obj = (SoundObject*) objV;

	int L = chain->GetCurrentSignalL();
	int R = chain->GetCurrentSignalR();

	int S = ( L - R ) / 2;
	S = (int) ( (double) S * ( 1.0 - obj->m_Properties->sop_EchoVolume ) );

	chain->OutputSignal( L + S, R - S, num );
}