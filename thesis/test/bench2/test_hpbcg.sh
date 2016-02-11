#!/bin/bash
a=2 b=3 c=4 d=8 ./exprEval a+b*c-d
a=2 b=3 c=4 d=8 ./exprEvalHPBCG a+b*c-d
cat out_exprEvalHPBCG.txt | llvm-mc -disassemble
