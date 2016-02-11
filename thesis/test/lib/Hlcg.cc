#include "Hlcg.hh"

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

//// FOR HLCG
#include "llvm/DerivedTypes.h"
#include "llvm/ValueSymbolTable.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/CodeGen/JITCodeEmitter.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/Linker.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/IRReader.h"
#include "llvm/Support/IRBuilder.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Bitcode/ReaderWriter.h" // pour ecrire l'IR
#include "llvm/Bitcode/BitstreamWriter.h" // pour ecrire l'IR
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"

// interface prive
#include "lib/ExecutionEngine/JIT/JIT.h"

// pour la fonction system() :(
#include <stdlib.h>
// pour dladdr
#include <dlfcn.h>

using namespace llvm;
using namespace std;
using namespace clang;
using namespace tool;

/*
Problem::Problem(const char *s) : sMessage(s) {}
Problem::~Problem() {} 
const char *Problem::what() const {return sMessage.c_str();}
*/

///////// RELOCATION

RelocInfo::RelocInfo(int caa, int car, int o, int i) : callAddrAbsolute(caa), callAddrRelat(car), offset(o), instSize(i) 
{
        Dl_info         info;
        dladdr((const void*)callAddrAbsolute, &info);
        cerr << "DLADDR:" << endl
                << "pathname:" << info.dli_fname << endl
                << "base:" << hex << (int)info.dli_fbase << endl
                << "symname:" << info.dli_sname << endl
                << "realaddr:" << hex << (int)info.dli_saddr << endl
                ;
        symbolName.assign(info.dli_sname);
}

///////// CREATE CODE GENERATOR LISTENER
//JITEventListener::~JITEventListener() {}

CodeGeneratorListener::CodeGeneratorListener(Hlcg &rInj)
: _inj(rInj), JITEventListener()
{
        std::cerr << "chargement du listener de JIT" << std::endl;
        _inj.addEventListener(this);
}

CodeGeneratorListener::~CodeGeneratorListener() {}

void	CodeGeneratorListener::setBaseFileName(std::string &input)
{
        if (_outfile.is_open())
        {_outfile.close();}
        _outFileStrInject = input.substr(0, input.find(".bc")) + ".inject.c";
        _outFileStrBc = _outFileStrInject.substr(0, _outFileStrInject.find(".c")) + ".bc";
        _outfile.open(_outFileStrInject.c_str());
}

static void dumpHex(uint8_t *buf, int start, int end)
{
        for (; start < end; ++start)
        {cerr << setw(2) << hex << (int)buf[start] << " ";}
}

