#
#	Makefile for cfrontend of clang
#	L.Auroux
#

NAME=		cfront				# final binary
SRC=		CFrontEnd.cc main.cc
OBJ=		${SRC:.cc=.o}

CPPFLAGS=-g -O3 -fPIC `llvm-config --cxxflags` -I `llvm-config --src-root` -I .  -fno-rtti
LDFLAGS=-g \
        -lclangFrontendTool \
        -lclangFrontend \
        -lclangDriver \
        -lclangSerialization \
        -lclangCodeGen \
        -lclangParse \
        -lclangSema \
        -lclangStaticAnalyzerFrontend \
        -lclangStaticAnalyzerCheckers \
        -lclangStaticAnalyzerCore \
        -lclangAnalysis \
        -lclangIndex \
        -lclangARCMigrate \
        -lclangRewrite \
        -lclangAST \
        -lclangLex\
        -lclangBasic \
        `llvm-config --ldflags --libs` \
        -ldl 

CXX=g++

all 	: echo ${NAME}
	@echo "All is done"

${NAME} : ${OBJ}
	@echo "Compiling"
	${CXX} -o ${NAME} ${OBJ} ${LDFLAGS}

echo :
	@echo compile ${OBJ}

clean 	:
	@rm -f *~ .*.swp ${OBJ}
	@echo "Objs are cleaned"

fclean	: clean
	@rm -f *~ ${NAME}
	@echo ${NAME} " are cleaned"
