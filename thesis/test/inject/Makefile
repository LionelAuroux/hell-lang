#
#	Multi-step compilation for code injection @ llvm level
#	L.Auroux
#

SRCC:=		test.c # $(wildcard *.c)			# target program
BC=		${SRCC:.c=.bc}
BC2=		${BC:.bc=.BC}
END=		${BC2:.BC=.end}
ENDBC=		${END:.end=.end.bc}
INJECT=		inject 				# code injector
INJSRC=		inject.cc
INJOBJ=		${INJSRC:.cc=.o}

CPPFLAGS= -g -O3 `llvm-config --cxxflags` -I `llvm-config --src-root` -I ../lib
CFLAGS=-I ../lib -I /usr/include/i386-linux-gnu
LDFLAGS=`llvm-config --ldflags --libs Support` -L../lib -l:libHlcg.so.1	
CXX=g++

.SUFFIXES : .c .bc .BC .s .end

.c.bc	: $@
	@clang ${CFLAGS} -emit-llvm -o $@ -c $< && echo clang $< "Success"

.bc.BC	: $@
	@../lib/hlcg -f mafonction $< && echo inject $< "Success"

.BC.end	: $@
	#@rm -f *.inject.c *.inject.bc
	@mv $< $@.bc && touch $@ && echo $@

all 	: bcode program
	@echo "Compilation done..."

program : ${END} 
	@clang -g ${ENDBC} -o test
	@echo "Linking done"

inject	: ${INJOBJ} 
	@echo "Compilation of code injector"
	${CXX}	-o ${INJECT} ${INJOBJ} ${LDFLAGS}

bcode	: ${BC2}
	@echo "ByteCode done..."

clean 	:
	@rm -f *~  ${BC2} ${END} ${ENDBC} *.inject.c *.inject.bc *.o
	@echo "All are cleaned"

fclean 	: clean
	@rm -f test ${BC}
	@echo "Target are cleaned"

iclean	:
	@rm -f *~ ${INJOBJ} ${INJECT}
	@echo "Inject are cleaned"
