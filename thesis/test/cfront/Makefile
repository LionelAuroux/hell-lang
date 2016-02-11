##
##  cfront_test by Auroux.l
##
NAME    =               cfront
SRC     =		main.cc
OBJ     =		${SRC:.cc=.o}

CPPFLAGS=		-g -O3 -fPIC `llvm-config --cxxflags` -I `llvm-config --src-root` -I .  -fno-rtti
LDFLAGS=		-g \
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

CXX	=		g++

all	:		${OBJ}
	${CXX} -o ${NAME} ${OBJ} ${LDFLAGS}
	@echo "All is done"

clean 	:
	@rm -f *~ .*.swp ${OBJ}
	@echo "All are cleaned"

fclean	: clean
	@rm -f ${NAME}
	@echo "Program are cleaned"
