#!/bin/sh
export CLASSPATH=/usr/local/share/java/antlr-runtime-3.3.jar:/home/iopi/works/hpbcg/src/isatobcg:/home/iopi/works/hpbcg/src/parser/org/hpbcg:/home/iopi/works/hpbcg/src/parser/:.
hpbcg exprEvalHPBCG.hg > exprEvalHPBCG.c
gcc -o exprEvalHPBCG -std=c99 test_exprEvalHPBCG.c
