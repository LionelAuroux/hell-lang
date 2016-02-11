/* Processor description from :/usr/local//include/hpbcg-power4.lst */
/* Processor description from :/usr/local//include/hpbcg-ia64.lst */
/* Processor description from :/usr/local//include/hpbcg-power4.lst */
/* Processor description from :/usr/local//include/hpbcg-x86.lst */

#include <hpbcg-x86.h> /* #cpu x86*/

int     exprTerminal(char **expr, map_t values, insn **rcode)
{
        int     val;
        char    *str = *expr;
        char    buf[32];
        int     n;
        insn    *code = *rcode;
        if (sscanf(str, "%[0-9]%n", buf, &n)) 
        {
                val = strtol(buf, 0, 0);
                ASM_1_BEGIN 
ORG(code);
mov_iIR(val,0);
push_iR(0);
ASM_1_END ;
                *rcode = hpbcg_asm_pc;
                str += n;
                printf("literal :%s[%d] = %d\n", buf, n, val);
                goto good;
        }
        else if (sscanf(str, "%[a-zA-Z]%n", buf, &n))
        {
                val = getVal(values, buf);
                ASM_1_BEGIN 
ORG(code);
mov_iIR(val,0);
push_iR(0);
ASM_1_END ;
                *rcode = hpbcg_asm_pc;
                str += n;
                printf("VARIABLE :%s[%d] = %d\n", buf, n, val);
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


int     exprEvalH(char **expr, map_t values, insn **rcode)
{
        if (exprTerminal(expr, values, rcode))
        {
                while (exprChar(expr, '*') && exprTerminal(expr, values, rcode) && 
                                ({
                                        insn    *code = *rcode;
                                        ASM_1_BEGIN 
ORG(code);
pop_iR(3);
pop_iR(0);
imul_iRR(3,0);
push_iR(0);
ASM_1_END ;
                                        *rcode = hpbcg_asm_pc;
                                        1;
                                })
                       || exprChar(expr, '/') && exprEvalH(expr, values, rcode) && 
                                ({
                                        insn    *code = *rcode;
                                        ASM_1_BEGIN 
ORG(code);
pop_iR(3);
pop_iR(0);
idiv_iR(3);
push_iR(0);
ASM_1_END ;
                                        *rcode = hpbcg_asm_pc;
                                        1;
                                })
                       || exprChar(expr, '%') && exprEvalH(expr, values, rcode) && 
                                ({
                                        insn    *code = *rcode;
                                        ASM_1_BEGIN 
ORG(code);
pop_iR(3);
pop_iR(0);
idiv_iR(3);
push_iR(2);
ASM_1_END ;
                                        *rcode = hpbcg_asm_pc;
                                        1;
                                })
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

        insn *code= (insn *)malloc(1024);
        insn *refcode = code;
        ASM_1_BEGIN 
ORG(code);
push_iR(5);
mov_iRR(4,5);
ASM_1_END ;
        code = hpbcg_asm_pc;
        
        if (exprEvalH(expr, values, &code))
        {
                while (exprChar(expr, '+') && exprEvalH(expr, values, &code) && 
                                ({
                                        ASM_1_BEGIN 
ORG(code);
pop_iR(3);
pop_iR(0);
add_iRR(3,0);
push_iR(0);
ASM_1_END ;
                                        code = hpbcg_asm_pc;
                                        1;
                                 })
                       || exprChar(expr, '-') && exprEvalH(expr, values, &code) && 
                                ({
                                        ASM_1_BEGIN 
ORG(code);
pop_iR(3);
pop_iR(0);
sub_iRR(3,0);
push_iR(0);
ASM_1_END ;
                                        code = hpbcg_asm_pc;
                                        1;
                                 })
                       );
                ASM_1_BEGIN 
pop_iR(0);
leave_i();
ret_i();
ASM_1_END ;
                iflush (refcode, hpbcg_asm_pc);
                FILE    *debug = fopen("out_exprEvalHPBCG.txt", "w+");
                for (int i = 0; i < (hpbcg_asm_pc - refcode); ++i)
                {
                        fprintf(debug, "0x%X ", refcode[i]);
                }
                fprintf(debug, "\n");
                fclose(debug);

                typedef int (*func)();
                func    compu = (func) refcode;
                *res = compu();
                return 1;
        }
        return 0;
}
