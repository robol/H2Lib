#ifndef __LAPACK_TYPES_H
#define __LAPACK_TYPES_H

#ifndef LAPACK_USE_LONGS
#define LAPACK_INT int
#else
#include <stddef.h>
#define LAPACK_INT ptrdiff_t
#endif

#endif
