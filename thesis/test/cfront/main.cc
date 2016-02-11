//
//	cfront - try to subsum clang
//	this file is also a tutorial for future student

// iostream
#include <iostream>

// LLVM Context
#include "llvm/LLVMContext.h"

// LLVM initialize phase
#include "llvm/Support/TargetSelect.h"

// LLVM Advanced Data Type for OwningPtr
#include "llvm/ADT/OwningPtr.h"

// for host triple
#include "llvm/Support/Host.h"

// for facade objects CompilerInstance/CompilerInvocation
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"

// for enum in IncludeDirGroup
#include "clang/Frontend/HeaderSearchOptions.h"

// for ExecuteCompilerInvocation
#include "clang/FrontendTool/Utils.h"

// for access FrontendOptions (action etc...)
#include "clang/Frontend/FrontendOptions.h"

// for client Diagnostic
#include "clang/Frontend/TextDiagnosticBuffer.h"

// for source manager
#include "clang/Basic/SourceManager.h"

// for target info
#include "clang/Basic/TargetInfo.h"

// for CodeGenerator
#include "llvm/Module.h"
#include "clang/CodeGen/ModuleBuilder.h"

// for AST
#include "clang/AST/ASTContext.h"

// for parser
#include "clang/Parse/ParseAST.h"

// for sema
#include "clang/Sema/Sema.h"

// for DeclContext
#include "clang/AST/DeclBase.h"

// add namespace in global scope
using namespace	std;


// just for debugging
bool    checkInstanceInvocation(clang::CompilerInvocation	*_CompInv, clang::CompilerInstance	*_CompInst)
{
        return (true
                        && ((&_CompInv->getAnalyzerOpts()) == (&_CompInst->getAnalyzerOpts()) 
                                || ({cerr << "Not same AnalyzerOpts" << endl; false;})
                           )
                        && ((&_CompInv->getCodeGenOpts()) == (&_CompInst->getCodeGenOpts()) 
                                || ({cerr << "Not same CodeGenOpts" << endl; false;})
                           )
                        && ((&_CompInv->getDependencyOutputOpts()) == (&_CompInst->getDependencyOutputOpts()) 
                                || ({cerr << "Not same OutputOpts" << endl; false;})
                           )
                        && ((&_CompInv->getDiagnosticOpts()) == (&_CompInst->getDiagnosticOpts()) 
                                || ({cerr << "Not same DiagnosticOpts" << endl; false;})
                           )
                        && ((&_CompInv->getFileSystemOpts()) == (&_CompInst->getFileSystemOpts()) 
                                || ({cerr << "Not same FileSystemOpts" << endl; false;})
                           )
                        && ((&_CompInv->getFrontendOpts()) == (&_CompInst->getFrontendOpts()) 
                                || ({cerr << "Not same FrontenOpts" << endl; false;})
                           )
                        && ((&_CompInv->getHeaderSearchOpts()) == (&_CompInst->getHeaderSearchOpts())
                                || ({cerr << "Not same HeaderSearchOpts" << endl; false;})
                           )
                        && ((&_CompInv->getLangOpts()) == (&_CompInst->getLangOpts())
                                        || ({cerr << "Not same LangOpts" << endl; false;})
                           )
                        && ((&_CompInv->getPreprocessorOpts()) == (&_CompInst->getPreprocessorOpts())
                                        || ({cerr << "Not same PreprocessorOpts" << endl; false;})
                           )
                        && ((&_CompInv->getPreprocessorOutputOpts()) == (&_CompInst->getPreprocessorOutputOpts())
                                        || ({cerr << "Not same PreprocessorOutputOpts" << endl; false;})
                           )
                        && ((&_CompInv->getTargetOpts()) == (&_CompInst->getTargetOpts())
                                        || ({cerr << "Not same TargetOpts" << endl; false;})
                           )
                        );
}

