#include <stdio.h>
#include <stdlib.h>

int rpnEval(char *expr, int value)
{
  int Stack[10];
  int top = 0;
  Stack[top++] = value;
  while (*expr)
    {
      char buf[32];
      int n;
      if (sscanf(expr, "%[0-9]%n", buf, &n)) 
	{
	  expr += n - 1;
	  Stack[top++] = strtol(buf, 0, 0);
	 }
      else if (*expr == '+') 
	{ Stack[top-2] = Stack[top-2] + Stack[top-1]; top--; }
      else if (*expr == '-') 
	{ Stack[top-2] = Stack[top-2] - Stack[top-1]; top--; }
      else if (*expr == '*')
	{ Stack[top-2] = Stack[top-2] * Stack[top-1]; top--; }
      else if (*expr == '/')
	{ Stack[top-2] = Stack[top-2] / Stack[top-1]; top--; }
      else {
	fprintf(stderr, "cannot compile: %s\n", expr);
	abort();
      }
      ++expr;
    }
  if (1 != top)
    {
      fprintf(stderr, "incorrect stack pointer\n");
      abort();
    }
  return Stack[0];
} /* rpnEval */
