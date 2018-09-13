#	include <cstdint>
#	include <cstdlib>
#	include <memory.h>

#define CALL
#define READONLY_MEM
#define LOCAL_MEM

#define ALIGN(x)  __align__(x)
#define INLINE inline