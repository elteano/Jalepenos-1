/*
 * execarray.c
 *
 * A simple program which executes 'array', and tests the return status.
 */

#include "syscall.h"

int
main ()
{
    int id = 1552;
    id = Exec("../test/array\0", 0, 0, 0);
    Exit(id);
}

