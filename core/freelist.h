#pragma once
#ifndef CORE_FREELIST_H_
#define CORE_FREELIST_H_ 1

namespace Core {

// enabled a view of free list chain in debug
#define USE_DEBUG_CRACKER

// NOT MT safe in general
// keeps freelist always big enough for all data
// TODO make this std-a-like interface 
template< typename Type, typename IndexType = uintptr_t >
class ExplicitFreeList {
public:
	static const IndexType InvalidIndex = ~0;

	typedef typename std::vector< Type >::value_type value_type;
	typedef typename std::vector< Type >::size_type size_type;
	typedef typename std::vector< Type >::difference_type difference_type;
	typedef typename std::vector< Type >::pointer pointer;
	typedef typename std::vector< Type >::const_pointer const_pointer;
	typedef typename std::vector< Type >::reference reference;
	typedef typename std::vector< Type >::const_reference const_reference;

	ExplicitFreeList() : capacity(0), current(0), currentFree(0) {}

	explicit ExplicitFreeList( size_type _count ) : data( _count ), freelist( _count ), currentFree( _count ) {
				for( size_type i = 0; i < _count; ++i ) { 
					freelist[i] = i; 
				}

			}

	IndexType push( const value_type& _val ) {
		IndexType index = alloc();
		data[ index ] = _val;
		return index;
	}

	IndexType alloc() {
		IndexType index;
		if( currentFree == 0 ) {
			resize( (data.size() * 2) + 1 );
		}
		assert( currentFree != 0 );
		index = freelist[--currentFree];
		assert( index != InvalidIndex);
		freelist[ currentFree ] = InvalidIndex;

		return index;
	}

	void resize( const size_type _count ) {
		size_type oldcount = data.size();
		assert( _count > oldcount );
		data.resize( _count );
		freelist.resize( _count );
		for( size_type i = oldcount; i < _count; ++i ) { 
			freelist[i] = InvalidIndex;
			freelist[currentFree++] = i; 
		}
	}

	bool empty() const { return currentFree == freelist.size(); }

	void erase( const IndexType _index ) {
		assert( _index < freelist.size() );
		freelist[ currentFree++ ] = _index;
	}

	Type& get( const IndexType _index ) { return data[_index]; }
	const Type& get( const IndexType _index ) const { return data[_index]; }

private:
	std::vector< TYPE >				data;
	std::vector< INDEX_TYPE > 		freelist;
	IndexType						currentFree;
};

// NOT MT safe in general
// keeps freelist always big enough for all data
// TODO make this std-a-like interface 
template< typename Type,typename IndexType = uintptr_t >
class FreeList {
public:
	static_assert( sizeof(Type) >= sizeof(IndexType), "IndexType must be less than size in bytes of the Type being stored" );
	static const IndexType InvalidIndex = ~0;

	typedef typename std::vector< Type >::value_type value_type;
	typedef typename std::vector< Type >::size_type size_type;
	typedef typename std::vector< Type >::difference_type difference_type;
	typedef typename std::vector< Type >::pointer pointer;
	typedef typename std::vector< Type >::const_pointer const_pointer;
	typedef typename std::vector< Type >::reference reference;
	typedef typename std::vector< Type >::const_reference const_reference;

	FreeList() : capacity(0), current(0), currentFree(0) {}

	explicit FreeList( size_type _count ) : data( _count ) {

#if defined(USE_DEBUG_CRACKER)
		debugCracker = reinterpret_cast<DebugCracker*>( &data[0] );
#endif
		
		// make each entry point to the next one
		for( size_type i = 0; i < _count-1; ++i ) {
			IndexType* freeptr = reinterpret_cast<IndexType*>( &data[i] );
			*freeptr = i + 1;
		}
		// tail of the list
		IndexType* freeptr = reinterpret_cast<IndexType*>( &data[_count-1] );
		*freeptr = InvalidIndex;
		// head of the list
		freeHead = 0;
		freeCount = _count;
	}

	IndexType push( const value_type& _val ) {
		IndexType index = alloc();
		data[ index ] = _val;
		return index;
	}

	IndexType alloc() {
		if( freeHead == InvalidIndex ) {
			resize( (data.size() * 2) + 1 );
		}
		assert( freeHead != InvalidIndex );

		IndexType oldHead = freeHead;
		freeHead = *reinterpret_cast<IndexType*>( &data[ freeHead ] );
		--freeCount;
		return oldHead;
	}

	void resize( const size_type _count ) {
		size_type oldcount = data.size();
		assert( _count > oldcount );
		data.resize( _count );
#if defined(USE_DEBUG_CRACKER)
		debugCracker = reinterpret_cast<DebugCracker*>( data.data() );
#endif
		for( size_type i = oldcount; i < _count-1; ++i ) { 
			*reinterpret_cast<IndexType*>( &data[i] ) = i + 1;
		}
		*reinterpret_cast<IndexType*>( &data[ _count -1 ] ) = freeHead;
		freeHead = oldcount;
		freeCount += (_count - oldcount);
	}

	bool empty() const { return freeCount == data.size(); }

	void erase( const IndexType _index ) {
		*reinterpret_cast<IndexType*>( &data[ _index ] ) = freeHead;
		freeHead = _index;
		freeCount++;
	}

	Type& get( const IndexType _index ) { return data[ _index ]; }
	const Type& get( const IndexType _index ) const { return data[ _index ]; }

private:
	std::vector<Type>				data;
	IndexType						freeHead;
	size_type						freeCount;
#if defined(USE_DEBUG_CRACKER)
	union DebugCracker {
		Type						data;
		IndexType					index;
	} *debugCracker;
#endif

};

} // end Core namespace

#endif