void	CodeGeneratorListener::initFile()
{
        _outfile << "#include <stdlib.h>" << endl;
        _outfile << "#include <stdio.h>" << endl;
        _outfile << "#include <string.h>" << endl;
        _outfile << "#include <stdarg.h>" << endl;
        _outfile << "#include <sys/mman.h>" << endl;

#define HEREDOC(X)	#X
        _outfile <<
// Attention Some Magic HERE
        HEREDOC(
                        typedef struct {int index; int64_t value; int offset; int constSize;} ConstInfo;
                        typedef struct {int callAddrAbsolute; int callAddrRelat; int offset; int instSize;} jitRelocInfo;
                        void	*compilette(void *func_ptr, int func_size, jitRelocInfo *reloc, ConstInfo *cvalue, ...)
                        {
                                int align_page = ((func_size / 4096) + 1) * 4096;
                                void *zone_exec = mmap(0, align_page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
                                if (zone_exec == MAP_FAILED)
                                {
                                        perror("MAPPAGE EXEC MEMOIRE IMPOSSIBLE");
                                        exit(42);
                                }
                                printf("copie mem %p %p %d\n", zone_exec, func_ptr, func_size);
                                memcpy(zone_exec, func_ptr, func_size);
                                for (int i = 0; reloc[i].callAddrAbsolute; ++i)
                                {
                                        int     *reloc_addr = (int*)(((char*)zone_exec) + reloc[i].offset + 1);
                                        printf("reloc :%p %p %p\n", (void*)*reloc_addr, (void*)reloc[i].callAddrAbsolute, (void*)reloc[i].callAddrRelat);
                                        int reloc_value = (reloc[i].callAddrAbsolute - (int)zone_exec) - reloc[i].offset - reloc[i].instSize;
                                        printf("reloc value:%p\n", (void*)reloc_value);
                                        *reloc_addr = reloc_value;
                                }
                                va_list	list_arg;
                                va_start(list_arg, cvalue);
                                // we fetch from additionnal args the constant substitution
                                int idx = va_arg(list_arg, int);
                                int val;
                                while (idx != -1)
                                {
                                        val = va_arg(list_arg, int);
                                        // do substitution
                                        int	*subst = (int*)zone_exec;
                                        if (subst[cvalue[idx].offset] == cvalue[idx].value)
                                        {       subst[cvalue[idx].offset] = val;}
                                        else
                                        {
                                                printf("value %d\n", subst[cvalue[idx].offset]);
                                                printf("value2 %d\n", subst[cvalue[idx].offset+1]);
                                                printf("value3 %d\n", subst[cvalue[idx].offset+2]);
                                                exit(42);
                                        }
                                        // next
                                        idx = va_arg(list_arg, int);
                                }
                                va_end(list_arg);
                                mprotect(zone_exec, func_size, PROT_READ | PROT_EXEC);
                                return zone_exec;
                        }
        )// HEREDOC
        << endl;
#undef HEREDOC
}

void    CodeGeneratorListener::compileFile()
{
        _outfile.close();
        // finalize le code extrait
        string cmd = string("clang -I ../lib -I /usr/include/i386-linux-gnu -g -emit-llvm -c ") + _outFileStrInject + string(" -o ") + _outFileStrBc;
        cerr << "Create BC FILE:" << cmd << endl;
        if (system(cmd.c_str()) != 0)
        {
                cerr << "Erreur dans l'execution du sous-shell via system" << endl;
                exit(42);
        }
}

void    CodeGeneratorListener::writeReloc()
{
        map< string, list< RelocInfo > >::iterator	mitb, mite;
        mitb = _mapReloc.begin();
        mite = _mapReloc.end();
        cerr << "MAP RELOC ITERE" << endl;
        for (; mitb != mite; ++mitb)
        {
                cerr << "WRITE RELOC!! in " << endl;
                string	sFuncname = mitb->first;
                list< RelocInfo >::iterator		litb, lite;
                litb = mitb->second.begin();
                lite = mitb->second.end();
                // first pass:declare extern
                for (; litb != lite; ++litb)
                {       _outfile << "extern void " << litb->symbolName<< "();" << endl;}
                // second pass:write reloc table
                litb = mitb->second.begin();
                _outfile << "jitRelocInfo reloc_" << sFuncname << "[] = {" << endl;
                for (; litb != lite; ++litb)
                {
                        _outfile << "{"; 
                        _outfile << "(int)" << litb->symbolName << ", ";
                        _outfile << "0x" << hex << litb->callAddrRelat << ", ";
                        _outfile << dec << litb->offset << ", ";
                        _outfile << dec << litb->instSize << ", ";
                        _outfile << "}," << endl;
                }
                _outfile << "{0, 0, 0, 0}" << endl;
                _outfile << "};" << endl;
        }
}

void    CodeGeneratorListener::writeConst()
{
        map< string, list< ConstInfo > >::iterator	mitb, mite;
        mitb = _mapConst.begin();
        mite = _mapConst.end();
        for (; mitb != mite; ++mitb)
        {
                string	sFuncname = mitb->first;
                list< ConstInfo >::iterator		litb, lite;
                litb = mitb->second.begin();
                lite = mitb->second.end();

                _outfile << "ConstInfo const_" << sFuncname << "[] = {" << endl;
                for (int i = 0; litb != lite; ++i, ++litb)
                {
                        _outfile << dec << "{";
                        _outfile << i << ", ";
                        _outfile << litb->value << ", ";
                        _outfile << litb->offset << ", ";
                        _outfile << litb->constSize << ", ";
                        _outfile << "}," << endl;
                }
                _outfile << "{0, 0, 0}" << endl;
                _outfile << "};" << endl;
        }
}

void    CodeGeneratorListener::processDisasm(string &sFuncname, const MCInst &rInst, uint8_t *pBuf, 
                                                int instIdx, int instSize)
{
        for (int iop = 0; iop < rInst.getNumOperands(); ++iop)
        {
                const MCOperand &rop = rInst.getOperand(iop);
                if (rop.isImm())
                {
                        cerr << "isImm:" << dec << rInst.getOpcode() << endl;
                        // pour l'instant en dur
                        if (rInst.getOpcode() == 320)
                        {
                                cerr << "CALL! inside " << sFuncname << endl;
                                int callAddr = rop.getImm();
                                int absAddress = (int)tool::calcAbsoluteAddress(callAddr, (uint8_t*)pBuf, instIdx, instSize);
                                RelocInfo r(absAddress, callAddr, instIdx, instSize);
                                _mapReloc[sFuncname].push_back(r);
                        }
                        else
                        {
                                int64_t	imm = rop.getImm();
                                double	*int2double = (double*)&imm;
                                float	*int2float = (float*)&imm;
                                cerr << "OP:" << dec << rInst.getOpcode() << endl;
                                //cerr << "valid:" << rop.isValid() << endl;
                                //cerr << "reg:" << rop.isReg() << endl;
                                //cerr << "imm:" << rop.isImm() << endl;
                                //cerr << "expr:" << rop.isExpr() << endl;
                                //cerr << "CONST! 0x" << hex << imm 
                                //        << " or (int)" << dec << imm 
                                //        << " or (double)" << *int2double 
                                //        << " or (float)" << *int2float << endl;
                                ConstInfo c(imm, instIdx, instSize);
                                _mapConst[sFuncname].push_back(c);
                        }
                }
        }
        _inj.getMCInstPrinter().printInst(&rInst, errs(), "_inj");
        cerr << endl << "--------" << endl;
}

void    CodeGeneratorListener::writeCodeGenerator(string &sFuncName, ofstream &_outfile, uint8_t *pBuf, int nSize)
{
        cerr << "EMIT JIT CODE DE FONCTION :" << sFuncName << " at:" << hex << (int)(pBuf) << endl;
        _outfile << setiosflags(ios::right);
        _outfile << "int size_" << sFuncName << " = " << dec << nSize << ";" << endl;
        _outfile << "char jit_" << sFuncName << "[] = {";
        for (int i = 0; i < nSize; ++i)
        {
                _outfile << setw(2) << setfill('\0') << "0x" << setfill('0') << hex << (int)pBuf[i];
                if (i + 1 != nSize)
                {_outfile << ", ";}
        }
        _outfile << "};" << endl;
}

// appele lors de la compilation de la fonction dans l'unite de compile
void    CodeGeneratorListener::NotifyFunctionEmitted(const Function &F, void *Code, size_t Size, 
                                                        const EmittedFunctionDetails &Details)
{
        string	sFunc = F.getNameStr();
        writeCodeGenerator(sFunc, _outfile, (uint8_t*)Code, Size);
        BufferMemoryObject buf((const uint8_t*)Code, Size);
        uint64_t instSize;
        for (uint64_t idx = 0; idx < Size; idx += instSize)
        {
                MCInst		theInst;
                _inj.getMCDisassembler().getInstruction(theInst, instSize, buf, idx, nulls(), nulls());
                string sFuncName = F.getNameStr();
                processDisasm(sFuncName, theInst, (uint8_t*)Code, idx, instSize);
                if (instSize == 0)
                {instSize = 1;}
        }
        cerr << endl;
}

// lorsque le code est ecrase
void    CodeGeneratorListener::NotifyFreeingMachineCode(void *OldPtr)
{
        //cerr << "FREE JIT CODE :" << hex << OldPtr << endl;
}

////////// PP CALLBACKS
#include "clang/Lex/PPCallbacks.h"
class MyPPCallbacks : public PPCallbacks
{
public:
        MyPPCallbacks()
        {
                cerr << "Init PP Callback" << endl;
        }

        virtual ~MyPPCallbacks(){}

        virtual void InclusionDirective(
        	SourceLocation  	HashLoc,
		const Token &  	IncludeTok,
		StringRef  	FileName,
		bool  	IsAngled,
		const FileEntry *  	File,
		SourceLocation  	EndLoc,
		StringRef  	SearchPath,
		StringRef  	RelativePath 
        )
        {
                cerr << "Inc directive:" << FileName.str() << endl;
                cerr << "Search Path:" << SearchPath.str() << endl;
                cerr << "RelPath:" << RelativePath.str() << endl;
        }

        virtual void 	MacroDefined (const Token &MacroNameTok, const MacroInfo *MI)
        {
/*                if (MacroNameTok.isAnyIdentifier())
                {
                        const char *id = MacroNameTok.getRawIdentifierData();
                        if (id)
                        cerr << "Defined:" << id << endl;
                }
                else */
                        cerr << "Def:" << MacroNameTok.getName() << endl;
        }
};
//////////

///////// CFRONT
CFrontEnd::CFrontEnd()
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
        _CI->getTargetOpts().Triple = sys::getHostTriple();
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
/*
        clang::HeaderSearchOptions      headerSearchOptions;
        clang::HeaderSearch             headerSearch(_CI->getFileManager());
        clang::Preprocessor             *preprocessor = new clang::Preprocessor(
                                                _CI->getDiagnostics(),
                                                _CI->getLangOpts(),
                                                &_CI->getTarget(),
                                                _CI->getSourceManager(),
                                                headerSearch,
                                                *_CI);
        clang::PreprocessorOptions      preprocessorOptions;
        clang::FrontendOptions          frontendOptions;
        clang::InitializePreprocessor(
                *preprocessor,
                preprocessorOptions,
                headerSearchOptions,
                frontendOptions
        );
        _CI->setPreprocessor(preprocessor);
*/
        _CI->createPreprocessor();
        _CI->getPreprocessor().addPPCallbacks(new MyPPCallbacks());
        cerr << "predef:" << _CI->getPreprocessor().getPredefines() << endl;
        cerr << "END!" << endl;
}


