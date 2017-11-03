/* GLIB - Library of useful routines for C programming
 *
 * Changed by He Caiguang; 2015-01-24
 */


#ifndef __G_TYPES_H__
#define __G_TYPES_H__

#include "gmacros.h"

G_BEGIN_DECLS

/* Provide type definitions for commonly used types.
 *  These are useful because a "gint8" can be adjusted
 *  to be 1 byte (8 bits) on all platforms. Similarly and
 *  more importantly, "gint32" can be adjusted to be
 *  4 bytes (32 bits) on all platforms.
 */

typedef signed char gint8;
typedef unsigned char guint8;
typedef signed short gint16;
typedef unsigned short guint16;
typedef signed int gint32;
typedef unsigned int guint32;
typedef signed long long gint64;
typedef unsigned long long guint64;

typedef char   gchar;
typedef short  gshort;
typedef long   glong;
typedef int    gint;
typedef gint   gboolean;

typedef unsigned char   guchar;
typedef unsigned short  gushort;
typedef unsigned long   gulong;
typedef unsigned int    guint;

typedef float   gfloat;
typedef double  gdouble;
typedef unsigned long long gsize;

typedef struct GArray {
    gchar *data;
    guint len;
} GArray;

/***** default padding of structures *****/
#define GST_PADDING		4
#define GST_PADDING_INIT	{ NULL }

/***** padding for very extensible base classes *****/
#define GST_PADDING_LARGE	20


/* Define min and max constants for the fixed size numerical types */
#define G_MININT8	((gint8)  0x80)
#define G_MAXINT8	((gint8)  0x7f)
#define G_MAXUINT8	((guint8) 0xff)

#define G_MININT16	((gint16)  0x8000)
#define G_MAXINT16	((gint16)  0x7fff)
#define G_MAXUINT16	((guint16) 0xffff)

#define G_MININT32	((gint32)  0x80000000)
#define G_MAXINT32	((gint32)  0x7fffffff)
#define G_MAXUINT32	((guint32) 0xffffffff)

#define G_MININT64	((gint64) G_GINT64_CONSTANT(0x8000000000000000))
#define G_MAXINT64	G_GINT64_CONSTANT(0x7fffffffffffffff)
#define G_MAXUINT64	G_GINT64_CONSTANT(0xffffffffffffffffU)

typedef void* gpointer;
typedef const void *gconstpointer;


G_END_DECLS


#endif /* __G_TYPES_H__ */
