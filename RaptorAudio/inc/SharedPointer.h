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

#include <Windows.h>
#include <process.h>

#define MAX_REF_COUNTERS 262144

namespace Raptor
{
	namespace Utility
	{
		struct SharedPointerData
		{
			static volatile unsigned int m_RefCounterPool[ MAX_REF_COUNTERS ];
			static unsigned int m_RefCounterIndex;

			static CRITICAL_SECTION m_RefCounterPoolAccessCSec;
			static bool m_RefCounterPoolAccessCSecInitialized;
		};

		template < class T > class SharedPointer
		{
		public:
			SharedPointer( void )
				:
				m_Initialized( false ),
				m_Object( 0 ),
				m_Index( 0 )
			{}


			SharedPointer( const SharedPointer& sharedPtr )
			{
				if ( sharedPtr.m_Object == 0 ) 
				{
					m_Initialized = false;
					m_Object = 0;
					m_Index = 0;
					return;
				}

				m_Index = sharedPtr.m_Index;
				m_RefCounter = sharedPtr.m_RefCounter;
				m_Object = sharedPtr.m_Object;

				InterlockedIncrement( m_RefCounter );

				m_Initialized = true;
			}

			SharedPointer& operator=( const SharedPointer& sharedPtr )
			{
				if ( m_Initialized )
				{
					if ( sharedPtr.m_Index == m_Index ) return *this;
					Release();
				}

				if ( sharedPtr.m_Object == 0 ) 
				{
					m_Initialized = false;
					m_Object = 0;
					m_Index = 0;
					return *this;
				}

				m_Index = sharedPtr.m_Index;
				m_RefCounter = sharedPtr.m_RefCounter;
				m_Object = sharedPtr.m_Object;

				InterlockedIncrement( m_RefCounter );

				m_Initialized = true;

				return *this;
			}

			SharedPointer( T* object )
			{
				if ( !SharedPointerData::m_RefCounterPoolAccessCSecInitialized )
				{
					InitializeCriticalSection( &SharedPointerData::m_RefCounterPoolAccessCSec );
					SharedPointerData::m_RefCounterPoolAccessCSecInitialized = true;
				}

				EnterCriticalSection( &SharedPointerData::m_RefCounterPoolAccessCSec );

				m_Index = SharedPointerData::m_RefCounterIndex;
				m_RefCounter = &SharedPointerData::m_RefCounterPool[ SharedPointerData::m_RefCounterIndex ];
				*m_RefCounter = 1;

				// QQQ: Optimize this
				while ( SharedPointerData::m_RefCounterPool[ SharedPointerData::m_RefCounterIndex ] != 0 )
				{
					SharedPointerData::m_RefCounterIndex++;
					if ( SharedPointerData::m_RefCounterIndex >= MAX_REF_COUNTERS ) SharedPointerData::m_RefCounterIndex = 0;
				}

				LeaveCriticalSection( &SharedPointerData::m_RefCounterPoolAccessCSec );

				m_Object = object;
				m_Initialized = true;
			}

			~SharedPointer( void )
			{
				if ( m_Initialized )
				{
					volatile unsigned int val = InterlockedDecrement( m_RefCounter );

					if ( val == 0 )
					{
						delete m_Object;
					}
				}
			}

		public:
			operator T*( void )
			{
				return m_Object;
			}

			T* operator->( void )
			{
				return m_Object;
			}

			T* GetPtr( void )
			{
				return m_Object;
			}

			unsigned int GetIndex( void )
			{
				return m_Index;
			}

		public:
			// Use with EXTREME care!
			template < class C > SharedPointer<C> _Convert( void )
			{
				return SharedPointer<C>( *( (SharedPointer<C>*) this ) );
			}

		public:
			void Release( void )
			{
				if ( m_Initialized )
				{
					volatile unsigned int val = InterlockedDecrement( m_RefCounter );

					if ( val == 0 )
					{
						delete m_Object;
					}
				}
			}

		private:
			volatile unsigned int* m_RefCounter;
			unsigned int m_Index;
			bool m_Initialized;
			T* m_Object;
		};
	};
};