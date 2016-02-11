#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <hpbcg-asm-common.h>

int	main()
{
      uint64_t startTick = 0; 
      
      printf("COMPILE TIME\n");

      startTick = getTick();
      system("cpp dummy.c > dummy.2.c");
      uint64_t cppref_tick = getTick() - startTick;
      printf("CPP REF %llu\n", cppref_tick);

      startTick = getTick();
      system("gcc -c dummy.2.c");
      uint64_t ref_tick = getTick() - startTick;
      printf("GCC REF %llu\n", ref_tick);

      uint64_t cpprpnStatic_tick = 0;
      do
      {
        startTick = getTick();
        system("cpp rpnStatic.c > rpnStatic.2.c");
        cpprpnStatic_tick = getTick() - startTick - cppref_tick;
      } while ((int64_t)cpprpnStatic_tick < 0);

      uint64_t rpnStatic_tick;
      do
      {
      	startTick = getTick();
      	system("gcc -c rpnStatic.2.c");
	rpnStatic_tick = getTick() - startTick - ref_tick;
      } while((int64_t)rpnStatic_tick < 0);

      uint64_t cpprpnEval_tick;
      do
      {
        startTick = getTick();
        system("cpp rpnEval.c > rpnEval.2.c");
        cpprpnEval_tick = getTick() - startTick - cppref_tick;
      } while ((int64_t)cpprpnEval_tick < 0);

      startTick = getTick();
      system("gcc -c rpnEval.2.c");
      uint64_t rpnEval_tick = getTick() - startTick - ref_tick;
      
      startTick = getTick();
      system("hpbcg test_rpn_x86.hg > rpnCodegen.c");
      uint64_t hpbcg_tick = getTick() - startTick;

      startTick = getTick();
      system("cpp rpnCodegen.c > rpnCodegen.2.c");
      uint64_t cpprpnCodegen_tick = getTick() - startTick - cppref_tick;

      startTick = getTick();
      system("gcc -c rpnCodegen.2.c");
      uint64_t rpnCodegen_tick = getTick() - startTick - ref_tick;

      uint64_t rpnStatic_total = rpnStatic_tick + cpprpnStatic_tick;
      uint64_t rpnEval_total = rpnEval_tick + cpprpnEval_tick;
      uint64_t rpnCodegen_total = rpnCodegen_tick + cpprpnCodegen_tick + hpbcg_tick;
      printf("rpnStatic %llu ticks \n", rpnStatic_total);
      printf("\tICG:cpp %llu ticks %F%%\n", cpprpnStatic_tick, cpprpnStatic_tick * 100.0 / rpnStatic_total);
      printf("\tCT:gcc %llu ticks %F%%\n", rpnStatic_tick, rpnStatic_tick * 100.0 / rpnStatic_total);
      printf("rpnEval %llu ticks x%F\n", rpnEval_total, rpnEval_total * 1.0 / rpnStatic_total);
      printf("\tICG:cpp %llu ticks %F%%\n", cpprpnEval_tick, cpprpnEval_tick * 100.0 / rpnEval_total);
      printf("\tCT:gcc %llu ticks %F%%\n", rpnEval_tick, rpnEval_tick * 100.0 / rpnEval_total);
      printf("rpnCodegen Total %llu ticks x%F\n", rpnCodegen_total, rpnCodegen_total * 1.0 / rpnStatic_total);
      printf("\tECG:Hpbcg %llu ticks %F%%\n", hpbcg_tick, hpbcg_tick * 100.0 / rpnCodegen_total);
      printf("\tICG:cpp %llu ticks %F%%\n", cpprpnCodegen_tick, cpprpnCodegen_tick * 100.0 / rpnCodegen_total);
      printf("\tCT:gcc %llu ticks %F%%\n", rpnCodegen_tick, rpnCodegen_tick * 100.0 / rpnCodegen_total);

      system("gcc -c main.c");
      system("gcc -o benchRpn main.o rpnEval.2.o rpnStatic.2.o rpnCodegen.2.o");
      system("./benchRpn");
}
