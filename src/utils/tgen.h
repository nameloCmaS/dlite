/* tgen.h -- simple templated text generator */
#ifndef _TGEN_H
#define _TGEN_H
/**
  @file
  @brief Simple templated text generator

  The main function in this library is tgen(). It takes a template and
  a list of substitutions and produces a new document.

  An example template may look like:

      Group: {group_name}
      Location: {group_location}
      Members:
      {list_members:  - {first_name} {last_name} lives in {country}\n}

  The example program @ref tgen_example.c will produce the following
  output from this template:

      Group: skiers
      Location: mountains
      Members:
        - Adam Smidth lives in Germany
        - Jack Daniel lives in USA
        - Fritjof Nansen lives in Norway

  A pair of braces, "{" and "}", that encloses a string is a *tag*.
  When the template is processed, the tags are replaced with new
  content according to the substitutions.  The general form for a tag
  is:

      {VAR%FMT:TEMPL}

  where the parts "%FMT" and ":TEMPL" are optional:

    - `VAR` identifies the tag in the substitutions.
    - `FMT` is an optional printf() format specifier for printing a
       string.  This is a security risk (since the current
       implementation passes it unverified to snprintf()), this
       feature can be disabled by setting the global variable
       `tgen_allow_formatting` to zero.
    - `TEMPL` is an optional template that may be used in
       nested calls.  It may contain embedded tags, as long
       as the opening and closing braces exactly match.

  Literal braces may be included in the template and the `TEMPL`
  section, if they are escaped according the following table:

  escape sequence | result | comment
  --------------- | ------ | -------
  `{{`            | `{`    | literal start brace
  `}}`            | `}`    | literal end brace
  `{}`            | `}`    | only use this if `TEMPL` ends with a `}`

  In addition can the normal C escape sequences (`\a`, `\b`, `\f`,
  `\n`, `\r`, `\t`, `\v` and `\\`) be used (if the global variable
  `tgen_convert_escape_sequences` is non-zero).

  There are two types of substitutions, variable substitutions
  and function substitutions:

    - A **variable substitution** relates `VAR` to a string replacing
      the tag.  If the tag contains a `TEMPL`-part, it will be
      ignored.

    - A **function substitution** relates `VAR` to a function.  When
      the template is processed, the function is called replacing the
      tag with its output.  The function uses `TEMPL` as a
      (sub)template.

  The strength of templating is that you can produce the same information
  in a completely different format just by changing the template, without
  changing the logics.
*/


/**
  ### Templated text generator example

  @example tgen_example.c
 */

#include <stdlib.h>
#include <stdio.h>

#include "map.h"


/**
   Error codes used by this library
*/
enum {
  TGenOk,
  TGenAllocationError,
  TGenSyntaxError,
  TGenIOError,
  TGenVariableError,
  TGenSubtemplateError,
  TGenMapError,
  TGenFormatError,
};


/**
  Buffer for generated output.
*/
typedef struct _TGenBuf {
  char *buf;    /*!< buffer */
  size_t size;  /*!< allocated size of buffer */
  size_t pos;   /*!< current position */
} TGenBuf;


/**
  A structure managing a list of substitutions
*/
typedef struct _TGenSubs {
  struct _TGenSub *subs;  /*!< list of substitutions */
  int size;               /*!< allocated size of subs */
  int nsubs;              /*!< length of subs */
  map_int_t map;          /*!< maps variable name to index in subs */
} TGenSubs;


/**
  Prototype for generator function that appends to the output buffer.

  - `s`: output buffer
  - `template`: input template
  - `len`: length of `template`.  A negative number indicates that
     the template is NUL-terminated.
  - `subs`: substitutions
  - `context`: pointer to user-defined context passed on to generator
    functions
*/
typedef int (*TGenFun)(TGenBuf *s, const char *template, int len,
                       const TGenSubs *subs, void *context);



/**
  Struct defining a substitution.
*/
typedef struct _TGenSub {
  char *var;      /*!< Variable that should be substituted */
  char *repl;     /*!< String that the variable should be replaced with.
                       May also be used as subtemplate if `sub` is given
                       and the main template does not provide a subtemplate
                       for this substitution. */
  TGenFun func;   /*!< Generator function, may be NULL */
} TGenSub;


/** Whether to convert standard escape sequences. */
extern int tgen_convert_escape_sequences;

/** Whether to allow interpreating FMT-part of tags (may be a security
    risk if the template come from an untrusted source). */
extern int tgen_allow_formatting;

