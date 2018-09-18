#pragma once
#ifndef CORE_FREELIST_H_
#define CORE_FREELIST_H_ 1

#include <vector>

namespace Core {

// enabled a view of free list chain in debug
#define USE_DEBUG_CRACKER

// NOT MT safe in general
// keeps freelist always big enough for all data
// TODO make this std-a-like interface 
template<typename Type, typename IndexType = uintptr_t>
class ExplicitFreeList
{
public:
	static const IndexType InvalidIndex = ~0;

	typedef typename std::vector<Type>::value_type value_type;
	typedef typename std::vector<Type>::size_type size_type;
	typedef typename std::vector<Type>::difference_type difference_type;
	typedef typename std::vector<Type>::pointer pointer;
	typedef typename std::vector<Type>::const_pointer const_pointer;
	typedef typename std::vector<Type>::reference reference;
	typedef typename std::vector<Type>::const_reference const_reference;

	ExplicitFreeList() : currentFree( 0 )
	{}

	explicit ExplicitFreeList( size_type _count ) : data( _count ), freelist( _count ), currentFree( _count )
	{
		for(size_type i = 0; i < _count; ++i)
		{
			freelist[i] = i;
		}
	}

	IndexType push( const value_type& _val )
	{
		IndexType index = alloc();
		data[index] = _val;
		return index;
	}

	IndexType alloc()
	{
		IndexType index;
		if(currentFree == 0)
		{
			resize((data.size() * 2) + 1 );
		}
		assert( currentFree != 0 );
		index = freelist[--currentFree];
		assert( index != InvalidIndex );
		freelist[currentFree] = InvalidIndex;

		return index;
	}

	void resize( const size_type _count )
	{
		size_type oldcount = data.size();
		assert( _count > oldcount );
		data.resize( _count );
		freelist.resize( _count );
		for(size_type i = oldcount; i < _count; ++i)
		{
			freelist[i] = InvalidIndex;
			freelist[currentFree++] = i;
		}
	}

	size_type size() const { return data.size(); }

	bool empty() const { return currentFree == freelist.size(); }

	void erase( IndexType const index_ )
	{
		assert( index_ < freelist.size());
		freelist[currentFree++] = index_;
	}

	Type& at( IndexType const index_ ) { return data.at(index_); }
	Type& operator[](IndexType const index_) { return data[index_]; }

	Type const& at( IndexType const index_ ) const { return data.at(index_); }
	Type const& operator[](IndexType const index_) const { return data[index_]; }

protected:
	std::vector<Type> data;
	std::vector<IndexType> freelist;
	IndexType currentFree;
};

// NOT MT safe in general
// keeps freelist always big enough for all data
// TODO make this std-a-like interface 
template<typename Type, typename IndexType = uintptr_t>
class IntrusiveFreeList
{
public:
	static_assert( sizeof( Type ) >= sizeof( IndexType ),
				   "IndexType must be less than size in bytes of the Type being stored" );
	static const IndexType InvalidIndex = ~0;

	typedef typename std::vector<Type>::value_type value_type;
	typedef typename std::vector<Type>::size_type size_type;
	typedef typename std::vector<Type>::difference_type difference_type;
	typedef typename std::vector<Type>::pointer pointer;
	typedef typename std::vector<Type>::const_pointer const_pointer;
	typedef typename std::vector<Type>::reference reference;
	typedef typename std::vector<Type>::const_reference const_reference;

	IntrusiveFreeList() : capacity( 0 ), current( 0 ), currentFree( 0 )
	{}

	explicit IntrusiveFreeList( size_type count_ ) : data( count_ )
	{

#if defined(USE_DEBUG_CRACKER)
		debugCracker = reinterpret_cast<DebugCracker *>( &data[0] );
#endif
		// make each entry point to the next one
		for(size_type i = 0; i < count_ - 1; ++i)
		{
			IndexType *freeptr = reinterpret_cast<IndexType *>( &data[i] );
			*freeptr = i + 1;
		}

		// tail of the list
		IndexType *freeptr = reinterpret_cast<IndexType *>( &data[count_ - 1] );
		*freeptr = InvalidIndex;

		// head of the list
		freeHead = 0;
		freeCount = count_;
	}

	IndexType push(value_type const& val_ )
	{
		IndexType index = alloc();
		data[index] = val_;
		return index;
	}

	IndexType alloc()
	{
		if(freeHead == InvalidIndex)
		{
			resize((data.size() * 2) + 1 );
		}
		assert( freeHead != InvalidIndex );

		IndexType oldHead = freeHead;
		freeHead = *reinterpret_cast<IndexType *>( &data[freeHead] );
		--freeCount;
		return oldHead;
	}

	void resize( size_type const count_ )
	{
		size_type oldcount = data.size();
		assert( count_ > oldcount );
		data.resize( count_ );
#if defined(USE_DEBUG_CRACKER)
		debugCracker = reinterpret_cast<DebugCracker *>( data.data());
#endif
		for(size_type i = oldcount; i < count_ - 1; ++i)
		{
			*reinterpret_cast<IndexType *>( &data[i] ) = i + 1;
		}
		*reinterpret_cast<IndexType *>( &data[count_ - 1] ) = freeHead;
		freeHead = oldcount;
		freeCount += (count_ - oldcount);
	}

	size_type size() const { return data.size(); }

	bool empty() const { return freeCount == size(); }

	void erase( IndexType const index_ )
	{
		*reinterpret_cast<IndexType *>( &data[index_] ) = freeHead;
		freeHead = index_;
		freeCount++;
	}

	Type& at( IndexType const index_ ) { return data.at(index_); }
	Type& operator[](IndexType const index_) { return data[index_]; }

	Type const& at( IndexType const index_ ) const { return data.at(index_); }
	Type const& operator[](IndexType const index_) const { return data[index_]; }

protected:
	std::vector<Type> data;
	IndexType freeHead;
	size_type freeCount;
#if defined(USE_DEBUG_CRACKER)
	union DebugCracker
	{
		Type data;
		IndexType index;
	} *debugCracker;
#endif
};
template<typename Type, typename IndexType = uintptr_t, class Enable = void>
class FreeList : public ExplicitFreeList<Type, IndexType>
{
public:
	FreeList() : ExplicitFreeList(){};
	explicit FreeList(size_type count_) : ExplicitFreeList(count_){}
};

template<typename Type, typename IndexType>
class FreeList<Type, IndexType,
		typename std::enable_if_t<
			std::is_pod<Type>{} && (sizeof(Type) >= sizeof(IndexType))
		>::value> : public IntrusiveFreeList<Type, IndexType>
{
public:
	FreeList() : IntrusiveFreeList(){};
	explicit FreeList(IntrusiveFreeList<Type,IndexType>::size_type count_) : IntrusiveFreeList(count_){}
};

} // end Core namespace

#endif