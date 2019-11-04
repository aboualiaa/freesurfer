/* getline.h --- Prototype for replacement getline function.
   Copyright (C) 2005 Free Software Foundation, Inc.
*/

/* Written by Simon Josefsson. */

/* Get size_t, FILE, ssize_t.  And getline, if available.  */
#include <cstddef>
#include <cstdio>
#include <sys/types.h>

#if !HAVE_DECL_GETLINE
ssize_t getline(char **lineptr, size_t *n, FILE *stream);
#endif /* !HAVE_GETLINE */
