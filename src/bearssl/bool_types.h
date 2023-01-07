#ifndef BOOL_TYPES_H__
#define BOOL_TYPES_H__

#if !defined(FALSE)
#define FALSE (0)
#endif
#if !defined(TRUE)
#define TRUE (!FALSE)
#endif
#if _MSC_VER >= 1800
#include <stdbool.h>
#else
#if !defined(bool)
#define bool int
#endif
#if !defined(true)
#define true TRUE
#endif
#if !defined(false)
#define false FALSE
#endif
#endif

#endif