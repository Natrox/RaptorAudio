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

#include <string>
#include <map>

namespace Raptor
{
	namespace Audio
	{
		namespace DSPVariableSemantics
		{
			enum DSPVariableSemantic
			{
				SEMANTIC_USER_VARIABLE,
				SEMANTIC_RANDOM_RANGED_PLAY,
				SEMANTIC_RANDOM_RANGED_SAMPLE
			};
		};

		struct DSPVariable
		{
			DSPVariable( void )
			{
				dv_Semantic = DSPVariableSemantics::SEMANTIC_USER_VARIABLE;

				dv_ValueMin = 0;
				dv_ValueMax = 0;
				dv_ComposedValue = 0;
			};

			void SetValue( double val )
			{
				dv_ComposedValue = val;
			}

			double dv_ValueMin;
			double dv_ValueMax;

			double dv_ComposedValue;

			DSPVariableSemantics::DSPVariableSemantic dv_Semantic;
		};

		struct DSPVariableSet
		{
			DSPVariableSet( void )
			{
				static DSPVariable dspVar;
				static DSPVariable dspVar2;

				dspVar2.SetValue( 1.0 );

				dvs_Val1 = &dspVar;
				dvs_Val2 = &dspVar;
				dvs_Val3 = &dspVar;

				dvs_DryWet = &dspVar2;
			}

			DSPVariable* dvs_Val1;
			DSPVariable* dvs_Val2;
			DSPVariable* dvs_Val3;

			DSPVariable* dvs_DryWet;
		};

		struct DSPVariables
		{
			static std::map< std::string, DSPVariable* > dv_VariableMap;

			static DSPVariable* CreateVariableObject( std::string name, DSPVariableSemantics::DSPVariableSemantic semantic, double minVal, double maxVal );
			static DSPVariable* CreateVariableObject( std::string name, DSPVariableSemantics::DSPVariableSemantic semantic, double defaultVal );

			static DSPVariable* GetVariableObject( std::string name );

			static void CalculateValuePlay( DSPVariable* var );
			static void CalculateValueSample( DSPVariable* var );
			static double GetComposedValue( DSPVariable* var );
		};
	};
};