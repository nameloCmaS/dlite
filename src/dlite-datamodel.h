/* dlite-datamodel.h -- access to an instance of an entity in a storage */
#ifndef _DLITE_DATAMODEL_H
#define _DLITE_DATAMODEL_H

/**
  @file
  @brief Direct access to data model for an entity instance in a storage.

  The dlite datamodel provides low level access to the data stored in
  an entity.  You do normally not call the functions defined in this
  header file if you access data via the interface defined in
  dlite-entity.h or generated by the dlite code generator.
*/


/**
  Opaque type for a DLiteDataModel.

  A data model refers to a specific instance of an entity in a storage.
 */
typedef struct _DLiteDataModel DLiteDataModel;

#include "dlite-type.h"
#include "dlite-storage.h"
#include "dlite-plugins.h"


/**
  @name Minimum API

  Minimum API that all plugins should implement.
  @{
 */


/**
  Returns a new data model for instance \a id in storage \a s or NULL on error.
  Should be free'ed with dlite_datamodel_free().
 */
DLiteDataModel *dlite_datamodel(const DLiteStorage *s, const char *id);

/**
  Frees up all resources allocated by dlite_datamodel().
 */
int dlite_datamodel_free(DLiteDataModel *d);


/**
  Returns newly malloc()'ed string with the metadata uri or NULL on error.
 */
char *dlite_datamodel_get_meta_uri(const DLiteDataModel *d);

/**
  Returns the size of dimension \a name or 0 on error.
 */
size_t dlite_datamodel_get_dimension_size(const DLiteDataModel *d,
                                          const char *name);

/**
  Copies property \a name to memory pointed to by \a ptr.
  Multi-dimensional arrays are supported.

  \param  d      DLiteDataModel data handle.
  \param  name   Name of the property.
  \param  ptr    Pointer to memory to write to.
  \param  type   Type of data elements.
  \param  size   Size of each data element.
  \param  ndims  Number of dimensions.
  \param  dims   Array of dimension sizes of length \p ndims.

  Returns non-zero on error.

  @note
  The memory pointed to by \a ptr must be at least of size

      size * dims[0] * ... * dims[ndim-1]

  In contrast to the other data types, getting data of DTStringPtr
  type only writes (char *) pointers to the actual strings in memory
  pointed to \a ptr.  The strings themself are allocated on the heap
  (using malloc()).  Hence, the user is responsible to free this
  memory herselves.
 */
int dlite_datamodel_get_property(const DLiteDataModel *d, const char *name,
                                 void *ptr, DLiteType type, size_t size,
                                 size_t ndims, const size_t *dims);

/** @} */


/**
  @name Optional API

  Optional API that plugins are free leave unimplemented.

  The plugin must provide dlite_datamodel_set_property(),
  dlite_datamodel_set_meta_uri() and dlite_datamodel_set_dimension_size()
  to support writing, otherwise only read is supported.

  All functions below are supported by the HDF5 plugin.
  @{
*/


/**
  Sets property \a name to the memory (of \a size bytes) pointed to by
  \a value.  The argument \a string_pointers has the same meaning as
  for dlite_datamodel_get_property().

  Returns non-zero on error.
*/
int dlite_datamodel_set_property(DLiteDataModel *d, const char *name,
                                 const void *ptr, DLiteType type, size_t size,
                                 size_t ndims, const size_t *dims);


/**
  Sets metadata uri.  Returns non-zero on error.
 */
int dlite_datamodel_set_meta_uri(DLiteDataModel *d, const char *uri);

/**
  Sets size of dimension \a name.  Returns non-zero on error.
*/
int dlite_datamodel_set_dimension_size(DLiteDataModel *d, const char *name,
                                       size_t size);


/**
   Returns a positive value if dimension \a name is defined, zero if
   it isn't and a negative value on error (e.g. if this function isn't
   supported by the plugin).
 */
int dlite_datamodel_has_dimension(DLiteDataModel *d, const char *name);

/**
   Returns a positive value if property \a name is defined, zero if it
   isn't and a negative value on error (e.g. if this function isn't
   supported by the plugin).
 */
int dlite_datamodel_has_property(DLiteDataModel *d, const char *name);

/**
   If the uuid was generated from a unique name, return a pointer to a
   newly malloc'ed string with this name.  Otherwise NULL is returned.
*/
char *dlite_datamodel_get_dataname(DLiteDataModel *d);

/** @} */





/**
 * Utility functions intended to be used by the plugins
   @{
 */

/** Copies data from nested pointer to pointers array \a src to the
    flat continuous C-ordered array \a dst. The size of dest must be
    sufficient large.  Returns non-zero on error. */
int dlite_copy_to_flat(void *dst, const void *src, size_t size,
                       size_t ndims, const size_t *dims);

/** Copies data from flat continuous C-ordered array \a dst to nested
    pointer to pointers array \a src. The size of dest must be
    sufficient large.  Returns non-zero on error. */
int dlite_copy_to_nested(void *dst, const void *src, size_t size,
                         size_t ndims, const size_t *dims);

/** @} */


#endif /* _DLITE_DATAMODEL_H */
