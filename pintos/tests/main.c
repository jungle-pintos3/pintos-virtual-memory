#include <random.h>
#include <stdio.h>  //TODO: 삭제해야함
#include "tests/lib.h"
#include "tests/main.h"

int
main (int argc UNUSED, char *argv[])
{
  test_name = argv[0];

  //TODO: 삭제해야함
  printf("[DEBUG] main: argv=%p, argv[0]=%p, *argv[0]='%s'\n",
         argv, argv[0], argv[0] ? argv[0] : "(null)");

  msg ("begin");
  random_init (0);
  test_main ();
  msg ("end");
  return 0;
}
