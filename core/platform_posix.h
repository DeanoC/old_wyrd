#	include <stdint.h>
#	include <stdlib.h>
#	include <malloc.h>

#define CALL
#define READONLY_MEM
#define LOCAL_MEM

#define ALIGN(x)  __align__(x)
#define INLINE inline