// simple test

#include "CFrontEnd.hh"

// FOR MyPPCallbacks
#include "clang/Basic/SourceLocation.h"
#include "clang/Lex/Token.h"

using namespace std;
using namespace clang;

// A local class for Preprocessing Callbacks
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
                if (MacroNameTok.isAnyIdentifier())
                {
                        const char *id = MacroNameTok.getRawIdentifierData();
                        if (id)
                        cerr << "Defined:" << id << endl;
                }
                else
                        cerr << "Def:" << MacroNameTok.getName() << endl;
        }
};

int     main()
{
        tool::CFrontEnd         cfront(new MyPPCallbacks);
        
        // calling clang stuff on a string
        cfront.parseIt(llvm::errs(), "#include <stdio.h>\nint main(){printf(\"TEST\\n\");}", (void*)&main);
}
