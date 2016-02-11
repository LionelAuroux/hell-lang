#include <stdio.h>

static int      function1()
{
        return 42;
}

double          function2(int   bla, double d)
{
        return bla * 1.0 + d;
}

int     main(int ac, char **av, char **env)
{
        for (int i = 0; av[i]; ++i)
        {       printf("arg: %d = %s\n", i, av[i]);}
        for (int i = 0; env[i]; ++i)
        {       printf("env: %d = %s\n", i, env[i]);}
}
