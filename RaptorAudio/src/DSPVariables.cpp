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

#include "DSPVariables.h"

using namespace Raptor;
using namespace Raptor::Audio;

std::map< std::string, DSPVariable* > DSPVariables::dv_VariableMap = std::map< std::string, DSPVariable* >();

double DoubleRandomRange( double min, double max )
{
	double res = (double) rand() / RAND_MAX;
	return min + res * ( max - min );
}

DSPVariable* DSPVariables::CreateVariableObject( std::string name, DSPVariableSemantics::DSPVariableSemantic semantic, double minVal, double maxVal )
{
	dv_VariableMap[ name ] = new DSPVariable();

	dv_VariableMap[ name ]->dv_Semantic = semantic;
	dv_VariableMap[ name ]->dv_ValueMin = minVal;
	dv_VariableMap[ name ]->dv_ValueMax = maxVal;

	return dv_VariableMap[ name ];
}

DSPVariable* DSPVariables::CreateVariableObject( std::string name, DSPVariableSemantics::DSPVariableSemantic semantic, double defaultVal )
{
	dv_VariableMap[ name ] = new DSPVariable();

	dv_VariableMap[ name ]->dv_Semantic = semantic;
	dv_VariableMap[ name ]->dv_ComposedValue = defaultVal;

	return dv_VariableMap[ name ];
}

DSPVariable* DSPVariables::GetVariableObject( std::string name )
{
	if ( dv_VariableMap.find( name ) == dv_VariableMap.end() )
	{
		return 0;
	}

	return dv_VariableMap[ name ];
}

void DSPVariables::CalculateValuePlay( DSPVariable* var )
{
	if ( var == 0 ) return;

	switch ( var->dv_Semantic )
	{
	case DSPVariableSemantics::SEMANTIC_USER_VARIABLE:
		break;
	case DSPVariableSemantics::SEMANTIC_RANDOM_RANGED_SAMPLE:
		break;
	case DSPVariableSemantics::SEMANTIC_RANDOM_RANGED_PLAY:
		var->dv_ComposedValue = DoubleRandomRange( var->dv_ValueMin, var->dv_ValueMax );
		break;
	default:
		break;
	};
}

void DSPVariables::CalculateValueSample( DSPVariable* var )
{
	if ( var == 0 ) return;

	switch ( var->dv_Semantic )
	{
	case DSPVariableSemantics::SEMANTIC_USER_VARIABLE:
		break;
	case DSPVariableSemantics::SEMANTIC_RANDOM_RANGED_PLAY:
		break;
	case DSPVariableSemantics::SEMANTIC_RANDOM_RANGED_SAMPLE:
		var->dv_ComposedValue = DoubleRandomRange( var->dv_ValueMin, var->dv_ValueMax );
		break;
	default:
		break;
	};
}

double DSPVariables::GetComposedValue( DSPVariable* var )
{
	return var->dv_ComposedValue;
}