// -- 
int     main(int ac, char **av, char **env)
{
        // Initialize LLVM targets first
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmPrinters();
        llvm::InitializeAllAsmParsers();

        // Initialize the 2 big facade object for CLANG
        llvm::OwningPtr< clang::CompilerInvocation >    _CompInv(new clang::CompilerInvocation());
        llvm::OwningPtr< clang::CompilerInstance >	_CompInst(new clang::CompilerInstance());

        // set language
        _CompInv->setLangDefaults(clang::IK_C, clang::LangStandard::lang_c99);

        // configure the instance with the invocation
        _CompInst->setInvocation(_CompInv.get());
        cerr << "_CompInv:" << _CompInv.get() << endl;
        cerr << "_CompInst:" << _CompInst.get() << endl;
        cerr << "_CompInv::CodeGenOpts:" << &_CompInv->getCodeGenOpts() << endl;
        cerr << "_CompInst::CodeGenOpts:" << &_CompInst->getCodeGenOpts() << endl;

        // now all object in instance come from invocation
        if (!checkInstanceInvocation(_CompInv.get(), _CompInst.get()))
        {
                cerr << "New object are instancied in the CompilerInstance" << endl;
                abort();
        }

        // now the invocation object is own by the instance
        _CompInv.take();

        // configure default path
        _CompInst->getFrontendOpts().DisableFree = true;
        _CompInst->getHeaderSearchOpts().ResourceDir = "/usr/local/lib/clang/3.0/";
        _CompInst->getHeaderSearchOpts().AddPath("/usr/local/lib/clang/3.0/include/", 
                                                clang::frontend::CSystem, false, false, false);
        _CompInst->getHeaderSearchOpts().AddPath("/usr/include/", 
                                                clang::frontend::CSystem, false, false, false);
        _CompInst->getHeaderSearchOpts().AddPath("/usr/include/i386-linux-gnu/", 
                                                clang::frontend::CSystem, false, false, false);
        _CompInst->getHeaderSearchOpts().AddPath("/usr/local/include/", 
                                                clang::frontend::CSystem, false, false, false);

        ///// create all
        
        // create diagnostic objet for notice/warning/error outputting
        // require: need to initialize targetOpts before
        _CompInst->getTargetOpts().Triple = llvm::sys::getHostTriple();
        _CompInst->createDiagnostics(0, 0);

        // initialize diagnostic
        _CompInst->getDiagnostics().setClient(new clang::TextDiagnosticBuffer);
        clang::TextDiagnosticBuffer *diag_buf = 
                        static_cast<clang::TextDiagnosticBuffer*>(_CompInst->getDiagnostics().getClient());
        diag_buf->FlushDiagnostics(_CompInst->getDiagnostics());

        // create file and source manager
        _CompInst->createFileManager();
        _CompInst->createSourceManager(_CompInst->getFileManager());

        // have we a target configured
        _CompInst->setTarget(clang::TargetInfo::CreateTargetInfo(_CompInst->getDiagnostics(), _CompInst->getTargetOpts()));
        if (!_CompInst->hasTarget())
        {
                cerr << "no target configured...force local target" << endl;
                abort();
        }

        // create a preprocessor
        _CompInst->createPreprocessor();
        if (!_CompInst->hasPreprocessor())
        {
                cerr << "no preprocessor" << endl;
                abort();
        }

        // configure backend
        clang::CodeGenerator   *_Cgen = clang::CreateLLVMCodeGen(_CompInst->getDiagnostics(), "__cfront__", 
                        _CompInst->getCodeGenOpts(), llvm::getGlobalContext());
        // AST Consumer on the code generator
        _CompInst->setASTConsumer(_Cgen);
        if (!_CompInst->hasASTConsumer())
        {
                cerr << "no AST consumer" << endl;
                abort();
        }

        // AST Context
        _CompInst->createASTContext();
        if (!_CompInst->hasASTContext())
        {
                cerr << "no AST context" << endl;
                abort();
        }

        // Sema create with preprocessor and codegenerator
        _CompInst->createSema(clang::TU_Complete, NULL);
        if (!_CompInst->hasSema())
        {
                cerr << "no Sema" << endl;
                abort();
        }

        /////// INPUT PART
        // set input File
        _CompInst->InitializeSourceManager("test.c");

        // check that the main file ID is correct
        const llvm::MemoryBuffer        *main_buf = _CompInst->getSourceManager()
                .getBuffer(_CompInst->getSourceManager().getMainFileID());
        cerr << "Main File Buf:" << main_buf << endl;

        // show content
        cerr << "-----begin----" << endl;
        for (const char *c = main_buf->getBufferStart(); c != main_buf->getBufferEnd(); ++c)
        {cerr << *c;}
        cerr << "-----end----" << endl;
        ////////

        ////////// configure frontend
        // not necessary
        //_CompInst->getFrontendOpts().ProgramAction = clang::frontend::EmitLLVM;

        // call the compiler
        clang::ParseAST(_CompInst->getSema(), true);

        // print info after compiler invocation
        cerr << "_CompInst::CodeGenOpts:" << &_CompInst->getCodeGenOpts() << endl;

        // Show diagnostic
        clang::TextDiagnosticBuffer::const_iterator diag_it;
        for (diag_it = diag_buf->note_begin(); diag_it != diag_buf->note_end(); ++diag_it)
                cerr << "Notice:" << diag_it->second << "\n";
        for (diag_it = diag_buf->warn_begin(); diag_it != diag_buf->warn_end(); ++diag_it)
                cerr << "Warning:" << diag_it->second << "\n";
        int numerr = 0;
        for (diag_it = diag_buf->err_begin(); diag_it != diag_buf->err_end(); ++diag_it, ++numerr)
                cerr << "Error:" << diag_it->second << "\n";

        // print statistic from parsing
        const clang::ASTContext        &astRead = _CompInst->getASTContext();
        astRead.PrintStats();
        astRead.Idents.PrintStats();

        // lookup inside
        _Cgen->GetModule()->print(llvm::errs(), NULL);

        // get DeclContext inside the sema
        clang::DeclContext *decl = _CompInst->getSema().getFunctionLevelDeclContext();
        cerr << "DUMP DECLARATION via prettyPrint:" << endl;
        decl->dumpDeclContext();

        // next step, AS, link, elf write
}
