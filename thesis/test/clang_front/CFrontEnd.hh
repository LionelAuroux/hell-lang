// CFrontEnd, a simple wrapper around clang
// L.Auroux

#ifndef __CFRONTEND_HH__
#define __CFRONTEND_HH__

// for portable integer/pointers
#include <stdint.h>

// only dependence of clang
#include "clang/Lex/PPCallbacks.h"

// only dependence of llvm
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

// begin class forward
namespace llvm
{
        class LLVMContext;
}

namespace clang
{
        class CompilerInstance;
        class CodeGenerator;
}
// end class forward 

namespace tool
{
        // class for direct handling of chunk of C code using clang
        class   CFrontEnd
        {
                llvm::LLVMContext               *_Ctx;
                clang::CodeGenerator            *_Cgen;
                clang::CompilerInstance         *_CI;
                clang::PPCallbacks              *_PreprocHandler;

        public:
                                                CFrontEnd(clang::PPCallbacks *pPreproc);
                virtual                         ~CFrontEnd();
                int                             parseIt(llvm::raw_ostream &rCompileLog, llvm::StringRef rData, void *MainAdr);
        };
}

#endif
