//#include <stdio.h>
#include <stdarg.h>
#include <sys/mman.h>

// qq macro {jit_,size_,reloc_,ptr_}*
#include "compilette.h"

int printf(const char *, ...);

int	mafonction(char *s, int a, int b, int c)
{
	printf(s, a, b, c, printf);
	return a * b + c - 666;
}

// toujours apres les fonctions
COMPIL(mafonction);

float	avecfloat(float a, float b, float c)
{
	a += a + (float)123456;
	b *= b + (float)234567;
	c += (float)345678;
	printf("version double\n");
	return a * b + c - 3.456789;
}

COMPIL(avecfloat);

double avecdouble(double a, double b, double c)
{
	return a * b + c;
}

long double avecextended(long double a, long double b, long double c)
{
	return a * b + c;
}

// avec vecteurs
typedef __attribute__(( ext_vector_type(4) )) float float4;
typedef __attribute__(( ext_vector_type(8) )) float float8;
typedef __attribute__(( ext_vector_type(4) )) double double4;
typedef __attribute__(( ext_vector_type(8) )) double double8;

float4	avf4(float4 a, float4 b, float4 c)
{
	return a * b + c;
}

float8	avf8(float8 a, float8 b, float8 c)
{
	return a * b + c;
}

double4	avd4(double4 a, double4 b, double4 c)
{
	return a * b + c;
}

double8	avd8(double8 a, double8 b, double8 c)
{
	return a * b + c;
}

int	main()
{
	printf("ptr to printf:%p\n", (void*)printf);
	printf("TEST:%d\n", mafonction("preliminaire %d %d %d\n", 3,4,5));
	avecfloat(1.2, 2.3, 3.4);

	if (size_mafonction != 0)
	{
		ptr_mafonction	new_mafonction = (ptr_mafonction) compilette(JIT_NAME(mafonction), SIZE_NAME(mafonction), RELOC_NAME(mafonction), 
			CONST_NAME(mafonction), 40, 999, -1);
		printf("TEST2:%d\n", new_mafonction("//version compilette: %d %d %d %p\n", 1, 2, 3));
		munmap(new_mafonction, size_mafonction);
	}
}
