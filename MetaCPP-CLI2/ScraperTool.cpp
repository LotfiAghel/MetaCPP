#include "ScraperTool.hpp"

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>
#include "clang/AST/RecursiveASTVisitor.h"
#include "ASTScraper.hpp"
#include "clang/Tooling/JSONCompilationDatabase.h"

#include "llvm/Support/raw_ostream.h"



#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include <string>
#include <sstream>
#include <iostream>
using namespace std;

namespace metacpp {

using namespace clang;
using namespace clang::ast_matchers;




    ScraperTool::ScraperTool(std::string dir,std::string source, std::vector<std::string> flags)
	{
#if _DEBUG
		// For now
		flags.push_back("-D_DEBUG");
#endif


        m_CompilationDatabase = new clang::tooling::FixedCompilationDatabase(dir, flags);

        m_ClangTool = new clang::tooling::ClangTool(*m_CompilationDatabase, source);
	}

	ScraperTool::~ScraperTool() {
        //delete m_CompilationDatabase;
		delete m_ClangTool;
	}

    /*class DeclVisitor : public clang::RecursiveASTVisitor<DeclVisitor> {
      clang::SourceManager *SourceManager;

    public:
      DeclVisitor(clang::SourceManager *SourceManager)
          : SourceManager(SourceManager) {}

      bool VisitNamedDecl(clang::NamedDecl *NamedDecl) {

         // clang::dyn_cast<clang::EnumDecl>(NamedDecl)
        cout<< "Found " << NamedDecl->getQualifiedNameAsString() <<" : "<<NamedDecl->getDeclKindName()<< " at "
                     << getDeclLocation(NamedDecl->getLocation()) << "\n";
        return true;
      }

    private:
      std::string getDeclLocation(clang::SourceLocation Loc) const {
        std::ostringstream OSS;
        OSS << SourceManager->getFilename(Loc).str() << ":"
            << SourceManager->getSpellingLineNumber(Loc) << ":"
            << SourceManager->getSpellingColumnNumber(Loc);
        return OSS.str();
      }
    };/**/
   class MyASTConsumer : public clang::ASTConsumer {
        //DeclVisitor Visitor;
    public:
        MyASTConsumer(ASTScraper* scraper) : scraper(scraper) /*,Visitor(scraper->SM)*/{};

        void HandleTranslationUnit(clang::ASTContext& context) {

            //Visitor.TraverseDecl(context.getTranslationUnitDecl());
            scraper->traverseChilds(context.getTranslationUnitDecl(),0);

        }

    private:

        ASTScraper* scraper;


    };

    // Action
    class MyASTAction : public clang::ASTFrontendAction {
    public:
        MyASTAction(ASTScraper* scraper) : scraper(scraper) {}

        std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& Compiler, llvm::StringRef InFile) override {
            scraper->SetContext(&Compiler.getASTContext());
            scraper->SM=&Compiler.getSourceManager();
            return std::unique_ptr<clang::ASTConsumer>(new MyASTConsumer(scraper));
        };

    private:
        ASTScraper* scraper;
    };

    // Factory
    class FrontendActionFactory : public clang::tooling::FrontendActionFactory {
    public:
        FrontendActionFactory(ASTScraper* scraper) : scraper(scraper) {}
        std::unique_ptr<clang::FrontendAction> create() override {
            return std::make_unique<MyASTAction>(scraper);
        }
    private:
        ASTScraper* scraper;
    };


	void ScraperTool::Run(ASTScraper* scraper)
	{
		// Consumer
        auto scraperAction = std::unique_ptr<FrontendActionFactory>(new FrontendActionFactory(scraper));

        m_ClangTool->run(scraperAction.get());
    }


}
