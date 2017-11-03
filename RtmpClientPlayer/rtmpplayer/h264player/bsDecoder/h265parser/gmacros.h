/* GLIB - Library of useful routines for C programming
 *
 * Changed by He Caiguang; 2015-01-24
 */


#ifndef __G_MACROS_H__
#define __G_MACROS_H__


/* We include stddef.h to get the system's definition of NULL
 */
#include <stddef.h>

#define g_return_val_if_fail(expr,val) { if (!(expr)) return (val); }
#define g_return_if_fail(expr) { if (!(expr)) return; }

#define _G_NEW(struct_type, n_structs, func) \
    ((struct_type *) func ((n_structs), sizeof (struct_type)))
//#define g_renew(struct_type, mem, n_structs)		_G_RENEW (struct_type, mem, n_structs, realloc)
#define g_new0(struct_type, n_structs)			_G_NEW (struct_type, n_structs, calloc)
#define g_new(struct_type, n_structs)			_G_NEW (struct_type, n_structs, calloc)

/* Guard C code in headers, while including them from C++ */
#ifdef  __cplusplus
#define G_BEGIN_DECLS  extern "C" {
#define G_END_DECLS    }
#else
#define G_BEGIN_DECLS
#define G_END_DECLS
#endif

/* Provide definitions for some commonly used macros.
 *  Some of them are only provided if they haven't already
 *  been defined. It is assumed that if they are already
 *  defined then the current definition is correct.
 */

#ifndef NULL
#  ifdef __cplusplus
#  define NULL        (0L)
#  else /* !__cplusplus */
#  define NULL        ((void*) 0)
#  endif /* !__cplusplus */
#endif

#ifndef	FALSE
#define	FALSE	(0)
#endif

#ifndef	TRUE
#define	TRUE	(!FALSE)
#endif

#undef	MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

#undef	MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

#undef	ABS
#define ABS(a)	   (((a) < 0) ? -(a) : (a))

#undef	CLAMP
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

/* Count the number of elements in an array. The array must be defined
 * as such; using this with a dynamically allocated array will give
 * incorrect results.
 */
#define G_N_ELEMENTS(arr)		(sizeof (arr) / sizeof ((arr)[0]))

/* Macros by analogy to GINT_TO_POINTER, GPOINTER_TO_INT
 */
#define GPOINTER_TO_SIZE(p)	((gsize) (p))
#define GSIZE_TO_POINTER(s)	((gpointer) (gsize) (s))



#endif /* __G_MACROS_H__ */
