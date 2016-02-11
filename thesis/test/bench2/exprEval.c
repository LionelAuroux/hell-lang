int     exprTerminal(char **expr, map_t values, int *val)
{
        char *str = *expr;
        char buf[32];
        int n;
        if (sscanf(str, "%[0-9]%n", buf, &n)) 
        {
                *val = strtol(buf, 0, 0);
                str += n;
                printf("literal :%s[%d] = %d\n", buf, n, *val);
                goto good;
        }
        else if (sscanf(str, "%[a-zA-Z]%n", buf, &n))
        {
                *val = getVal(values, buf);
                str += n;
                printf("VARIABLE :%s[%d] = %d\n", buf, n, *val);
                goto good;
        }
        return 0;

good:
        *expr = str;
        return 1;
}

int     exprChar(char **expr, char c)
{
        if (**expr == c)
        {       
                ++*expr;
                return 1;
        }
        return 0;
}

int     exprEvalH(char **expr, map_t values, int *res)
{
        if (exprTerminal(expr, values, res))
        {
                int tmp = 0;
                while (exprChar(expr, '*') && exprTerminal(expr, values, &tmp) && ({*res *= tmp; 1;})
                       || exprChar(expr, '/') && exprEvalH(expr, values, &tmp) && ({*res /= tmp; 1;})
                       || exprChar(expr, '%') && exprEvalH(expr, values, &tmp) && ({*res %= tmp; 1;})
                       );
                return 1;
        }
        return 0;
}

int     exprEval(char **expr, map_t  values, int *res)
{
        if (!*expr)
        {       
                printf("bad expr pointer\n");
                abort();
        }
        if (exprEvalH(expr, values, res))
        {
                int tmp = 0;
                while (exprChar(expr, '+') && exprEvalH(expr, values, &tmp) && ({*res += tmp; 1;})
                       || exprChar(expr, '-') && exprEvalH(expr, values, &tmp) && ({*res -= tmp; 1;})
                       );
                return 1;
        }
        return 0;
}
