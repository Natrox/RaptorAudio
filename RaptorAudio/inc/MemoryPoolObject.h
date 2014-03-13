/*

  ICECAT Memory Pool Object by Sam Hardeman / 110157
  v 1.0.0

  Inheritable memory pool object. Each type has it its own automatically created memory pool.	

  Usage: Inherit from ( public MemoryPoolObject<class, pool_size> ) and use new / delete as always.

  Remarks:

	In Debug, if new does not succeed ( because the memory pool is full ), memory will be allocated on the heap using malloc.
	In Release, if new does not succeed, a void* with value 0 will be returned.

	If you intend to allow allocation on the heap in both Debug and Release (without warnings), please define ALLOW_HEAP_ALLOCATION 
	before including this header. Please be aware that allocating many objects outside of the memory pool using this class will 
	be a lot slower than using ::new for these objects. Also note that MemoryPoolObject::delete performs an additional check if
	ALLOW_HEAP_ALLOCATION is defined, which introduces a small negative performance impact. Consider increasing the PoolSize instead.

*/

#pragma once

#ifdef _DEBUG
	// Already includes <memory>
	#include <typeinfo>
#else
	#include <memory>
#endif

#define NOT_IN_POOL -1

template <typename Obj, unsigned int PoolSize> class MemoryPoolObject;

template <typename Obj> class FixedMemoryChunk
{
private:
	FixedMemoryChunk( void )
	{
		m_Data = malloc( sizeof( Obj ) );
		m_Occupied = false;
	}

	~FixedMemoryChunk( void )
	{
		free( m_Data );
	}

	void* m_Data;
	bool m_Occupied;

protected:
	template <typename Obj, unsigned int PoolSize> friend class MemoryPoolObject;
};

template <typename Obj, unsigned int PoolSize> class MemoryPoolObject
{
public:
	void* operator new ( size_t size )
	{
		void* retval = 0;

		for ( unsigned int i = 0; i < PoolSize; i++ )
		{
			if ( m_PoolObjects[i].m_Occupied == false )
			{
				m_PoolObjects[i].m_Occupied = true;
				retval = m_PoolObjects[i].m_Data;

				MemoryPoolObject<Obj, PoolSize>* object = (MemoryPoolObject<Obj, PoolSize>*) retval;

				object->m_ChunkNumber = i;
				break;
			}
		}

#if defined( _DEBUG ) || defined( ALLOW_HEAP_ALLOCATION )

		if ( retval == 0 )
		{
			retval = malloc( size );
			MemoryPoolObject<Obj, PoolSize>* object = (MemoryPoolObject<Obj, PoolSize>*) retval;

	#ifndef ALLOW_HEAP_ALLOCATION

			const char* type = typeid( Obj ).name();
			printf( "Warning! Instance of '%s' does not fit in the memory pool! Increase the PoolSize (currently %d) for this pool object.\n", type, PoolSize );

	#endif

			object->m_ChunkNumber = NOT_IN_POOL;
		}

#endif

		return retval;
	}

	void operator delete( void* ptr )
	{
		MemoryPoolObject<Obj, PoolSize>* object = (MemoryPoolObject<Obj, PoolSize>*) ptr;

#if defined( _DEBUG ) || defined( ALLOW_HEAP_ALLOCATION )

		if ( object->m_ChunkNumber == NOT_IN_POOL )
		{
			free( ptr );
			return;
		}

#endif

		m_PoolObjects[object->m_ChunkNumber].m_Occupied = false;
	}

protected:
	MemoryPoolObject( void )
	{}

	~MemoryPoolObject( void )
	{}

private:
	int m_ChunkNumber;
	static FixedMemoryChunk<Obj> m_PoolObjects[PoolSize];
};

template <typename Obj, unsigned int PoolSize> FixedMemoryChunk<Obj> MemoryPoolObject<Obj, PoolSize>::m_PoolObjects[PoolSize];