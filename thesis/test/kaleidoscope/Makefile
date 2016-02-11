TARGET = toy
SRC = ${TARGET}.cc
OBJ = ${SRC:.cc=.o}
CPPFLAGS= -g -O3 `llvm-config --cppflags`
LDFLAGS=`llvm-config --ldflags --libs core interpreter jit native`

all: ${OBJ}
	g++ -o ${TARGET} ${OBJ} ${LDFLAGS}

clean:
	rm -f *~ *.o

fclean: clean
	rm -f ${TARGET}