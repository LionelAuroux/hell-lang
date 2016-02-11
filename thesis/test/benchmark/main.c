#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <hpbcg-asm-common.h>

typedef int (*pifi)(int);

pifi rpnCompile(char *expr);

int __attribute__((noinline)) static_f2c(int i);

int __attribute__((noinline)) static_c2f(int i);

int rpnEval(char *expr, int value);

int main(int argc, char * argv[])
{
  int i;
  printf("RUNTIME\n");
  if (argc == 1)
    {
      uint64_t startTick = getTick();
      pifi c2f = rpnCompile("9*5/32+");
      pifi f2c = rpnCompile("32-5*9/");
      uint64_t codegen_tick = getTick() - startTick;

      printf("\nC:");
      for (i = 0; i <= 100; i+= 10) printf("%3d ", i);
      printf("\nF:");
      for (i = 0; i <= 100; i+= 10) printf("%3d ", c2f(i));
      printf("\n");
      printf("\nF:");
      for (i = 32; i <= 212; i+= 10) printf("%3d ", i);
      printf("\nC:");
      for (i = 32; i <= 212; i+= 10) printf("%3d ", f2c(i));
      printf("\n");

      startTick = getTick();
      for (i = 0; i  <= 100; i+= 10)  	rpnEval("9*5/32+", i);
      for (i = 32; i <= 212; i+= 10) 	rpnEval("32-5*9/", i);
      uint64_t interpret_tick = getTick() - startTick;

      startTick = getTick();
      for (i = 0; i  <= 100; i+= 10)  	c2f(i);
      for (i = 32; i <= 212; i+= 10) 	f2c(i);
      uint64_t dynamic_tick = getTick() - startTick;

      startTick = getTick();
      for (i = 0; i  <= 100; i+= 10)  	static_c2f(i);
      for (i = 32; i <= 212; i+= 10) 	static_f2c(i);
      uint64_t static_tick = getTick() - startTick;

      printf("%llu ticks static compiled\n", static_tick);
      printf("%llu ticks dynamycally compiled x%F\n", dynamic_tick, dynamic_tick * 1.0 / static_tick);
      printf("%llu ticks interpreted x%F\n", interpret_tick, interpret_tick * 1.0 / static_tick);
      printf("%llu ticks code generation overhead x%F (take x%F interpretation)\n", codegen_tick, codegen_tick * 1.0 / static_tick, codegen_tick * 1.0 / interpret_tick);
    }
  else
    {
      pifi fun = rpnCompile (argv[1]);
      printf("\n:");
      for (i = 0; i <= 100; i+= 10) printf("%4d ", i);
      printf("\n:");
      for (i = 0; i <= 100; i+= 10) printf("%4d ", fun(i));
      printf("\n");
    }
  return 0;
}