int                             CFrontEnd::parseIt(llvm::raw_ostream &rCompileLog, StringRef rData, void *MainAdr)
{
        TextDiagnosticBuffer *diag_buf = static_cast<TextDiagnosticBuffer*>(_CI->getDiagnostics().getClient());
        diag_buf->FlushDiagnostics(_CI->getDiagnostics());
        rCompileLog << "diag_buf:" << diag_buf << "\n";
        // from memory not file
        MemoryBuffer    *mem_buf = MemoryBuffer::getMemBufferCopy(rData, __FUNCTION__);
        const char      *outfname = "__tmp__.c";
        std::ofstream   outfile;
        outfile.open(outfname);
        outfile << mem_buf->getBuffer().str();
        outfile.close();
        //_CI->getSourceManager().createMainFileIDForMemBuffer(mem_buf);
        rCompileLog << "mem_buf:" << mem_buf << "\n";
        // preproc?
        /*
        _CI->getPreprocessor().EnterMainSourceFile();
        clang::Token    tok;
        clang::Preprocessor     &rPP = _CI->getPreprocessor();
        do
        {
                rPP.Lex(tok);
                if (_CI->getDiagnostics().hasErrorOccurred())
                {       break;}
                rPP.DumpToken(tok);
                cerr << endl;
        }
        while (tok.isNot(tok::eof));
        */

        const FileEntry *fe = _CI->getFileManager().getFile(outfname);
        cerr << "file entry:" << fe << endl;
        cerr << "info:" << fe->getName() << endl;
        cerr << "BuiltInc:" << _CI->getHeaderSearchOpts().UseBuiltinIncludes << endl;
        cerr << "SysInc:" << _CI->getHeaderSearchOpts().UseStandardSystemIncludes << endl;
        _CI->getHeaderSearchOpts().ResourceDir = "/usr/local/lib/clang/3.0/";
        cerr << "Res:" << _CI->getHeaderSearchOpts().ResourceDir << endl;
        cerr << "From comp:" << CompilerInvocation::GetResourcesPath("./test", MainAdr) << endl;
        _CI->getHeaderSearchOpts().AddPath("/usr/include/", frontend::CSystem, false, false, false);
        _CI->getHeaderSearchOpts().AddPath("/usr/include/linux/", frontend::CSystem, false, false, false);
        _CI->getHeaderSearchOpts().AddPath("/usr/local/lib/clang/3.0/include/", frontend::CSystem, false, false, false);
        _CI->getHeaderSearchOpts().AddPath("/usr/include/i386-linux-gnu/", frontend::CSystem, false, false, false);
        _CI->getHeaderSearchOpts().AddPath("../lib/", frontend::CSystem, true, false, false);
        std::vector< HeaderSearchOptions::Entry >::iterator it = _CI->getHeaderSearchOpts().UserEntries.begin();
        std::vector< HeaderSearchOptions::Entry >::iterator ite = _CI->getHeaderSearchOpts().UserEntries.end();
        for (; it != ite; ++it)
        {
                cerr << "entry:" << it->Path << endl;
        }
        _CI->getHeaderSearchOpts().UseStandardSystemIncludes = 1;
        _CI->getSourceManager().createMainFileID(fe);
        /*
        diag_buf->BeginSourceFile(_CI->getLangOpts(), &_CI->getPreprocessor());
        {
                clang::Parser  parser(_CI->getPreprocessor(), _CI->getSema());
                parser.ParseTranslationUnit();
                //ParseAST(_CI->getPreprocessor(), _Cgen, _CI->getASTContext());
        }
        diag_buf->EndSourceFile();
        */
        
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
        //_CI->getFrontendOpts().ProgramAction = frontend::EmitLLVM;
        _CI->getFrontendOpts().ProgramAction = frontend::ASTPrint;
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

///////// INJECTOR
Hlcg::Hlcg() : pContext(0), pModule(0), pTarget(0), pAsmInfo(0), pPrint(0), pDisasm(0), pEngineBuild(0), pExec(0)
{
        // instancie ce qu'il faut pour une cible de compilation local
        InitializeNativeTarget();
        llvm::InitializeAllAsmPrinters();
        llvm::InitializeAllDisassemblers();

        // cree le contexte global
        pContext = &getGlobalContext();
        std::string Error;
        pTarget = TargetRegistry::lookupTarget(sys::getHostTriple(), Error);
        cerr << "Instancie une target:" << hex << pTarget << " Error:" << Error << endl;

        pAsmInfo = pTarget->createMCAsmInfo(sys::getHostTriple());
        cerr << "Instancie un asmInfo:" << hex << pAsmInfo << endl;
        
        dialect = pAsmInfo->getAssemblerDialect();
        cerr << "Get le dialect:" << dialect << endl;
        
        pSTI = pTarget->createMCSubtargetInfo(sys::getHostTriple(), sys::getHostCPUName(), "");
        cerr << "Get le subtargetInfo:" << pSTI << endl;
        
        pPrint = pTarget->createMCInstPrinter(dialect, *pAsmInfo, *pSTI);
        cerr << "Instancie un Instruction Printer:" << hex << pPrint << endl;

        pDisasm = pTarget->createMCDisassembler(*pSTI);
        cerr << "Instancie un Disassembler:" << hex << pDisasm << endl;
}
	
int     Hlcg::addEventListener(JITEventListener *pJitEvent)
{
        if (!pExec)
        {return 0;}
        pExec->RegisterJITEventListener(pJitEvent);
        return 1;
}

void    Hlcg::_resetStateForNewModule()
{
        if (pModule) {delete pModule; pModule = 0;}
        if (pExec) {delete pExec; pExec = 0;}
        if (pEngineBuild) {delete pEngineBuild; pEngineBuild = 0;}
}

void    Hlcg::loadIRFile(const std::string &sInput)
{
        SMDiagnostic	err;
        //_resetStateForNewModule();
        // charge le bc dans le context courant
        pModule = ParseIRFile(sInput, err, *pContext);
        if (pModule == 0)
        {
                cerr << "Can't load IR File:" << sInput << endl << err.getMessage() << endl;
                exit(42);
        }
        cerr << "Load Module ID:" << pModule->getModuleIdentifier() << endl;
        // cree un builder
        if (!pEngineBuild)
                pEngineBuild = new EngineBuilder(pModule);
        if (pEngineBuild == 0)
        {
                cerr << "Can't create Engine Builder" << endl;
                exit(42);
        }
        // moteur d'execution en JIT
        pEngineBuild->setEngineKind(EngineKind::JIT);
        pEngineBuild->setOptLevel(CodeGenOpt::Aggressive);
        if (!pExec)
                pExec = pEngineBuild->create();
        if (pExec == 0)
        {
                cerr << "Can't create Execution Engine" << endl;
                exit(42);
        }
        pExec->DisableLazyCompilation();
}

void    Hlcg::linkIRFile(const string &sInfile, const string &sOutfile)
{
        cerr << "LINK " << sInfile << " TO " << sOutfile << endl;
        // Link le fichier dans le module courant
        SMDiagnostic	err;
        Module	*injModule = ParseIRFile(sInfile, err, *pContext);
	
        string sErrLink;
        if (Linker::LinkModules(pModule, injModule, Linker::DestroySource, &sErrLink))
        {
                cerr << "Erreur de link:" << sErrLink << endl;
                exit(42);
        }

        //pModule->dump();
        // REWRITE the BC
        std::vector< unsigned char >	theBuffer;
        BitstreamWriter	bt(theBuffer);
        WriteBitcodeToStream(pModule, bt);
        std::ofstream	out(sOutfile.c_str(), ios::out | ios::binary);
        std::vector< unsigned char >::iterator vitb, vite;
        vitb = theBuffer.begin();
        vite = theBuffer.end();
        for (; vitb != vite; ++vitb)
                out << *vitb;
        out.close();
}

Hlcg::~Hlcg()
{
        if (pModule) delete pModule;
}

