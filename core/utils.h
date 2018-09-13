/// \file	core\core_utils.h
/// \brief	Declares the core utils class. 
/// \details	
///		just some general helper macros and functions

#pragma once

#ifndef CORE_UTILS_H_
#define CORE_UTILS_H_

/// \brief	Returns the number of elements in a staic array. 
/// \param	array	The array must be static.
/// \todo	Google has a version that detects it is a static array, this just bugs out if not... 
#define NUM_ARRAY_ELEMENTS( array ) ( sizeof( (array) ) / sizeof( (array)[0] ) )

/// \brief	return the bit of the number. 
/// \param	num	returns the bit representing this index. 
#define BIT( num ) (1ul << (num))

/// \brief	return the bit of the number. 
/// \param	num	returns the bit representing this index. 
#define BIT64( num ) (1ull << (num))

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

/// \fn	CORE_INLINE int floor2Int( float value )
/// \brief	float to ine. 
/// \param	value	The float value to convert. 
/// \return	the integer value of the float (truncate AKA floor). 
CALL inline int floor2Int( float value )
{
	int output;
	// TODO x86 asm verison probably cand be removed
#if CPU_FAMILY == CPU_X86 && COMPILER == MS_COMPILER
	_asm
	{
		fld value
		fistp output
	}
#else
	output = static_cast<int>( floorf(value) );
#endif

	return output;
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

#if PLATFORM_OS != MS_WINDOWS
static uint8_t s_LogTable256[] = 
{
	0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};
#endif


/// \brief	return Log2 of v. 
/// return log2 of an int. this is equivalent to finding the highest bit that has been set
/// or the number to shift 1 left by to get the nearest lower power of 2
/// \param	v	The number to get the log2 of. 
/// \return	log2(v). 
CALL inline unsigned int log2(unsigned int v)
{
	// TODO use an intrinsic so hw version portable to mac and linux at least
#if PLATFORM_OS == MS_WINDOWS
	unsigned long log2 = 0;

	_BitScanReverse( &log2, v );

	return unsigned int( log2 );
#else
	unsigned int r;     // r will be lg(v)
	unsigned int t, tt; // temporaries

	if ((tt = v >> 16) != 0)
	{
		r = ((t = tt >> 8)!=0) ? 24 + ((unsigned int)s_LogTable256[t]) : 16 + ((unsigned int)s_LogTable256[tt]);
	}
	else 
	{
		r = ((t = v >> 8)!=0) ? 8 + ((unsigned int)s_LogTable256[t]) : ((unsigned int)s_LogTable256[v]);
	}
	return r;
#endif
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

}	//namespace Core


#endif
