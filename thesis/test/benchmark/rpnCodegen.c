/* Processor description from :/usr/local//include/hpbcg-power4.lst */
/* Processor description from :/usr/local//include/hpbcg-ia64.lst */
/* Processor description from :/usr/local//include/hpbcg-power4.lst */
/* Processor description from :/usr/local//include/hpbcg-x86.lst */
/* Processor description from :/usr/local//include/hpbcg-armthumb.lst */
/* -*- C -*- */

#include <hpbcg-x86.h> /* #cpu x86*/

#include <stdio.h>
#include <stdlib.h>


typedef int (*pifi)(int);


pifi rpnCompile(char *expr)
{
  insn *code= (insn *)malloc(1024);
  ASM_1_BEGIN 
ORG(code);
push_iR(5);
mov_iRR(4,5);
mov_iIRR(8,5,0);
ASM_1_END 
  while (*expr)
    {
      char buf[32];
      int n;
      // lit un nombre
      if (sscanf(expr, "%[0-9]%n", buf, &n)) 
	{
	  expr+= n - 1;
	  n = strtol(buf, 0, 0);
	  // modifie eax
	  ASM_1_BEGIN 
push_iR(0);
mov_iIR(n,0);
ASM_1_END  
	 }
      // tete de pile dans ecx 
      else if (*expr == '+') ASM_1_BEGIN 
pop_iR(1);
add_iRR(1,0);
ASM_1_END  else if (*expr == '-') ASM_1_BEGIN 
mov_iRR(0,1);
pop_iR(0);
sub_iRR(1,0);
ASM_1_END  else if (*expr == '*') ASM_1_BEGIN 
pop_iR(1);
imul_iRR(1,0);
ASM_1_END  else if (*expr == '/') ASM_1_BEGIN 
mov_iRR(0,1);
pop_iR(0);
cltd_i();
idiv_iR(1);
ASM_1_END  else {
	fprintf(stderr, "cannot compile: %s\n", expr);
	abort();
      }
      ++expr;
    }
  ASM_1_BEGIN 
leave_i();
ret_i();
ASM_1_END ;
  iflush (code, hpbcg_asm_pc);
  printf("Code generated\n");
  return (pifi)code;
}


