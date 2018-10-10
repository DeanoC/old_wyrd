/// \file	core\core_utils.h
/// \brief	Declares the core utils class. 
/// \details	
///		just some general helper macros and functions

#pragma once
#ifndef CORE_UTILS_H_
#define CORE_UTILS_H_

#include <type_traits>

/// \brief	Returns the number of elements in a staic array. 
/// \param	array	The array must be static.
/// \todo	Google has a version that detects it is a static array, this just bugs out if not... 
#define NUM_ARRAY_ELEMENTS( array ) ( sizeof( (array) ) / sizeof( (array)[0] ) )

/// \brief	marks the variable unused. 
/// \param	i	The variable to mark as unused. 
#define UNUSED( i ) (void)i

/// \def	MACRO_TEXT(x) #x
/// \brief	Converts parameter in text. 
/// \param	x	The parameter. 
#define MACRO_TEXT(x) #x

/// \brief	returns the text of the value of the parameter. 
/// \param	x	The parameter. 
#define MACRO_VALUE(x) MACRO_TEXT(x)

namespace Core {

/// \brief	return the bit of the number.
/// \param	num	returns the bit representing this index.
template<typename T>
constexpr auto Bit( T num ) -> T
{
	static_assert(std::is_integral<T>::value, "Integral required.");
	return (T(1) << (num));
}

/// \brief	Query if 'iNum' is power of two. 
/// \param	iNum	Number to test. 
/// \return	true if pow 2, false if not. 
CALL inline bool isPow2( unsigned int iNum ) 
{
	return ((iNum & (iNum - 1)) == 0);
}


/// \brief	Gets the next pow 2. 
/// \param	iNum	Number to get next pow 2 off. 
/// \return	The next pow 2 of iNum. 
CALL inline unsigned int nextPow2( unsigned int iNum ) 
{
	iNum -= 1;
	iNum |= iNum >> 16;
	iNum |= iNum >> 8;
	iNum |= iNum >> 4;
	iNum |= iNum >> 2;
	iNum |= iNum >> 1;

	return iNum + 1;
}


/// \brief	Align input to align val. 
/// \param	k		The value to align. 
/// \param	align	The alignment boundary. 
/// \return	k aligned to align. 
template< typename T, typename T2 >
CALL inline T alignTo(T k, T2 align)
{
	return ((k+align-1) & ~(align-1));
}

namespace detail
{

template <typename T, std::size_t...Is>
std::array<T, sizeof...(Is)> make_array(const T& value, std::index_sequence<Is...>)
{
	return {{(static_cast<void>(Is), value)...}};
}
}

template <std::size_t N, typename T>
std::array<T, N> make_array(const T& value)
{
	return detail::make_array(value, std::make_index_sequence<N>());
}

}	//namespace Core


#endif
