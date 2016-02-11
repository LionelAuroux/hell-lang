// CFrontEnd, a simple wrapper around clang
// L.Auroux

#include "CFrontEnd.hh"

//// FOR CFRONTEND
#include "clang/Driver/Arg.h"
#include "clang/Driver/ArgList.h"
#include "clang/Driver/CC1Options.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/OptTable.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Frontend/Utils.h"
#include "clang/Frontend/PreprocessorOptions.h"
#include "clang/Frontend/FrontendOptions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/TextDiagnosticBuffer.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/HeaderSearchOptions.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/FrontendTool/Utils.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Parse/Parser.h"

// LLVM PART
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/MemoryObject.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"

using namespace std;
using namespace clang;
using namespace tool;

///////// CFRONT
CFrontEnd::CFrontEnd(clang::PPCallbacks *pPreproc) : _PreprocHandler(pPreproc)
{
        llvm::InitializeAllTargets();
        llvm::InitializeAllAsmPrinters();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllDisassemblers();

        clang::CompilerInvocation       *cinv = new clang::CompilerInvocation();
        cinv->setLangDefaults(clang::IK_C, LangStandard::lang_c99);
        _CI = new clang::CompilerInstance();
        _CI->setInvocation(cinv);
        cerr << "New CI:" << _CI << endl;

        _CI->getCodeGenOpts().EmitDeclMetadata = true;
        _CI->getCodeGenOpts().InstrumentFunctions = true;
        _CI->getTargetOpts().Triple = llvm::sys::getHostTriple();
        _CI->createDiagnostics(0, 0);
        _CI->setTarget(TargetInfo::CreateTargetInfo(_CI->getDiagnostics(), _CI->getTargetOpts()));
        if (!_CI->hasTarget())
        {       
                cerr << "Not initialized Target" << endl;
                abort();
        }
        _CI->getDiagnostics().setClient(new clang::TextDiagnosticBuffer);
        _CI->createFileManager();
        _CI->createSourceManager(_CI->getFileManager());
        _CI->createPreprocessor();
        _CI->getPreprocessor().addPPCallbacks(_PreprocHandler);
        cerr << "predef:" << _CI->getPreprocessor().getPredefines() << endl;
        cerr << "END!" << endl;
}

CFrontEnd::~CFrontEnd()
{}

int                             CFrontEnd::parseIt(llvm::raw_ostream &rCompileLog, StringRef rData, void *MainAdr)
{
        TextDiagnosticBuffer *diag_buf = static_cast<TextDiagnosticBuffer*>(_CI->getDiagnostics().getClient());
        diag_buf->FlushDiagnostics(_CI->getDiagnostics());
        rCompileLog << "diag_buf:" << diag_buf << "\n";
        // from memory not file
        llvm::MemoryBuffer    *mem_buf = llvm::MemoryBuffer::getMemBufferCopy(rData, __FUNCTION__);
        const char      *outfname = "__tmp__.c";
        std::ofstream   outfile;
        outfile.open(outfname);
        outfile << mem_buf->getBuffer().str();
        outfile.close();
        //_CI->getSourceManager().createMainFileIDForMemBuffer(mem_buf);
        rCompileLog << "mem_buf:" << mem_buf << "\n";

        const FileEntry *fe = _CI->getFileManager().getFile(outfname);
        cerr << "file entry:" << fe << endl;
        cerr << "info:" << fe->getName() << endl;
        cerr << "BuiltInc:" << _CI->getHeaderSearchOpts().UseBuiltinIncludes << endl;
        cerr << "SysInc:" << _CI->getHeaderSearchOpts().UseStandardSystemIncludes << endl;
        _CI->getHeaderSearchOpts().ResourceDir = "/usr/local/lib/clang/3.0/";
        cerr << "Res:" << _CI->getHeaderSearchOpts().ResourceDir << endl;
        cerr << "From comp:" << CompilerInvocation::GetResourcesPath("./test", MainAdr) << endl;
        // TODO: Use standard sys include
        // cause UseStandardSystemIncludes don't work
        _CI->getHeaderSearchOpts().AddPath("/usr/include/", frontend::CSystem, false, false, false);
        _CI->getHeaderSearchOpts().AddPath("/usr/include/linux/", frontend::CSystem, false, false, false);
        _CI->getHeaderSearchOpts().AddPath("/usr/local/lib/clang/3.0/include/", frontend::CSystem, false, false, false);
        _CI->getHeaderSearchOpts().AddPath("/usr/include/i386-linux-gnu/", frontend::CSystem, false, false, false);
        _CI->getHeaderSearchOpts().AddPath("/usr/include/x86_64-linux-gnu/", frontend::CSystem, false, false, false);
        _CI->getHeaderSearchOpts().AddPath("../lib/", frontend::CSystem, true, false, false);
        /////
        std::vector< HeaderSearchOptions::Entry >::iterator it = _CI->getHeaderSearchOpts().UserEntries.begin();
        std::vector< HeaderSearchOptions::Entry >::iterator ite = _CI->getHeaderSearchOpts().UserEntries.end();
        for (; it != ite; ++it)
        {
                cerr << "entry:" << it->Path << endl;
        }
        _CI->getHeaderSearchOpts().UseStandardSystemIncludes = 1;
        _CI->getSourceManager().createMainFileID(fe);
        
        std::pair< InputKind, std::string > ifile(IK_C, outfname);
        _CI->getFrontendOpts().Inputs.push_back(ifile);
        
        
        _CI->getFrontendOpts().ProgramAction = frontend::PrintPreprocessedInput;
        if (clang::ExecuteCompilerInvocation(_CI))
        {        rCompileLog << "parse done!" << "\n";}

        ASTConsumer     *astConsum = new ASTConsumer();//TODO: inherit and put logic into
        _CI->setASTConsumer(astConsum);
        _CI->createASTContext();
        _CI->createSema(clang::TU_Complete, NULL);
        _Ctx = new llvm::LLVMContext();
        _Cgen = clang::CreateLLVMCodeGen(_CI->getDiagnostics(), "__cfront__", _CI->getCodeGenOpts(), *_Ctx);


        ///// NEXT

        //_CI->getFrontendOpts().ProgramAction = frontend::EmitLLVM;
        //_CI->getFrontendOpts().ProgramAction = frontend::ASTPrint;
        cerr << "AST Ctx:" << &_CI->getASTContext() << endl;
        if (clang::ExecuteCompilerInvocation(_CI))
        {        rCompileLog << "compile done!" << "\n";}
        // Show diagnostic
        TextDiagnosticBuffer::const_iterator diag_it;
        for (diag_it = diag_buf->note_begin(); diag_it != diag_buf->note_end(); ++diag_it)
                rCompileLog << "Notice:" << diag_it->second << "\n";
        for (diag_it = diag_buf->warn_begin(); diag_it != diag_buf->warn_end(); ++diag_it)
                rCompileLog << "Warning:" << diag_it->second << "\n";
        int numerr = 0;
        for (diag_it = diag_buf->err_begin(); diag_it != diag_buf->err_end(); ++diag_it, ++numerr)
                rCompileLog << "Error:" << diag_it->second << "\n";
        if (!numerr)
        {        _Cgen->GetModule()->print(rCompileLog, NULL);}
        _CI->getASTContext().Idents.PrintStats();
        return numerr;
}
