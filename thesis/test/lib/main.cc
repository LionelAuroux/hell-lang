#include "Hlcg.hh"

#include "llvm/Support/CommandLine.h"
#include "llvm/DerivedTypes.h"
#include "llvm/ValueSymbolTable.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/Support/raw_ostream.h"

// pour la fonction system() :(
#include <stdlib.h>
#include <stdio.h>

using namespace llvm;
using namespace clang;
using namespace std;
using namespace tool;

// GLOBAL OPTION

JITEventListener::~JITEventListener() {}/// TODO: avoid this fuuuuu
PPCallbacks::~PPCallbacks() {}

static cl::opt< bool >
dummyBeginParameter("a", cl::desc("dummy parameter"), cl::Hidden);

// inputFilename - The filename to read from.
static cl::opt< std::string >
inputFilename(cl::Positional, cl::desc("<input bitcode file>"), cl::init(""), cl::value_desc("input filename"));

static cl::list< std::string >
listFunc("f", cl::desc("Specify function to compilette"), cl::ZeroOrMore, cl::value_desc("function"));

// --

void    showProto(Module::iterator &itb)
{
        const FunctionType	*typef = itb->getFunctionType();
        FunctionType::param_iterator	pitb = typef->param_begin();
        FunctionType::param_iterator	pite = typef->param_end();
        string  type_str;
        llvm::raw_string_ostream        rso(type_str);
        typef->getReturnType()->print(rso);
        cerr << "Ret type:" << rso.str() << endl;
        for (; pitb != pite; ++pitb)
        {       
                string  type_str;
                llvm::raw_string_ostream        rso(type_str);
                (*pitb)->print(rso);
                cerr << "param:" << rso.str() << endl;
        }
}

void    changeType(Module::iterator &itb)
{
        BasicBlock      &bentry = itb->getEntryBlock();	
        BasicBlock::iterator	bitb, bite;
        bitb = bentry.begin();
        bite = bentry.end();
        cerr << "total code size:" << bentry.size() << endl;
        for (; bitb != bite; ++bitb)
        {
                cerr << "entryblock:" << bitb->getOpcodeName() << endl;
        }
        Function::BasicBlockListType    &listblock = itb->getBasicBlockList();
        Function::iterator jitb, jite;
        jitb = listblock.begin();
        jite = listblock.end();
        for (; jitb != jite; ++jitb)
        {
                cerr << "basicblock:" << jitb->begin()->getOpcodeName() << " size:" << jitb->size() << endl;
        }
        ValueSymbolTable        &rsymtab = itb->getValueSymbolTable();
        cerr << "DUMP SYM TABLE" << endl;
        rsymtab.dump();
}

void    jitFunction(ExecutionEngine &exec, Module::iterator &f)
{
        typedef void (*print)(const char*, int, int, int);
        void	*pf = exec.recompileAndRelinkFunction(f);
        //test
        ((print)pf)("//version normale: %d %d %d %p\n", 2,3,4);
}

int     main(int ac, char **av)
{
        // workaround for desactivating global flags for llvm option
        cl::Option*	param = dummyBeginParameter.getNextRegisteredOption();
        while (param)
        {
                param->setHiddenFlag(cl::ReallyHidden);
                param = param->getNextRegisteredOption();
        }
        dummyBeginParameter.setHiddenFlag(cl::ReallyHidden);

        cl::ParseCommandLineOptions(ac, av, "hlcg - a tool to do compilette at high level\n");

        if (inputFilename.compare("") == 0)
        {
                cerr << "No input file" << endl;
                //TODO -> gen auto usage from cl module
                exit(42);
        }

        cl::list< std::string >::iterator       fitb, fite;
        fitb = listFunc.begin();
        fite = listFunc.end();
        for (; fitb != fite; ++fitb)
        {
                cerr << "F:" << *fitb << endl;
        }


        //CFrontEnd       front(inputFilename);
        //cerr << "IN CLANG" << inputFilename << endl;
        //return 42;

        Hlcg            inject;
        string          input = inputFilename;
        string          output = input.substr(0, input.find(".bc"));
        output += ".BC";

        printf("myadr:%p\n", printf);

        cerr << "adr printf:" << hex << (int)printf << endl;
        cerr << "IN:" << input << " OUT:" << output << endl;
        inject.loadIRFile(input);
        CodeGeneratorListener   jit_listen(inject);
	
        jit_listen.setBaseFileName(input);
        jit_listen.initFile();
        // itere les modules pour JITTER certaine fonction
        Module::iterator itb = inject.getModule().begin();
        Module::iterator ite = inject.getModule().end();
        for (; itb != ite; ++itb)
        {
                cerr << "Func:" << itb->getNameStr() << endl;
		/*
		if (itb->getNameStr().compare("craft") == 0)
		{	
			showProto(itb);
			changeType(itb);
		}*/
                if (itb->getNameStr().compare("mafonction") == 0)
                {
			showProto(itb);
                        cerr << "JIT IT!" << endl;
                        jitFunction(inject.getEEngine(), itb);
                }
                if (itb->getNameStr().compare("craft") == 0)
                {
			showProto(itb);
                        cerr << "JIT IT!" << endl;
                        jitFunction(inject.getEEngine(), itb);
                }
        }
        jit_listen.writeReloc();
        jit_listen.writeConst();
        jit_listen.compileFile();
        inject.linkIRFile(jit_listen.getFileName(), output);
}
