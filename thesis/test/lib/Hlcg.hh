#ifndef __HLCG_HH__
#define __HLCG_HH__

// for portable integer/pointers
#include <stdint.h>

// only dependence of llvm
#include "llvm/Support/MemoryObject.h"
#include "llvm/ExecutionEngine/JITEventListener.h" // pour le JIT
#include "clang/Lex/PPCallbacks.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

// STL
#include <memory>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <list>
#include <map>

// class forward
namespace llvm
{
        class LLVMContext;
        class Module;
        class Target;
        class MCAsmInfo;
        class MCSubtargetInfo;
        class MCInstPrinter;
        class MCInst;
        class MCDisassembler;
        class EngineBuilder;
        class ExecutionEngine;
        class JITEventListener;
}

namespace clang
{
        class CompilerInstance;
        class CodeGenerator;
}

namespace tool
{
        
        /*
        #include <exception>
        class	Problem : std::exception
        {
        	std::string	sMessage;
        	public:
        		Problem(const char *) throw();
        		virtual ~Problem() throw();
        		virtual const char *what() const throw();
        };
        */
        
        
        
        static inline uint8_t*	calcAbsoluteAddress(int nCallAddr, uint8_t *pBeginCode, int nOffset, int nSizeInst)
        {
                return (uint8_t*)(nCallAddr + (int)pBeginCode + nOffset + nSizeInst);
        }
        
        static inline uint8_t*	calcRelativeAddress(uint8_t *pFunc, uint8_t *pBeginCode, int nOffset, int nSizeInst)
        {
        
        }
        
        // classe d'abstraction d'un buffer de code en memoire necessaire pour MCDisassembler
        class BufferMemoryObject : public llvm::MemoryObject 
        {
                private:
                        const uint8_t *Bytes;
                        uint64_t Length;
                public:
                        BufferMemoryObject(const uint8_t *bytes, uint64_t length): Bytes(bytes), Length(length) {}
                        uint64_t getBase() const {return 0;}
                        uint64_t getExtent() const {return Length;}
                        int readByte(uint64_t addr, uint8_t *byte) const
                        {
                                if (addr > getExtent())
                                        return -1;
                                *byte = Bytes[addr];
                                return 0;
                        }
        };
        
        // information de relocation des addresses relatives pour les calls
        struct	RelocInfo
        {
                int             callAddrAbsolute;
                int             callAddrRelat;
                int             offset;
                int             instSize;
                std::string     symbolName;
                RelocInfo(int caa, int car, int o, int i);
        };
        
        // position des constantes dans le flux
        struct	ConstInfo
        {
                int             idx;
                int64_t         value;
                int             offset;
                int             constSize;
                ConstInfo(int64_t v, int o, int s) : idx(0), value(v), offset(o), constSize(s) {}
        };
        
        // hook pour l'evenement de compilation de l'IR en natif
        class   Hlcg;
        class	CodeGeneratorListener : public llvm::JITEventListener
        {
                Hlcg                                            &_inj;
                std::string                                     _outFileStrInject;
                std::string                                     _outFileStrBc;
                std::ofstream                                   _outfile;	
                std::map< std::string, std::list<RelocInfo> >   _mapReloc;
                std::map< std::string, std::list<ConstInfo> >   _mapConst;
        
                public:
                // non virtual members
                                                                CodeGeneratorListener(Hlcg&);
                        // gestion du fichier bitcode de sortie
                        void                                    setBaseFileName(std::string &);
                        const std::string                       &getFileName() const {return _outFileStrBc;}
        
                // virtual members
                        virtual                                 ~CodeGeneratorListener();
        
                        // appelé par l'execution engine
                        virtual void                            NotifyFunctionEmitted(const llvm::Function &F, void *Code, 
                                                                                                size_t Size, 
        											const llvm::JITEvent_EmittedFunctionDetails &Details);  
                        virtual void                            NotifyFreeingMachineCode(void *OldPtr);
        
                        // creer le fichier de base avec les headers/fonctions par defaut
                        virtual void                            initFile();
                        // desassemble une instruction
                        virtual void                            processDisasm(std::string &, const llvm::MCInst &rInst, uint8_t* pBuf, int instIdx, int instSize);
                        // ecrit les codes generateurs pour la fonction
                        virtual void                            writeCodeGenerator(std::string &sFuncName, std::ofstream &_outfile, uint8_t *pBuf, int nSize);
                        // ecrit toutes les relocations calculés 
                        virtual void                            writeReloc();
                        // ecrit toutes les constantes
                        virtual void                            writeConst();
                        // compile le fichier de sortie en bitcode
                        virtual void				compileFile();
        };
        
        // classe pour la manipulation direct du C via la lib clang
        class   CFrontEnd
        {
                llvm::LLVMContext               *_Ctx;
                clang::CodeGenerator            *_Cgen;
                clang::CompilerInstance         *_CI;
//                clang::FileManager              *_FM;
                public:
                                                CFrontEnd();
                int                             parseIt(llvm::raw_ostream &rCompileLog, llvm::StringRef rData, void *MainAdr);
        };
        
        // classe basique de manipulation et d'injection de code via LLVM
        class	Hlcg
        {
                llvm::LLVMContext               *pContext;
                llvm::Module                    *pModule;
                const llvm::Target              *pTarget;
                const llvm::MCSubtargetInfo     *pSTI;
                llvm::MCAsmInfo	                *pAsmInfo;
                int                             dialect;
                llvm::MCInstPrinter             *pPrint;
                llvm::MCDisassembler            *pDisasm;
                llvm::EngineBuilder             *pEngineBuild;
                llvm::ExecutionEngine           *pExec;
        	
                public:
                        llvm::LLVMContext       &getContext() const {return *pContext;}
                        llvm::Module            &getModule() const {return *pModule;}
                        const llvm::Target      &getTarget() const {return *pTarget;}
                        llvm::MCDisassembler    &getMCDisassembler() const {return *pDisasm;}
                        llvm::MCInstPrinter     &getMCInstPrinter() const {return *pPrint;}
                        llvm::ExecutionEngine   &getEEngine() const {return *pExec;};
        
                public:
                // non virtual members
                                                Hlcg();
                        // charge un fichier bitcode
                        void                    loadIRFile(const std::string&);
                        // link un bitcode avec le module courant
                        void                    linkIRFile(const std::string&, const std::string&);
                        // ajoute une callback pour le JIT
                        int                     addEventListener(llvm::JITEventListener*);
                // virtual members
                        virtual 		~Hlcg();
                private:
                        // reinit les modules utilisés
                        void			_resetStateForNewModule();
        };
        
}

#endif
