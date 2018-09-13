#ifndef _DLITE_TYPES_H
#define _DLITE_TYPES_H

/**
  @file
  @brief Data types for instance properties

  The types of properties is in DLite described by its `dtype` (a type
  number from enum DLiteType) and `size` (size in bytes of a single
  data point).

  Note that the type (`dtype` and `size`) concerns a single data
  point.  The dimensionality (`ndims` and `dims`) of a property comes
  in addition and is not treated by the functions in this file.

  The properties can have most of the basic types found in C, with
  some additions, as summarised in the table below:

  type      | dtype          | sizes          | pointer                       | description                      | examples names
  ----      | -----          | -----          | -------                       | -----------                      | --------------
  blob      | dliteBlob      | any            | uint8_t *                     | binary blob, sequence of bytes   | blob32, blob128
  bool      | dliteBool      | sizeof(bool)   | bool *                        | boolean                          | bool
  int       | dliteInt       | 1, 2, 4, {8}   | int8_t *, int16_t *, ...      | signed integer                   | (int), int8, int16, int32, {int64}
  uint      | dliteUInt      | 1, 2, 4, {8}   | uint8_t *, uint16_t *, ...    | unsigned integer                 | (uint), uint8, uint16, uint32, {uint64}
  float     | dliteFloat     | 4, 8, {10, 16} | float32_t *, float64_t *, ... | floating point                   | (float), (double), float32, float64, {float80, float128}
  fixstring | dliteFixString | any            | char *                        | fix-sized NUL-terminated string  | string20
  string    | dliteStringPtr | sizeof(char *) | char **                       | pointer to NUL-terminated string | string

  The column "pointer" shows the C type of the `ptr` argument for
  functions like dlite_instance_get_property() and
  dlite_instance_set_property().  Note that this pointer type is the
  same regardless we are referring to a scalar or an array.  For arrays
  the pointer points to the first element.

  The column "examples names" shows examples of how these types whould
  be written when specifying the type of a property.
    - The type names in parenthesis are included for portability with
      SOFT5, but not encouraged because their may vary between platforms.
    - The type names in curly brackets may not be defined on all
      platforms.  The headers "integers.h" and "floats.h" provides
      macros like `HAVE_INT64_T`, `HAVE_FLOAT128_T`... that can be
      used to check for availability.

  Some additional notes:
    - *blob*: is a sequence of bytes of length `size`.  When writing
      the type name, you should always append the size as shown in
      the examples in table.
    - *bool*: corresponds to the bool type as defined in <stdbool.h>.
      To support systems lacking <stdbool.h> you can use "boolean.h"
      provided by dlite.
    - *fixstring*: corresponds to `char fixstring[size]` in C. The
      size includes the terminating NUL.
    - *string*: corresponds to `char *string` in C, pointing to memory
      allocated with malloc().  If you free a string, you should always
      set the pointer to NULL, since functions like dlite_entity_free()
      otherwise will try to free it again, causing memory corruption.
*/

#include <stdlib.h>

#include "boolean.h"
#include "triplestore.h"

/* Expands to the struct alignment of type */
#define alignof(type) ((size_t)&((struct { char c; type d; } *)0)->d)

/* Expands to the amount of padding that should be added before `type`
   if `type` is to be added to a struct at offset `offset`. */
#define padding_at(type, offset)                                        \
  ((alignof(type) - ((offset) & (alignof(type) - 1))) & (alignof(type) - 1))


typedef struct _DLiteProperty  DLiteProperty;
typedef struct _DLiteDimension DLiteDimension;
typedef struct _Triplet        DLiteRelation;



/** Basic data types */
typedef enum _DLiteType {
  dliteBlob,             /*!< Binary blob, sequence of bytes */
  dliteBool,             /*!< Boolean */
  dliteInt,              /*!< Signed integer */
  dliteUInt,             /*!< Unigned integer */
  dliteFloat,            /*!< Floating point */
  dliteFixString,        /*!< Fix-sized NUL-terminated string */
  dliteStringPtr,        /*!< Pointer to NUL-terminated string */

  dliteDimension,        /*!< Dimension, for entities */
  dliteProperty,         /*!< Property, for entities */
  dliteRelation,         /*!< Subject-predicate-object relation */
} DLiteType;


/**
  Returns descriptive name for \a dtype or NULL on error.
*/
const char *dlite_type_get_dtypename(DLiteType dtype);

/**
  Returns the dtype corresponding to \a dtypename or -1 on error.
*/
DLiteType dlite_type_get_dtype(const char *dtypename);

/**
  Writes the type name corresponding to \a dtype and \a size to \a typename,
  which must be of size \a n.  Returns non-zero on error.
*/
int dlite_type_set_typename(DLiteType dtype, size_t size,
                            char *typename, size_t n);

/**
  Returns true if name is a DLiteType, otherwise false.
 */
bool dlite_is_type(const char *name);

/**
  Assigns \a dtype and \a size from \a typename.  Returns non-zero on error.
*/
int dlite_type_set_dtype_and_size(const char *typename,
                                  DLiteType *dtype, size_t *size);


/**
  Returns non-zero id `dtype` contains allocated data, like dliteStringPtr.
 */
int dlite_type_is_allocated(DLiteType dtype);

/**
  Copies value of given dtype from `src` to `dest`.  If the dtype contains
  allocated data, new memory will be allocated for `dest`.

  Returns a pointer to the memory area `dest` or NULL on error.
*/
void *dlite_type_copy(void *dest, const void *src,
                      DLiteType dtype, size_t size);

/**
  Clears the memory pointed to by `p`.  Its type is gived by `dtype`
  and `size`.

  Returns a pointer to the memory area `p` or NULL on error.
*/
void *dlite_type_clear(void *p, DLiteType dtype, size_t size);


/**
  Returns the struct alignment of the given type or 0 on error.
 */
size_t dlite_type_get_alignment(DLiteType dtype, size_t size);

/**
  Returns the amount of padding that should be added before `type`,
  if `type` (of size `size`) is to be added to a struct at offset `offset`.
*/
size_t dlite_type_padding_at(DLiteType dtype, size_t size, size_t offset);


/**
  Returns the offset the current struct member with dtype \a dtype and
  size \a size.  The offset of the previous struct member is \a prev_offset
  and its size is \a prev_size.

  Returns -1 on error.
 */
int dlite_type_get_member_offset(size_t prev_offset, size_t prev_size,
                                 DLiteType dtype, size_t size);

#endif /* _DLITE_TYPES_H */
