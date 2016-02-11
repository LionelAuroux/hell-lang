#!/bin/sh
export CLASSPATH=/usr/share/java/antlr3.jar:/home/iopi/works/hpbcg/src/isatobcg:/home/iopi/works/hpbcg/src/parser/org/hpbcg:/home/iopi/works/hpbcg/src/parser/:.
gcc -o build build.c
./build
rm *.2.c
rm *.o
rm build
