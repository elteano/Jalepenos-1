#include "syscall.h"

int
main()
{
    int result = 1000;
    result = Exec("../test/FILEDOESNOTEXIST", 0, 0, 0);
    Exit(result);


}