/**
  @name Utility functions
  @{
 */

/**
  Appends `n` bytes from string `src` to end of output buffer `s`.

  If `n` is negative, all of `str` (which must be NUL-terminated) is
  appended.

  Returns non-zero on error.
 */
int tgen_buf_append(TGenBuf *s, const char *src, int n);

/**
  Like tgen_buf_append() but allows frintf() formatting of the input.
 */
int tgen_buf_append_fmt(TGenBuf *s, const char *fmt, ...);

/**
  Like tgen_buf_append_fmt(), but takes a `va_list` instead of a
  variable number of arguments.
 */
int tgen_buf_append_vfmt(TGenBuf *s, const char *fmt, va_list ap);

/**
  Returns the line number of position `t` in `template`.
*/
int tgen_lineno(const char *template, const char *t);

/**
  Reads a file and returns a newly allocated buffer with its content or
  NULL on error.
 */
char *tgen_readfile(const char *filename);

/** @} */


/**
  @name Functions for managing substitutions
  @{
 */

/**
   Initiates memory used by `subs`.
*/
void tgen_subs_init(TGenSubs *subs);

/**
   Deinitiates memory used by `subs`.
*/
void tgen_subs_deinit(TGenSubs *subs);

/**
  Returns substitution corresponding to `var` or NULL if there are no
  matching substitution.
*/
const TGenSub *tgen_subs_get(const TGenSubs *subs, const char *var);

/**
  Like tgen_subs_get(), but allows `var` to not be NUL-terminated by
  specifying its length with `len`. If `len` is negative, this is equivalent
  to calling tgen_subs_get().
*/
const TGenSub *tgen_subs_getn(const TGenSubs *subs, const char *var, int len);

/**
  Adds variable `var` to list of substitutions `subs`.  `repl` and
  `func` are the corresponding replacement string and generator
  function, respectively.

  Returns non-zero on error.
*/
int tgen_subs_set(TGenSubs *subs, const char *var, const char *repl,
                  TGenFun func);

/**
  Like tgen_subs_add(), but allows `var` to not be NUL-terminated by
  specifying its length with `len`.  If `len` is negative, this is
  equivalent to calling tgen_subs_get().

  Returns non-zero on error.
*/
int tgen_subs_setn(TGenSubs *subs, const char *var, int len,
                   const char *repl, TGenFun func);

/**
  Like tgen_subs_set(), but allows printf() formatting of the
  replacement string.

  Returns non-zero on error.
*/
int tgen_subs_set_fmt(TGenSubs *subs, const char *var, TGenFun func,
                      const char *repl_fmt, ...);

/**
  Like tgen_subs_setn(), but allows printf() formatting of the
  replacement string.

  Returns non-zero on error.
*/
int tgen_subs_setn_fmt(TGenSubs *subs, const char *var, int len,
                       TGenFun func, const char *repl_fmt, ...);

/**
  Like tgen_subs_setn(), but allows printf() formatting of the
  replacement string.

  Returns non-zero on error.
*/
int tgen_subs_setn_vfmt(TGenSubs *subs, const char *var, int len,
                        TGenFun func, const char *repl_fmt, va_list ap);

/**
  Initiates `dest` and copies substitutions from `src` to it.  `dest`
  should not be initiated in beforehand.

  Returns non-zero on error.  In this case, `dest` will be left in a
  non-initialised state.
 */
int tgen_subs_copy(TGenSubs *dest, const TGenSubs *src);

/** @} */


/**
  @name Functions for text generations
  @{
 */

/**
  Returns a newly malloc'ed string based on `template`, where all
  occurences of ``{VAR}`` are replaced according to substitution `VAR`
  in the array `substitutions`, which has length `n`.

  The template may also refer to a substitution as ``{VAR:TEMPL}``.
  If the substitution corresponding to `VAR` provide a substitution
  function (via its `subs` member), `TEMPL` will be passed as
  subtemplate to the substitution function.  If `TEMPL` is not given,
  then the subtemplate will be taken from the `repl` member of the
  corresponding substitution.

  `context` is a pointer to user data passed on to the substitution
  function.

  Returns NULL, on error.
 */
char *tgen(const char *template, const TGenSubs *subs, void *context);


/**
  Like tgen(), but appends to `s` instead of returning the substituted
  template.  `len` is the length of `template`.

  Returns non-zero on error.
 */
int tgen_append(TGenBuf *s, const char *template, int len,
                const TGenSubs *subs, void *context);

/** @} */


#endif /* _TGEN_H */
