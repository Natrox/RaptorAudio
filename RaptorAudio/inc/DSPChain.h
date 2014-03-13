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

#include <vector>

#include "DSPVariables.h"
#include "SoundObject.h"
#include "SoundMixer.h"

namespace Raptor
{
	namespace Audio
	{
		class DSPChain;
		struct DSPChainEntry;

		typedef void (*DSPFunction)( unsigned int, DSPChainEntry*, DSPChain*, void* );

		namespace DSPFunctionSemantics
		{
			enum DSPFunctionSemantic
			{
				SEMANTIC_CARRY_SIGNAL,
				SEMANTIC_SUBSTITUTE_SIGNAL,
				SEMANTIC_ADDITIVE_SIGNAL,
				SEMANTIC_SUBTRACTIVE_SIGNAL,
				SEMANTIC_NO_SIGNAL,
				SEMANTIC_PERFORM_PER_PLAY
			};
		};

		struct DSPChainEntry
		{
			mutable bool dce_Enabled;

			DSPFunction dce_Function;
			DSPVariableSet dce_VariableSet;
			DSPFunctionSemantics::DSPFunctionSemantic dce_Semantic;
			std::string dce_Identifier;
		};

		static short Clip( int a )
		{
			if ( a > SHRT_MAX )
			{
				a = SHRT_MAX;
			}

			else if ( a < SHRT_MIN )
			{
				a = SHRT_MIN;
			}

			return (short) a;
		}

		static short ClippedMix( int a, int b )
		{
			int maxVal = 0;

			if ( a > 0 && b > 0 )
				maxVal = ( ( ( (int) a * (int) b ) >> 15 ) + 1 );
				
			else if ( a < 0 && b < 0 )
				maxVal = -( ( ( (int) a * (int) b ) >> 15 ) + 1 );

			int val = ( ( a + b ) ) - maxVal;
			
			if ( val > SHRT_MAX ) val = SHRT_MAX;
			if ( val < SHRT_MIN ) val = SHRT_MIN;

			return (short) ( ( a + b ) ) - maxVal;
		}

		class DSPChain
		{
		public:
			DSPChain( void );
			~DSPChain( void );

		public:
			void AddToChain( std::string dspIdentifier, DSPFunctionSemantics::DSPFunctionSemantic semantic, DSPFunction function, DSPVariableSet varSet );

			short& GetSignalL( void );
			short& GetSignalR( void );

			short& GetCarryL( void );
			short& GetCarryR( void );

			short GetCurrentSignalL( void );
			short GetCurrentSignalR( void );

			void OutputSignal( int L, int R, unsigned int dspNum );

			void PerformProcessing( SoundObject* obj );
			void PerformPerPlayProcessing( SoundObjectPropertiesInternal obj );
			void PerformRandomizePlay( void );

			const DSPChainEntry* const GetEffectEntry( std::string identifier );

		private:
			short m_SignalL;
			short m_SignalR;

			short m_CarryL;
			short m_CarryR;

			bool m_CarryFlag;

			std::vector< DSPChainEntry* > m_DSPChainEntries;
			friend class SoundMixer;
		};
	};
};