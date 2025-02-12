/* setenv.h -- environment related cross-platform compatibility functions
 *
 * Copyright (C) 2017 SINTEF
 *
 * 
 * Distributed under terms of the MIT license.
 */

#include "setenv.h"


/* setenv() - change or add an environment variable */
#ifndef HAVE_SETENV
# ifdef HAVE__PUTENV_S
int setenv(const char *name, const char *value, int overwrite)
{
  if (overwrite || getenv(name))
    return _putenv(name, value);
  return 0;
}
# endif
#endif
