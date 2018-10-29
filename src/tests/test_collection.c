#include "config.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "minunit/minunit.h"
#include "dlite.h"
#include "dlite-collection.h"

#define STRINGIFY(s) _STRINGIFY(s)
#define _STRINGIFY(s) # s


//DLiteEntity *entity = NULL;
//DLiteInstance *inst = NULL;
DLiteCollection *coll = NULL;


/***************************************************************
 * Test collection
 ***************************************************************/

MU_TEST(test_collection_create)
{
  mu_check((coll = dlite_collection_create("mycoll")));
}


MU_TEST(test_collection_add)
{
  char *path, *uri;
  DLiteStorage *s;
  DLiteInstance *e, *inst;

  path = STRINGIFY(DLITE_ROOT) "/src/tests/test-entity.json";
  //uri = "http://meta.sintef.no/0.1/test_entity";
  mu_check((s = dlite_storage_open("json", path, "mode=r")));
  mu_check((e = dlite_instance_load(s, NULL, NULL)));
  mu_check(!dlite_storage_close(s));
  //dlite_metastore_add((DLiteMeta *)e);


  path = STRINGIFY(DLITE_ROOT) "/src/tests/test-data.json";
  //uri = "my_test_instance";
  uri = "e076a856-e36e-5335-967e-2f2fd153c17d";
  mu_check((s = dlite_storage_open("json", path, "mode=r")));
  mu_check((inst = dlite_instance_load(s, uri, e)));
  mu_check(!dlite_storage_close(s));

  mu_check(!dlite_collection_add_new(coll, "e", e));
  mu_check(!dlite_collection_add_new(coll, "inst", inst));
}


MU_TEST(test_collection_free)
{
  dlite_collection_decref(coll);
}


//MU_TEST(test_collection_load)
//{
//  DLiteStorage *s;
//  DLiteInstance *e;  /* the entity cast to a DLiteInstance */
//  char *path = STRINGIFY(DLITE_ROOT) "/tools/tests/Chemistry-0.1.json";
//  char *uri = "http://www.sintef.no/calm/0.1/Chemistry";
//
//  mu_check((s = dlite_storage_open("json", path, "mode=r")));
//  mu_check((entity = dlite_entity_load(s, uri)));
//  mu_check(!dlite_storage_close(s));
//
//  e = (DLiteInstance *)entity;
//
//  mu_assert_int_eq(2, dlite_instance_get_dimension_size(e, "ndimensions"));
//  mu_assert_int_eq(8, dlite_instance_get_dimension_size(e, "nproperties"));
//}
//
//
///* */
//MU_TEST(test_entity_property)
//{
//  const DLiteProperty *prop;
//  const char *descr;
//
//  descr = "Chemical symbol of each chemical element.  By convension the "
//    "dependent element (e.g. Al) is listed first.";
//  mu_check((prop = dlite_entity_get_property(entity, "elements")));
//  mu_assert_string_eq("elements", prop->name);
//  mu_assert_int_eq(dliteStringPtr, prop->type);
//  mu_assert_int_eq(sizeof(char *), prop->size);
//  mu_assert_int_eq(1, prop->ndims);
//  mu_assert_string_eq(descr, prop->description);
//}
//
//
//MU_TEST(test_instance_create)
//{
//  size_t dims[] = {3, 2};
//
//  char *alloy = "6063";
//  char *elements[] = {"Al", "Mg", "Si"};
//  char *phases[] = {"beta\"", "beta'"};
//  double X0[] = {0.99, 0.005, 0.005};
//  double Xp[2][3] = {
//    {2./11., 5./11., 4./11.},
//    {0.,     9./14., 5./14.}
//  };
//  double volfrac[] = {0.005, 0.001};
//  double rpart[] = {7e-9, 15e-9};
//  double atvol[] = {1.9e-29, 1.8e-29};
//
//  DLiteStorage *s;
//
//  inst = dlite_instance_create(entity, dims, "myinst");
//
//  mu_check(!dlite_instance_set_property(inst, "alloy", &alloy));
//  mu_check(!dlite_instance_set_property(inst, "elements", elements));
//  mu_check(!dlite_instance_set_property(inst, "phases", phases));
//  mu_check(!dlite_instance_set_property(inst, "X0", X0));
//  mu_check(!dlite_instance_set_property(inst, "Xp", Xp));
//  mu_check(!dlite_instance_set_property(inst, "volfrac", volfrac));
//  mu_check(!dlite_instance_set_property(inst, "rpart", rpart));
//  mu_check(!dlite_instance_set_property(inst, "atvol", atvol));
//
//  mu_check((s = dlite_storage_open("json", "alloys.json", "mode=w")));
//  mu_check(!dlite_instance_save(s, inst));
//  mu_check(!dlite_storage_close(s));
//}
//
//
//MU_TEST(test_instance_load)
//{
//  char *path = STRINGIFY(DLITE_ROOT) "/src/tests/alloys.json";
//  int i;
//  DLiteStorage *s;
//  DLiteInstance *inst2, *e=(DLiteInstance *)entity;
//  int ndims = dlite_instance_get_dimension_size(e, "ndimensions");
//  int nprops = dlite_instance_get_dimension_size(e, "nproperties");
//  int *dims = calloc(ndims, sizeof(int)) ;
//  char *id = "8411a72c-c7a3-5a6a-b126-1e90b8a55ae2";
//  //char *id = "http://www.sintef.no/calm/0.1/Chemistry";
//
//  mu_check((s = dlite_storage_open("json", path, "mode=r")));
//  mu_check((inst2 = dlite_instance_load(s, id, entity)));
//  mu_check(!dlite_storage_close(s));
//
//  for (i=0; i<ndims; i++) {
//    int len = dlite_instance_get_dimension_size_by_index(inst, i);
//    dims[i] = dlite_instance_get_dimension_size_by_index(inst2, i);
//    mu_assert_int_eq(len, dims[i]);
//  }
//
//  for (i=0; i<nprops; i++) {
//    const DLiteProperty *p = dlite_entity_get_property_by_index(entity, i);
//    if (p->ndims) {
//    } else {
//    }
//  }
//
//  mu_check((s = dlite_storage_open("json", "alloys2.json", "mode=w")));
//  mu_check(!dlite_instance_save(s, inst2));
//  mu_check(!dlite_storage_close(s));
//
//  dlite_instance_decref(inst2);
//  free(dims);
//}
//
//
//MU_TEST(test_instance_free)
//{
//  dlite_instance_decref(inst);
//}
//
//
//MU_TEST(test_entity_free)
//{
//  dlite_entity_decref(entity);
//}


/***********************************************************************/

MU_TEST_SUITE(test_suite)
{
  MU_RUN_TEST(test_collection_create);     /* setup */

  MU_RUN_TEST(test_collection_add);

  MU_RUN_TEST(test_collection_free);       /* tear down */
}



int main()
{
  MU_RUN_SUITE(test_suite);
  MU_REPORT();
  return (minunit_fail) ? 1 : 0;
}
