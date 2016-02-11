#include "Hlcg.hh"

#include "llvm/Support/CommandLine.h"
#include "llvm/DerivedTypes.h"
#include "llvm/ValueSymbolTable.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"

// pour la fonction system() :(
#include <stdlib.h>
#include <stdio.h>

using namespace llvm;
using namespace std;
using namespace tool;


void	showProto(Module::iterator &itb)
{
	const FunctionType	*typef = itb->getFunctionType();
	FunctionType::param_iterator	pitb = typef->param_begin();
	FunctionType::param_iterator	pite = typef->param_end();
	cerr << "Ret type:" << typef->getReturnType()->getDescription() << endl;
	for (; pitb != pite; ++pitb)
	{	cerr << "param:" << pitb->get()->getDescription() << endl;}
}

void	changeType(Module::iterator &itb)
{
	BasicBlock	&bentry = itb->getEntryBlock();	
	BasicBlock::iterator	bitb, bite;
	bitb = bentry.begin();
	bite = bentry.end();
	cerr << "total code size:" << bentry.size() << endl;
	for (; bitb != bite; ++bitb)
	{
		cerr << "entryblock:" << bitb->getOpcodeName() << endl;
	}
	Function::BasicBlockListType	&listblock = itb->getBasicBlockList();
	Function::iterator jitb, jite;
	jitb = listblock.begin();
	jite = listblock.end();
	for (; jitb != jite; ++jitb)
	{
		cerr << "basicblock:" << jitb->begin()->getOpcodeName() << " size:" << jitb->size() << endl;
	}
	ValueSymbolTable	&rsymtab = itb->getValueSymbolTable();
	cerr << "DUMP SYM TABLE" << endl;
	rsymtab.dump();
}

void	jitFunction(ExecutionEngine &exec, Module::iterator &f)
{
	typedef void (*print)(const char*, int, int, int);
	void	*pf = exec.recompileAndRelinkFunction(f);
	//test
	((print)pf)("//version normale: %d %d %d %p\n", 2,3,4);
}

int	main(int ac, char **av)
{
	if (ac != 2)
	{
		cerr << "USAGE: " << av[0] << " file.bc" << endl;
		exit(42);
	}

	Injector	inject;
	string		input = av[1];
	string		output = input.substr(0, input.find(".bc"));
	output += ".BC";

	printf("myadr:%p\n", printf);

	cerr << "adr printf:" << hex << (int)printf << endl;
	cerr << "IN:" << input << " OUT:" << output << endl;
	inject.loadIRFile(input);
	CodeGeneratorListener	jit_listen(inject);
	
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
			cerr << "JIT IT!" << endl;
			jitFunction(inject.getEEngine(), itb);
		}
		if (itb->getNameStr().compare("craft") == 0)
		{
			cerr << "JIT IT!" << endl;
			jitFunction(inject.getEEngine(), itb);
		}
	}
	jit_listen.writeReloc();
	jit_listen.writeConst();
	jit_listen.compileFile();
	inject.linkIRFile(jit_listen.getFileName(), output);
}
