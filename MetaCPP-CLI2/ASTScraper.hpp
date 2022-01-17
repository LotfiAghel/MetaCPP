#ifndef METACPP_ASTSCRAPER_HPP
#define METACPP_ASTSCRAPER_HPP

#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclTemplate.h>
#include "MetaCPP/Storage.hpp"
#include "MetaCPP/QualifiedName.hpp"
#include "MetaCPP/QualifiedType.hpp"

namespace metacpp {
	class ASTScraper {
	public:
        clang::SourceManager *SM;
		struct Configuration {
			std::string AnnotationRequired;
		};

		ASTScraper(Storage* storage, const Configuration& config);

		void ScrapeTranslationUnit(const clang::TranslationUnitDecl* tuDecl);

        void traverseChilds(const clang::DeclContext* ctxDecl, Type* parent);

        void handleNamedDecl(const clang::NamedDecl* namedDecl, Type* parent);
        void handleNameSpaceDecl(const clang::DeclContext* namedDecl, Type* parent);

        Type *ScrapeEnumDecl(const clang::EnumDecl* cxxRecordDecl, Type* parent);

        std::vector<std::string> split(std::string s,std::string delimiter);
        std::string getClearPath(std::string sr);
        Type* traverseCXXRecordDecl(const clang::CXXRecordDecl* cxxRecordDecl, Type* parent);
        Type* traverseCXXTemplateDecl(const clang::ClassTemplateDecl* cxxRecordDecl, Type* parent);
        Type* traverseClassTemplateSpecializationDecl(const clang::ClassTemplateSpecializationDecl* cxxRecordDecl, Type* parent);

        std::vector<TemplateArg> ResolveCXXRecordTemplate(const clang::CXXRecordDecl* cxxRecordDecl, QualifiedName& qualifiedName);
        Type* ScrapeType(const clang::CXXRecordDecl* cxxRecordDecl,const clang::Type* cType,QualifiedName defultQulifName=QualifiedName());


        void getFieldsOfType(const clang::CXXRecordDecl* , Type* parent);

        Field ScrapeFieldDecl(const clang::FieldDecl* fieldDecl, Type* parent);
		void ScrapeMethodDecl(const clang::CXXMethodDecl* cxxMethodDecl, Type* parent);


        void beforVisit(const clang::NamedDecl* namedDecl, Type* parent){}
        void afterVisit(const clang::NamedDecl* namedDecl, Type* parent);

		QualifiedType ResolveQualType(clang::QualType qualType);
        std::set<std::string> ScrapeAnnotations(const clang::Decl* decl);

		/* Utility */
		void MakeCanonical(clang::QualType& qualType);
		QualifiedName ResolveQualifiedName(std::string qualifiedName);
		void RemoveAll(std::string& source, const std::string& search);
		AccessSpecifier TransformAccess(const clang::AccessSpecifier as);

		bool IsReflected(const std::vector<std::string>& attrs);

		void SetContext(clang::ASTContext* context);

        std::string getDeclLocation(clang::SourceLocation Loc) const;

	private:
        clang::ASTContext* m_Context=nullptr;
		Configuration m_Config;
		Storage* m_Storage;
	};
}

#endif
