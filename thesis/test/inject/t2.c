#include <stdio.h>
#include <stdarg.h>
#include <sys/mman.h>

// need some adhoc functions
#include "compilette.h"

int	foo(char *s, int a, int b, int c)
{
	printf(s, a, b, c, printf);
	return a * b + c - 666;
}

// allow to compile foo and provide some adhoc data and types for foo 
COMPIL(foo);

int	main()
{
	// create a specialized function with constant value 666 change into 999
	ptr_foo	new_foo = (ptr_foo) compilette(JIT_NAME(foo), SIZE_NAME(foo), RELOC_NAME(foo), CONST_NAME(foo), 40, 999, -1);
	printf("TEST:%d\n", new_foo("//compilette version: %d %d %d %p\n", 1, 2, 3));
}
