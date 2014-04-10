
#include <glib.h>
#include <libguile.h>
#include "guile-mappings.h"

#include "engine-helpers.h"
#include "gnc-module.h"
#include "test-engine-stuff.h"
#include "test-stuff.h"
#include "Query.h"
#include "TransLog.h"


static void
test_query (Query *q, SCM val2str)
{
  SCM scm_q;
  SCM str_q;
  SCM res_q;
  SCM args = SCM_EOL;
  Query *q2;
  char * str;

  scm_q = gnc_query2scm (q);
  args = scm_cons (scm_q, SCM_EOL);
  str_q = scm_apply (val2str, args, SCM_EOL);

  args = scm_cons (scm_makfrom0str ("'"), scm_cons (str_q, SCM_EOL));
  str_q = scm_string_append (args);

  str = gh_scm2newstr (str_q, NULL);

  if (str) {
    res_q = scm_c_eval_string (str);
  } else {
    res_q = SCM_BOOL_F;
  }

  q2 = gnc_scm2query (res_q);

  if (!xaccQueryEqual (q, q2))
  {
    failure ("queries don't match");
    fprintf (stderr, "%s\n\n", str);
    scm_q = gnc_query2scm (q2);
    gh_display (scm_q); gh_newline ();
    exit (1);
  }
  else
  {
    success ("queries match");
  }
  if (q2) xaccFreeQuery (q2);
  if (str) free (str);
}

static void
run_tests (void)
{
  Query *q;
  SCM val2str;
  int i;

  val2str = scm_c_eval_string ("gnc:value->string");
  g_return_if_fail (SCM_PROCEDUREP (val2str));

  for (i = 0; i < 244; i++) {
    q = get_random_query ();
    test_query (q, val2str);
    xaccFreeQuery (q);
    printf("%d ", i);
    fflush(stdout);
  }

  { 
    q = get_random_query ();
    test_query (q, val2str);
    xaccFreeQuery (q);
    printf("%d ", i);
    fflush(stdout);
  }

  printf("\n");
}

static void
main_helper (void *closure, int argc, char **argv)
{
  gnc_module_load("gnucash/engine", 0);
  gnc_module_load("gnucash/app-utils", 0);

  xaccLogDisable ();

  /* scm conversion doesn't handle binary atm */
  kvp_exclude_type (KVP_TYPE_BINARY);

  /* double->string->double is not idempotent */
  kvp_exclude_type (KVP_TYPE_DOUBLE);

  /* Initialize to a known RNG position */
  guid_init();
  srand(1);

  run_tests ();

  print_test_results ();

  exit (get_rv ());
}

int
main (int argc, char **argv)
{
  scm_boot_guile (argc, argv, main_helper, NULL);
  return 0;
}