#
#	Multi-step compilation for code injection @ llvm level
#	L.Auroux
#

NAME=		Hlcg				# code injector
SRC=		${NAME:=.cc}
OBJ=		${SRC:.cc=.o}
SO=		lib${NAME:=.so.1}
HLCG=		hlcg 				# code injector
HLCGSRC=	main.cc
HLCGOBJ=	${HLCGSRC:.cc=.o}
TEST=           test
TESTSRC=        main_test.cc
TESTOBJ=        ${TESTSRC:.cc=.o}

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
        `llvm-config --ldflags --libs`

CXX=g++

all 	: echo ${SO} install hlcg
	@echo "All is done"

test    : ${SO} install ${TESTOBJ}
	@echo "Compilation of TEST"
	${CXX} -o ${TEST} ${TESTOBJ} `llvm-config --ldflags` -L . -l:${SO}

${SO}   : ${OBJ}
	@echo "Make lib"
	${CXX} -shared -Wl,-soname,${SO} -o ${SO} ${OBJ} ${LDFLAGS}

hlcg	: ${HLCGOBJ} 
	@echo "Compilation of code hlcg"
	${CXX} -o ${HLCG} ${HLCGOBJ} `llvm-config --ldflags` -L . -l:${SO} 

install :
	@sudo cp ${SO} /usr/local/lib
	@sudo ldconfig -f /etc/ld.so.conf

echo :
	@echo compile ${OBJ}

clean 	:
	@rm -f *~ ${OBJ} ${HLCOBJ} ${TESTOBJ}
	@echo "All are cleaned"

fclean	: clean
	@rm -f *~ ${SO} ${HLCG} ${TEST}
	@echo "Inject are cleaned"
