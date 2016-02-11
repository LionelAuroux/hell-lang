#include "head.inc.c"
#include "exprEvalHPBCG.c"

int     main(int ac, char **argv, char **env)
{
        if (ac < 2)
        {
                printf("Usage: %s EXPR\n", argv[0]);
                printf("we using env for value mapping\n");
                exit(-42);
        }
        map_t   mapping = createMap(env);
        int     res = 0;
        if (exprEval(&argv[1], mapping, &res))
        {       printf("%d\n", res);}
}
