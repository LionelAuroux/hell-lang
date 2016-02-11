#ifdef WITHCOMPLEXSUPPORT
#include <complex.h>
#define tComplex _Complex double
#define RE(var)   __real__ (var)
#define IMAG(var) __imag__ (var)
void cSet(tComplex *a, tReal re, tReal im) { __real__ *a = re;  __imag__ *a = im; } 
tComplex cMul(tComplex a, tComplex b)	{ return a*b; 	} /* cMul */
tComplex cSub(tComplex a, tComplex b)	{ return a-b;   } /* cSub */
tComplex cAdd(tComplex a, tComplex b)   { return a+b;   } /* cAdd */
tReal module(tComplex a)
{
  tReal r = __real__ a;
  tReal i = __imag__ a;
  return a*a + i*i;
} /* module */

void pComplex(tComplex a, FILE *o)
{
  fprintf(o, "%+2.2f+i*%+2.2f ", __real__  a, __imag__ a);
} /* pComplex */

#else  /* Without COMPLEXSUPPORT */

typedef struct { tReal re, im; } tComplex;
#define RE(var)    (var).re
#define IMAG(var)  (var).im
void cSet(tComplex *a, tReal re, tReal im)	{ a->re = re; a->im = im;} 
tComplex cMul(tComplex a, tComplex b)
{
  tComplex tmp;
  tmp.re = a.re * b.re - a.im * b.im;
  tmp.im = a.re * b.im + a.im * b.re;
  return tmp;
} /* cMul */

tComplex cSub(tComplex a, tComplex b)
{
  tComplex tmp;
  tmp.re = a.re - b.re;
  tmp.im = a.im - b.im;
  return tmp;
} /* cSub */

tComplex cAdd(tComplex a, tComplex b)
{
  tComplex tmp;
  tmp.re = a.re + b.re;
  tmp.im = a.im + b.im;
  return tmp;
} /* cAdd */

tReal module(tComplex a){  return a.re*a.re + a.im*a.im;} /* module */
void pComplex(tComplex a, FILE *o){  fprintf(o, "%+2.2f+i*%+2.2f ", a.re, a.im);} /* pComplex */
#endif


typedef struct { 
  union { 
    tComplex complexValues;
    int          intValues[2];
  } u;
} tComplexUnion;

