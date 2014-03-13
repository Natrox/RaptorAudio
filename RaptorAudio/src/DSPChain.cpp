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

using namespace Raptor;
using namespace Raptor::Audio;

DSPChain::DSPChain( void )
	:
m_SignalL( 0 ),
m_SignalR( 0 ),
m_CarryL( 0 ),
m_CarryR( 0 ),
m_CarryFlag( false )
{}

DSPChain::~DSPChain( void )
{
	for ( unsigned int i = 0; i < m_DSPChainEntries.size(); i++ )
	{
		delete m_DSPChainEntries[i];
	}
}

void DSPChain::AddToChain( std::string dspIdentifier, DSPFunctionSemantics::DSPFunctionSemantic semantic, DSPFunction function, DSPVariableSet varSet )
{
	DSPChainEntry* newEntry = new DSPChainEntry();

	newEntry->dce_Identifier = dspIdentifier;
	newEntry->dce_VariableSet = varSet;
	newEntry->dce_Semantic = semantic;
	newEntry->dce_Function = function;
	newEntry->dce_Enabled = true;

	m_DSPChainEntries.push_back( newEntry );
}

short& DSPChain::GetSignalL( void )
{
	return m_SignalL;
}

short& DSPChain::GetSignalR( void )
{
	return m_SignalR;
}

short& DSPChain::GetCarryL( void )
{
	return m_CarryL;
}

short& DSPChain::GetCarryR( void )
{
	return m_CarryR;
}

short DSPChain::GetCurrentSignalL( void )
{
	if ( m_CarryFlag )
	{
		return m_CarryL;
	}

	return m_SignalL;
}

short DSPChain::GetCurrentSignalR( void )
{
	if ( m_CarryFlag )
	{
		return m_CarryR;
	}

	return m_SignalR;
}

void DSPChain::OutputSignal( int L, int R, unsigned int dspNum )
{
	DSPChainEntry* var = m_DSPChainEntries[ dspNum ];

	switch ( var->dce_Semantic )
	{
	case DSPFunctionSemantics::SEMANTIC_CARRY_SIGNAL:
		m_CarryL = Clip( L );
		m_CarryR = Clip( R );
		m_CarryFlag = true;
		break;

	case DSPFunctionSemantics::SEMANTIC_SUBSTITUTE_SIGNAL:
		m_SignalL = Clip( L );
		m_SignalR = Clip( R );
		m_CarryFlag = false;
		break;

	case DSPFunctionSemantics::SEMANTIC_ADDITIVE_SIGNAL:
		m_SignalL = ClippedMix( L, m_SignalL );
		m_SignalR = ClippedMix( R, m_SignalR );
		m_CarryFlag = false;
		break;

	case DSPFunctionSemantics::SEMANTIC_SUBTRACTIVE_SIGNAL:
		m_SignalL = ClippedMix( -L, m_SignalL );
		m_SignalR = ClippedMix( -R, m_SignalR );
		m_CarryFlag = false;
		break;

	case DSPFunctionSemantics::SEMANTIC_NO_SIGNAL:
	default:
		break;
	}
}

void DSPChain::PerformProcessing( SoundObject* obj )
{
	for ( unsigned int i = 0; i < m_DSPChainEntries.size(); i++ )
	{
		if ( m_DSPChainEntries[i]->dce_Enabled == false ) continue;

		if ( m_DSPChainEntries[i]->dce_Semantic != DSPFunctionSemantics::SEMANTIC_PERFORM_PER_PLAY )
		{
			m_DSPChainEntries[i]->dce_Function( i, m_DSPChainEntries[i], this, obj );
		}
	}
}

void DSPChain::PerformPerPlayProcessing( SoundObjectPropertiesInternal obj )
{
	for ( unsigned int i = 0; i < m_DSPChainEntries.size(); i++ )
	{
		if ( m_DSPChainEntries[i]->dce_Enabled == false ) continue;

		if ( m_DSPChainEntries[i]->dce_Semantic == DSPFunctionSemantics::SEMANTIC_PERFORM_PER_PLAY )
		{
			m_DSPChainEntries[i]->dce_Function( i, m_DSPChainEntries[i], this, obj );
		}
	}
}

void DSPChain::PerformRandomizePlay( void )
{
	for ( unsigned int i = 0; i < m_DSPChainEntries.size(); i++ )
	{
		DSPVariables::CalculateValuePlay( m_DSPChainEntries[i]->dce_VariableSet.dvs_DryWet );
		DSPVariables::CalculateValuePlay( m_DSPChainEntries[i]->dce_VariableSet.dvs_Val1 );
		DSPVariables::CalculateValuePlay( m_DSPChainEntries[i]->dce_VariableSet.dvs_Val2 );
		DSPVariables::CalculateValuePlay( m_DSPChainEntries[i]->dce_VariableSet.dvs_Val3 );
	}
}

const DSPChainEntry* const DSPChain::GetEffectEntry( std::string identifier )
{
	for ( unsigned int i = 0; i < m_DSPChainEntries.size(); i++ )
	{
		if ( identifier.compare( m_DSPChainEntries[i]->dce_Identifier ) == 0 )
		{
			return m_DSPChainEntries[i];
		}
	}

	return 0;
}
