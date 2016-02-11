// compilette avec llvm 
#ifndef _COMPILETTE_H
#define _COMPILETTE_H

#include <stdint.h>
#include <stdarg.h>

typedef struct {int callAddrAbsolute; int callAddrRelat; int offset; int instSize;} jitRelocInfo;
typedef struct {int index; int64_t value; int offset; int constSize;} ConstInfo;
void	*compilette(void *func_ptr, int func_size, jitRelocInfo *reloc, ConstInfo *cvalue, ...);

static inline uint64_t	rdtsc()
{
	uint32_t	hi, lo;
	__asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
	return (uint64_t)hi << 32 | lo;
}

#define JIT_NAME(X)	jit_##X
#define SIZE_NAME(X)	size_##X
#define RELOC_NAME(X)	reloc_##X
#define PTR_NAME(X)	ptr_##X
#define CONST_NAME(X)   const_##X

// a utiliser apres la declaration/definition d'une fonction et cree toutes les variables/types utiles
#define COMPIL(X) 				\
	extern int 		SIZE_NAME(X);	\
	extern char 		JIT_NAME(X)[];	\
	extern jitRelocInfo 	RELOC_NAME(X)[];\
	extern ConstInfo	CONST_NAME(X)[];\
	typedef typeof(X)	*PTR_NAME(X);
#endif
