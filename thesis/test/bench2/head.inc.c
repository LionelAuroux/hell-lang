#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {char *field; int value;} assoc_t;
typedef assoc_t*                       map_t;

int     getVal(map_t values, char *s)
{
        for (int i = 0; values[i].field; ++i)
                if (strcmp(s, values[i].field) == 0)
                        return values[i].value;
        printf("unknown value %s\n", s);
        abort();
}

// make a mapping
map_t   createMap(char **env)
{
        int count = 0;
        for (count = 0; env[count]; ++count)
                /*printf("%s\n", env[count])*/;
        map_t   mapping = calloc(count, sizeof(assoc_t));
        for (int i = 0; i < count; ++i)
        {
                for (int pos = 0; env[i][pos]; ++pos)
                        if (env[i][pos] == '=')
                        {
                                mapping[i].field = calloc(pos, sizeof(char));
                                strncpy(mapping[i].field, env[i], pos);
                                mapping[i].value = atoi(&env[i][pos + 1]);
                                break;
                        }
        }
        /*for (int i = 0; i < count; ++i)*/
        /*{       printf("%s = %d\n", mapping[i].field, mapping[i].value);}*/
        return mapping;
}

