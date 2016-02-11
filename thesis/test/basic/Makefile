LIST_FILE=$(wildcard *.cc)
TARGET = ${LIST_FILE:.cc=}
SRC = ${TARGET}.cc
OBJ = ${SRC:.cc=.o}
LLVM_TARGET = ${TARGET}.stage2
LLVM_SRC := $(wildcard *.stage2.ll)
LLVM_BC=${LLVM_SRC:.stage2.ll=.stage2.bc}
LLVM_AS=${LLVM_BC:.stage2.bc=.stage2.s}
LLVM_OBJ=${LLVM_AS:.stage2.s=.stage2.o}
CPPFLAGS= -g -O3 `llvm-config --cxxflags`
LDFLAGS=`llvm-config --ldflags --libs ` -ldl
CXX=g++

.SUFFIXES : .ll .bc .s .o

.ll.bc 	: $@
	@llvm-as -f $< && echo "Bytecode compiling " $< "Success"

.bc.s	: $@
	@llc -f $< && echo "Generating assembly " $< "Success"

.s.o	: $@
	@as -o $@ $<  && echo "Assembling " $< "Success"

all: ${OBJ}
	@g++ -o ${TARGET} ${OBJ} ${LDFLAGS} && echo "Compiling codegenerator"
	@./${TARGET} > ${TARGET}.stage2.ll && echo "Generating llvm code"
	@echo "--- Generated llvm code ---"
	@cat ${TARGET}.stage2.ll

llvm: ${LLVM_OBJ}
	@gcc -o ${LLVM_TARGET} ${LLVM_OBJ}

clean:
	@rm -f *~ *.o *.s *.bc *.ll && echo "Cleaning temporary files"

fclean: clean
	@rm -f ${TARGET} ${LLVM_TARGET} && echo "Cleaning target files"
