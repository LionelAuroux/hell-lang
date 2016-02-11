#include "llvm/DerivedTypes.h"
#include "llvm/ValueSymbolTable.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/PassManager.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Support/IRBuilder.h"
#include "llvm/ExecutionEngine/JITEventListener.h" // pour le JIT
#include "llvm/Bitcode/ReaderWriter.h" // pour ecrire l'IR
#include "llvm/Bitcode/BitstreamWriter.h" // pour ecrire l'IR

#include <iostream>
#include <vector>
using namespace llvm;
using namespace std;

// unite de compilation
static Module *TheModule;

// instancie un builder sur le context global
static IRBuilder<> Builder(getGlobalContext());

//hack IO
#include <unistd.h>

// hook pour l'evenement de compilation de l'IR en natif
class	MyJITListener : public JITEventListener
{
public:
  MyJITListener();
  virtual ~MyJITListener();
  virtual void 	NotifyFunctionEmitted(const Function &F, void *Code, size_t Size, 
				      const EmittedFunctionDetails &Details);  
  virtual void 	NotifyFreeingMachineCode(void *OldPtr);
};

// constructeur/destructeur basique
MyJITListener::MyJITListener()  : JITEventListener() {std::cerr << "chargement du listener de JIT" << std::endl;}
MyJITListener::~MyJITListener() {}

// appele lors de la compilation de la fonction dans l'unite de compile
void 	MyJITListener::NotifyFunctionEmitted(const Function &F, void *Code, size_t Size, 
						     const EmittedFunctionDetails &Details)
{
  cerr << "EMIT JIT CODE DE FONCTION :" // << F.getName()
       << endl;
  cerr << "DUMP NATIVE CODE:" << endl;
  for (int i = 0; i < Size; ++i)
    {
      cerr << hex << (int)((unsigned char*)Code)[i] << " ";
    }
  cerr << endl;
  cerr << "DECOUPAGE PAR LIGNE:" << endl;
  for (int i = 0; i < Details.LineStarts.size(); ++i)
    {
      cerr << "slot " << dec << i << ": " << hex << Details.LineStarts[i].Address << endl;
    }
}

// lorsque le code est ecrase
void 	MyJITListener::NotifyFreeingMachineCode(void *OldPtr)
{
  cerr << "FREE JIT CODE :" << hex << OldPtr << endl;
}

int	main()
{
  // instancie ce qu'il faut pour une cible de compilation local
  InitializeNativeTarget();

  // cree le contexte global
  LLVMContext &Context = getGlobalContext();

  // Cree un module
  TheModule = new Module("Mon TEST A MOI", Context);

  // cree le prototype de printf
  std::vector<const Type*>	proto_printf;
  // premier parametre un i8*
  proto_printf.push_back(Type::getInt8PtrTy(getGlobalContext()));
  // proto de type: void printf(i8*, ...);
  FunctionType	*type_printf = FunctionType::get(Type::getVoidTy(getGlobalContext()), proto_printf, true);
  // cree une fonction dans le module
  Function	*f_printf = Function::Create(type_printf, Function::ExternalLinkage, "printf", TheModule);

  // cree un prototype de fonction pour le main
  std::vector<const Type*>	protof;
  // int argc
  protof.push_back(Type::getInt32Ty(getGlobalContext()));
  // cree un **i8 pour le char** argv
  const Type *simple_i8 = Type::getInt8Ty(getGlobalContext());
  PointerType *ptr_i8 = PointerType::getUnqual(simple_i8);
  PointerType *ptr_ptr_i8 = PointerType::getUnqual(ptr_i8);
  protof.push_back(ptr_ptr_i8);
  protof.push_back(ptr_ptr_i8);
  // cree un fonction type du prototype
  FunctionType	*ft = FunctionType::get(Type::getInt32Ty(getGlobalContext()), protof, false);
  // cree une fonction dans le module
  Function	*f = Function::Create(ft, Function::ExternalLinkage, "main", TheModule);
  
  // donne des noms au argument
  Function::arg_iterator	alist = f->arg_begin();
  alist->setName("ac");++alist;
  alist->setName("av");++alist;
  alist->setName("ae");
  
  
  // cree un  block de code dans la fonction
  BasicBlock	*bb = BasicBlock::Create(getGlobalContext(), "entry", f);
  Builder.SetInsertPoint(bb);

  // Cree 2 constante
  Value	*v1 = ConstantInt::get(getGlobalContext(), APInt(32, 66, false));
  Value	*v2 = ConstantInt::get(getGlobalContext(), APInt(32, 12, false));
  // Cree une addition entre les 2
  Value *res1 = Builder.CreateAdd(f->getValueSymbolTable().lookup("ac"), v1, "res1");
  Value *res2 = Builder.CreateMul(f->getValueSymbolTable().lookup("ac"), v2, "res2");
  Value *res = Builder.CreateAdd(res1, res2, "res");
    
  // cree une variable globale de type chaine de caractere
  Value *pstr = Builder.CreateGlobalString(";c cool:%d\n", ".str");
    
  // cree le getelementptr avec le bon niveau d'indirection pour notre const char* global
  std::vector<Value*> ops;
  ops.push_back(ConstantInt::get(getGlobalContext(), APInt(32, 0, false)));
  ops.push_back(ConstantInt::get(getGlobalContext(), APInt(32, 0, false)));
  Value *the_gep = Builder.CreateGEP(pstr, ops.begin(), ops.end(), "pstr");
  
  // empile les arguments
  std::vector<Value*> printf_args;
  printf_args.push_back(the_gep);
  printf_args.push_back(res);
  // cree un appel a printf
  // ATTENTION: ne pas nommer les fonctions VOID
  Value	*ret_printf = Builder.CreateCall(f_printf, printf_args.begin(), printf_args.end(), "");
  

  // retour de fonction
  Builder.CreateRet(res);
  
  // valide rapidement l'IR
  verifyFunction(*f);

  // ECRIT LE BITCODE
  std::vector< unsigned char >	theBuffer;
  BitstreamWriter	bt(theBuffer);
  WriteBitcodeToStream(TheModule, bt);
  cerr << "DUMP BIT CODE:" << endl;
  for (int i = 0; i < theBuffer.size(); ++i)
    {
      cerr << hex << (int)theBuffer[i] << " ";
    }
  cerr << endl;

  // EXECUTION -> GENERE CODE NATIF
  ExecutionEngine	*exec = EngineBuilder(TheModule).create();
  MyJITListener		jit_listen;
  exec->RegisterJITEventListener(&jit_listen);
  exec->DisableLazyCompilation();
  void	*pf = exec->recompileAndRelinkFunction(f);
  cerr << "fonction a :" << hex << pf << endl;
  typedef int (*fake_main)(int, const char**, const char**);
  fake_main m = (fake_main) pf;
  const char *arg[] = {"test", "12", 0};
  const char *arge[] = {"a=b", "c=d", 0};
  m(2, arg, arge);
  ///
  
  dup2(1, 2);///!!!! hack pour le makefile
  TheModule->dump();
}
