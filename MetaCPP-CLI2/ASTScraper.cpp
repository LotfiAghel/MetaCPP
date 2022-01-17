#include "ASTScraper.hpp"

#include <iostream>

#include <clang/AST/RecordLayout.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/VTableBuilder.h>
#include <clang/AST/Decl.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/StaticAnalyzer/Checkers/SValExplainer.h>
#include "MetaCPP/Type.hpp"
#include <sstream>
using namespace std;
namespace metacpp {
	ASTScraper::ASTScraper(Storage* storage, const Configuration& config)
		: m_Config(config), m_Storage(storage)
	{
	}


    void ASTScraper::traverseChilds(const clang::DeclContext* ctxDecl, Type* parent)
	{
		for (clang::DeclContext::decl_iterator it = ctxDecl->decls_begin(); it != ctxDecl->decls_end(); ++it)
		{
			const clang::Decl* decl = *it;
			if (decl->isInvalidDecl())
				continue;

			auto namedDecl = clang::dyn_cast<clang::NamedDecl>(decl);
			if (namedDecl)
                handleNamedDecl(namedDecl, parent);
		}
	}

    void ASTScraper::handleNamedDecl(const clang::NamedDecl* namedDecl, Type* parent)
	{



		clang::Decl::Kind kind = namedDecl->getKind();
        std::cout << "got to handle " << namedDecl->getDeclKindName() << std::endl;
		switch (kind) {
        case clang::Decl::Kind::ClassTemplate:{
            std::string qualifiedName = namedDecl->getQualifiedNameAsString();
            std::cout << "Unhandled! ClassTemplate " << namedDecl->getDeclKindName() << ": " << qualifiedName << std::endl;
            std::cout<<std::endl;
            traverseCXXTemplateDecl(clang::dyn_cast<clang::ClassTemplateDecl>(namedDecl), parent);
            break;

        }
        case clang::Decl::Kind::Typedef:{
            break;
        }/**/
        case clang::Decl::Kind::CXXConstructor:{
            break;
        }
        case clang::Decl::Kind::CXXDestructor:{
            break;
        }
        case clang::Decl::Kind::CXXMethod:{
            break;
        }
		default:
		{
			// Unhandled
            std::string qualifiedName = namedDecl->getQualifiedNameAsString();
            std::cout << "Unhandled! " << namedDecl->getDeclKindName() << ": " << qualifiedName << std::endl;
            std::cout<<std::endl;
			break;
		}
        case clang::Decl::Kind::Enum:{
            std::string qualifiedName = namedDecl->getQualifiedNameAsString();
            std::cout << "handle enum " << namedDecl->getDeclKindName() << ": " << qualifiedName << std::endl;
            ScrapeEnumDecl(clang::dyn_cast<clang::EnumDecl>(namedDecl), parent);
            break;
        }
		case clang::Decl::Kind::Namespace:
            //handleNameSpaceDecl(namedDecl->castToDeclContext(namedDecl), parent);
            traverseChilds(namedDecl->castToDeclContext(namedDecl), parent);
			break;
        case clang::Decl::Kind::CXXRecord:{


            traverseCXXRecordDecl(clang::dyn_cast<clang::CXXRecordDecl>(namedDecl), parent);
			break;
        }
        case clang::Decl::Kind::ClassTemplateSpecialization:{
            std::string qualifiedName = namedDecl->getQualifiedNameAsString();
            std::cout << "handled " << namedDecl->getDeclKindName() << ": " << qualifiedName << std::endl;
            traverseClassTemplateSpecializationDecl(clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(namedDecl), parent);
            std::cout<<std::endl;
            break;
        }
		case clang::Decl::Kind::Field:
            //ScrapeFieldDecl(clang::dyn_cast<clang::FieldDecl>(namedDecl), parent);
			break;
		}


    }

    void ASTScraper::handleNameSpaceDecl(const clang::DeclContext *namedDecl, Type *parent){
       // afterVisit(namedDecl,parent);
    }
    Type* ASTScraper::ScrapeEnumDecl(const clang::EnumDecl* cxxRecordDecl, Type* parent)
    {


        const clang::Type* cType = cxxRecordDecl->getTypeForDecl();
        auto qualifiedName =ResolveQualifiedName(cxxRecordDecl->getQualifiedNameAsString());



        metacpp::Type* type = ScrapeType(nullptr,cType,qualifiedName);

        if(type==nullptr)
            return nullptr;
        std::cout<<"def ENUM "<<type->GetQualifiedName().GetName()<<std::endl;
        if (type) {
            const clang::CXXRecordDecl* typeCxxRecordDecl = cType->getAsCXXRecordDecl();

            type->SetAccess(TransformAccess(cxxRecordDecl->getAccess()));
            type->SetHasDefaultConstructor(false);
        }


        for (auto it = cxxRecordDecl->enumerator_begin(); it != cxxRecordDecl->enumerator_end(); it++) {
            clang::EnumConstantDecl* method = *it;
            auto str=method->getNameAsString();
            auto vlue=method->getInitVal().getExtValue();
            type->m_enum_fields.push_back({vlue,str});
            if (method != 0) {
                std::cout<<"enum field "<<str<<" > "<<vlue<<std::endl;
            }
        }
        return type;
    }

    std::vector<string> ASTScraper::split(string s, string delimiter){
        std::vector<std::string> res;
        size_t pos = 0;
        std::string token;
        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            res.push_back(token);
            s.erase(0, pos + delimiter.length());
        }
        res.push_back(s);
        return res;

    }

    string ASTScraper::getClearPath(string sr){
        std::vector<std::string> res0=split(sr,"/"),res;

        for(const std::string &f:res0)
            if(f=="..")
                res.pop_back();
            else
                res.push_back(f);

        std::ostringstream OSS;
        for(const auto &f:res)
            OSS << f << "/";
        auto resa=OSS.str();
        return resa.substr(0,resa.size()-1);
    }


    Type* ASTScraper::traverseCXXRecordDecl(const clang::CXXRecordDecl* cxxRecordDecl, Type* parent)
    {

        beforVisit(cxxRecordDecl,parent);
        if (cxxRecordDecl->isAnonymousStructOrUnion()) {
            afterVisit(cxxRecordDecl, parent);
            return 0;
        }




        const clang::Type* cType = cxxRecordDecl->getTypeForDecl();
        metacpp::Type* type = ScrapeType(cxxRecordDecl,cType,QualifiedName());



		if (type) {

			const clang::CXXRecordDecl* typeCxxRecordDecl = cType->getAsCXXRecordDecl();

			type->SetAccess(TransformAccess(cxxRecordDecl->getAccess()));
            type->isAbstract=typeCxxRecordDecl->isAbstract();
			type->SetHasDefaultConstructor(!typeCxxRecordDecl->hasUserProvidedDefaultConstructor() && typeCxxRecordDecl->needsImplicitDefaultConstructor());


			// methods

		}
        afterVisit(cxxRecordDecl, type);
		return type;
    }
    Type* ASTScraper::traverseCXXTemplateDecl(const clang::ClassTemplateDecl* cxxRecordDecl, Type* type)
    {

        beforVisit(cxxRecordDecl,type);

        //afterVisit(cxxRecordDecl, type); Crash mikone
        return nullptr;
    }

    Type *ASTScraper::traverseClassTemplateSpecializationDecl(const clang::ClassTemplateSpecializationDecl *templateDecl, Type *parent){
        QualifiedName qualifiedName;
        std::string typeName = templateDecl->getQualifiedNameAsString();
        std::vector<TemplateArg> templateArgs;

        if (templateDecl) {
            typeName += "<";

            const clang::TemplateArgumentList& args = templateDecl->getTemplateArgs();

            for (int i = 0; i < (int)args.size(); i++) {
                const clang::TemplateArgument& arg = args[i];
                if (i)
                    typeName += ",";

                TemplateArg qualifiedType;
                switch (arg.getKind()) {
                    case clang::TemplateArgument::Type:
                        //m_Storage->GetTypeID(arg.getAsType());
                        qualifiedType = TemplateArg(ResolveQualType(arg.getAsType()));
                        break;
                    case clang::TemplateArgument::Integral:
                        //m_Storage->GetTypeID(arg.getAsType());
                        qualifiedType = TemplateArg(arg.getAsIntegral().getExtValue());
                        break;
                    default:
                        std::cout << "Unsupported template argument!" << std::endl;
                        break;
                }
                if(qualifiedType.kind==TemplateArg::Kind::TypeName){
                    if (qualifiedType.type.GetTypeID() != 0)
                        typeName += qualifiedType.type.GetQualifiedName(m_Storage);
                    else
                        typeName += "INVALID";
                }else if(qualifiedType.kind==TemplateArg::Kind::Integral){
                    typeName += qualifiedType.integerValue;
                }

                templateArgs.push_back(qualifiedType);
            }

            typeName += ">";
        }

        qualifiedName = ResolveQualifiedName(typeName);
        qualifiedName.templateArgs=templateArgs;

        if(qualifiedName.m_Name=="MachinGunTank")
            cout<<"GObject"<<endl;

        afterVisit(templateDecl,parent);

        //return templateArgs;

    }

    std::vector<TemplateArg> ASTScraper::ResolveCXXRecordTemplate(const clang::CXXRecordDecl* cxxRecordDecl, QualifiedName& qualifiedName)
	{
		std::string typeName = cxxRecordDecl->getQualifiedNameAsString();
        std::vector<TemplateArg> templateArgs;

		// Template arguments
		auto templateDecl = llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(cxxRecordDecl);
		if (templateDecl) {
			typeName += "<";

			const clang::TemplateArgumentList& args = templateDecl->getTemplateArgs();

			for (int i = 0; i < (int)args.size(); i++) {
				const clang::TemplateArgument& arg = args[i];
				if (i)
					typeName += ",";

                TemplateArg qualifiedType;
				switch (arg.getKind()) {
					case clang::TemplateArgument::Type:
                        //m_Storage->GetTypeID(arg.getAsType());
                        qualifiedType = TemplateArg(ResolveQualType(arg.getAsType()));
                        break;
                    case clang::TemplateArgument::Integral:
                        //m_Storage->GetTypeID(arg.getAsType());
                        qualifiedType = TemplateArg(arg.getAsIntegral().getExtValue());
                        break;
					default:
						std::cout << "Unsupported template argument!" << std::endl;
						break;
				}
                if(qualifiedType.kind==TemplateArg::Kind::TypeName){
                    if (qualifiedType.type.GetTypeID() != 0)
                        typeName += qualifiedType.type.GetQualifiedName(m_Storage);
                    else
                        typeName += "INVALID";
                }else if(qualifiedType.kind==TemplateArg::Kind::Integral){
                    typeName += "INT__VLAUE";
                }

				templateArgs.push_back(qualifiedType);
			}

			typeName += ">";
            if(typeName.find("GObject")>-1)
                cout<<"GObject"<<endl;
		}

		qualifiedName = ResolveQualifiedName(typeName);
        qualifiedName.templateArgs=templateArgs;

		return templateArgs;
	}

    Type* ASTScraper::ScrapeType(const clang::CXXRecordDecl* cxxRecordDecl,const clang::Type* cType,QualifiedName defultQulifName)
    {




        QualifiedName qualifiedName;
        metacpp::TypeKind kind;
        std::vector<TemplateArg> templateArgs;

        switch (cType->getTypeClass()) {
        case clang::Type::TypeClass::Builtin:
        {
            static clang::LangOptions lang_opts;
            static clang::PrintingPolicy printing_policy(lang_opts);

            auto builtin = cType->getAs<clang::BuiltinType>();
            std::string name = builtin->getName(printing_policy);
            if (name == "_Bool")
                name = "bool";
            qualifiedName = QualifiedName({}, name);
            kind = metacpp::TypeKind::PRIMITIVE;
            break;
        }
        case clang::Type::TypeClass::Record:
        {
            auto cxxRecordDecl = cType->getAsCXXRecordDecl();
            if (cxxRecordDecl->isThisDeclarationADefinition() == clang::VarDecl::DeclarationOnly)
                return 0; // forward declaration
            templateArgs = ResolveCXXRecordTemplate(cxxRecordDecl, qualifiedName);

            if (qualifiedName.GetName().size() == 0)
                return 0;
            qualifiedName.templateArgs=templateArgs;
            kind = cType->isStructureType() ? metacpp::TypeKind::STRUCT : metacpp::TypeKind::CLASS;
            break;
        }
        case clang::Type::TypeClass::Enum:
        {

            qualifiedName = QualifiedName(defultQulifName);
            if (qualifiedName.GetName().size() == 0)
                return 0;
            kind = metacpp::TypeKind::ENUM;
            break;
        }
        default:
            std::cout << "Unsupported TypeClass " << cType->getTypeClassName() << std::endl;
            return 0;
        }

        TypeID typeId = m_Storage->AssignTypeID(qualifiedName.FullQualified());


        //cout<< "Found " << qualifiedName.FullQualified()<<endl;

        metacpp::Type* type=m_Storage->GetType(typeId);

        if (type!=nullptr && type->openForScrab)
            return type;

        if (type==nullptr){
            type = new metacpp::Type(typeId, qualifiedName);

            m_Storage->AddType(type);
        }
        type->openForScrab=true;


        //if(cxxRecordDecl==nullptr)//TODO for template class its seems wrong
//            return type;

        std::string path="";
        clang::CXXRecordDecl *cxxRecordDecl2=cType->getAsCXXRecordDecl();

        //cout<< "Found " << cxxRecordDecl->getQualifiedNameAsString() <<" : "<< " at "
        //    << path<<" != "<<getDeclLocation(cxxRecordDecl->getLocation())<< "\n";


        if(cxxRecordDecl2==nullptr){
            cerr<<"cxxRecordDecl2==nullptr"<<endl;

            auto p=cType->getAs<clang::TemplateSpecializationType>();
            cerr<<defultQulifName.GetName()<<endl;
            if(p==nullptr)
                    return type;
            auto SpecializationDecl=p
                                           ->desugar()
                                           ->getAs<clang::RecordType>()
                                           ->getDecl();

            for (const auto *Field : SpecializationDecl->fields()) {
              Field->getType().dump();
            }

            return type;

        }
        if(cxxRecordDecl2==nullptr){
            cerr<<"cxxRecordDecl2==nullptr after try"<<endl;



            return type;
        }
        path=getClearPath(getDeclLocation(cxxRecordDecl2->getLocation()));

        type->final=true;
        long long xttt=path.find("/home/lotfi/MyCompany/Tank.io/Classes/");
        if(xttt>-1)
            cout<<"gg"<<endl;
        if(path=="/home/lotfi/MyCompany/Tank.io/Classes/GameCore/units/unit/MachinGunTank.h")
            cout<<"gg"<<endl;
        if(qualifiedName.m_Name=="Misle")
            cout<<"go"<<endl;
        if(qualifiedName.m_Name=="AAAABBB")
            cout<<"go"<<endl;

        if(qualifiedName.m_Name=="TNodeManager"){

            cout<<"go"<<endl;
        }

        if(qualifiedName.m_Name=="ASDF_ASDF")
            cout<<"go"<<endl;


        if(qualifiedName.m_Name=="GObject")

            cout<<"go"<<endl;


        m_Storage->GetFile(path)->types.insert(type);
        type->file=m_Storage->GetFile(path);




        //if(m_Storage->HasType(typeId))
        //    return type;



		type->SetSize(m_Context->getTypeSize(cType) / 8);
		type->SetKind(kind);
		type->SetHasDefaultConstructor(cType->getTypeClass() == clang::Type::TypeClass::Builtin && qualifiedName.GetName() != "void");
		type->SetPolymorphic(false);









		if (auto cxxRecordDecl = cType->getAsCXXRecordDecl()) {
			type->SetPolymorphic(cxxRecordDecl->isPolymorphic());

			// Parse base classes
            for (const clang::CXXBaseSpecifier *it = cxxRecordDecl->bases_begin(); it != cxxRecordDecl->bases_end(); it++)
			{
                //it->getTypeSourceInfo();
				QualifiedType base_qtype = ResolveQualType(it->getType());
				type->AddBaseType(base_qtype, TransformAccess(it->getAccessSpecifier()));

				if (it == cxxRecordDecl->bases_begin()) {
					m_Storage->GetType(base_qtype.GetTypeID())->AddDerivedType(typeId);
                    auto type2=m_Storage->GetType(base_qtype.GetTypeID());
                    //auto z=clang::dyn_cast<clang::CXXRecordDecl>(base_qtype);
                    //getAllFields(z,type2);
				}
			}
			
			// Scrape annotations

            type->annotations  = ScrapeAnnotations(cxxRecordDecl);


            for (auto it = cxxRecordDecl->method_begin(); it != cxxRecordDecl->method_end(); it++) {
                clang::CXXMethodDecl* method = *it;
                if (method != 0) {
                    ScrapeMethodDecl(method, type);
                }
            }
            getFieldsOfType(cxxRecordDecl,type);


            if (cxxRecordDecl->isAbstract())
                type->SetHasDefaultConstructor(false);

		}

		for (auto qt : templateArgs)
            type->m_TemplateArguments2.push_back(qt);


       // if(cxxRecordDecl!=nullptr)
       //     if (auto declCtx = clang::TypedefDecl::castToDeclContext(cxxRecordDecl))
        //          traverseChilds(declCtx, type);


        //if (auto declCtx = cType->getAsTagDecl()->castToDeclContext(cType->getAsTagDecl()))
        //    ScrapeDeclContext(declCtx, type);

		return type;
    }

    void ASTScraper::getFieldsOfType(const clang::CXXRecordDecl* cxxRecordDecl, Type* type){
        type->m_Fields.clear();
        for (auto it = cxxRecordDecl->field_begin(); it != cxxRecordDecl->field_end(); it++) {
            clang::FieldDecl* field = *it;
            if (field != 0) {
                type->m_Fields.push_back(ScrapeFieldDecl(field, type));
            }
        }
    }
    Field ASTScraper::ScrapeFieldDecl(const clang::FieldDecl* fieldDecl, Type* parent)
    {

        //if (fieldDecl->isAnonymousStructOrUnion())
        //	return;

		metacpp::QualifiedName qualifiedName = ResolveQualifiedName(fieldDecl->getQualifiedNameAsString());

		Field field(ResolveQualType(fieldDecl->getType()), qualifiedName);

        field.SetOffset(0);//m_Context->getFieldOffset(fieldDecl) / 8);// soem time crash thats depend on your code our includes
        //field.SetOffset(m_Context->getFieldOffset(fieldDecl) / 8); //soem time crash thats depend on your code our includes

        field.annotations  = ScrapeAnnotations(fieldDecl);

        return field;
        //cType->getAsCXXRecordDecl()

	}

	void ASTScraper::ScrapeMethodDecl(const clang::CXXMethodDecl* cxxMethodDecl, Type* parent)
	{
		auto constructor = clang::dyn_cast<clang::CXXConstructorDecl>(cxxMethodDecl);
		auto destructor = clang::dyn_cast<clang::CXXDestructorDecl>(cxxMethodDecl);

		if (constructor && constructor->isDefaultConstructor() && constructor->getAccess() == clang::AccessSpecifier::AS_public) {
			parent->SetHasDefaultConstructor(true);
		}

		metacpp::QualifiedName qualifiedName = ResolveQualifiedName(cxxMethodDecl->getQualifiedNameAsString());

		Method method(qualifiedName);

		method.SetOwner(parent->GetTypeID());


		//cxxMethodDecl->dumpColor();

		//std::cout << cxxMethodDecl->isUserProvided() << cxxMethodDecl->isUsualDeallocationFunction() << cxxMethodDecl->isCopyAssignmentOperator() << cxxMethodDecl->isMoveAssignmentOperator() << std::endl;
		//std::cout << cxxMethodDecl->isDefaulted() << std::endl;

		// params
		for (auto it = cxxMethodDecl->param_begin(); it != cxxMethodDecl->param_end(); it++) {
			clang::ParmVarDecl* param = *it;

		}

		parent->AddMethod(method);
    }

    void ASTScraper::afterVisit(const clang::NamedDecl *namedDecl, Type *parent){
        if(namedDecl!=nullptr)
            if (auto declCtx = clang::TypedefDecl::castToDeclContext(namedDecl))
                traverseChilds(declCtx, parent);

    }

    QualifiedType ASTScraper::ResolveQualType(clang::QualType qualType)
    {
        MakeCanonical(qualType);

		QualifiedType qualifiedType;

		qualifiedType.SetConst(qualType.isConstant(*m_Context));

		if (auto ptr = clang::dyn_cast<clang::PointerType>(qualType.split().Ty)) {
			qualifiedType.SetQualifierOperator(QualifierOperator::POINTER);
			qualType = ptr->getPointeeType();
		}
		else if (auto ref = clang::dyn_cast<clang::ReferenceType>(qualType.split().Ty)) {
			qualifiedType.SetQualifierOperator(QualifierOperator::REFERENCE);
			qualType = ref->getPointeeType();
		} else
			qualifiedType.SetQualifierOperator(QualifierOperator::VALUE);

		MakeCanonical(qualType);

		const clang::Type* cType = qualType.split().Ty;

        Type* type = ScrapeType(nullptr,cType);//TODO for template class its seems wrong whay nullptr?
		qualifiedType.SetTypeID(type ? type->GetTypeID() : 0);

		return qualifiedType;
	}

	void ASTScraper::MakeCanonical(clang::QualType& qualType)
	{
		if (!qualType.isCanonical())
			qualType = qualType.getCanonicalType();
	}

	QualifiedName ASTScraper::ResolveQualifiedName(std::string qualifiedName)
	{
		RemoveAll(qualifiedName, "::(anonymous union)");
		RemoveAll(qualifiedName, "::(anonymous struct)");
		RemoveAll(qualifiedName, "::(anonymous)");

		RemoveAll(qualifiedName, "(anonymous union)");
		RemoveAll(qualifiedName, "(anonymous struct)");
		RemoveAll(qualifiedName, "(anonymous)");

		return QualifiedName(qualifiedName);
	}

	void ASTScraper::RemoveAll(std::string& source, const std::string& search)
	{
		std::string::size_type n = search.length();
		for (std::string::size_type i = source.find(search); i != std::string::npos; i = source.find(search))
			source.erase(i, n);
	}

	AccessSpecifier ASTScraper::TransformAccess(const clang::AccessSpecifier as)
	{
		switch (as) {
		case clang::AccessSpecifier::AS_public:
		case clang::AccessSpecifier::AS_none:
			return AccessSpecifier::PUBLIC;
		case clang::AccessSpecifier::AS_protected:
			return AccessSpecifier::PROTECTED;
		case clang::AccessSpecifier::AS_private:
		default:
			return AccessSpecifier::PRIVATE;
		}
	}

    std::set<std::string> ASTScraper::ScrapeAnnotations(const clang::Decl* decl)
	{
        std::set<std::string> annotations;

		if (decl && decl->hasAttrs()) {
			auto it = decl->specific_attr_begin<clang::AnnotateAttr>();
			auto itEnd = decl->specific_attr_end<clang::AnnotateAttr>();


            std::cout << "decl! " << decl->getDeclKindName()  << std::endl;
            std::cout<<std::endl;

            //clang::At
			// __attribute__((annotate("...")))
			for (; it != itEnd; it++) {
				const clang::AnnotateAttr* attr = clang::dyn_cast<clang::AnnotateAttr>(*it);


                if (attr){
                    std::string tt=attr->getAnnotation();
                    std::cout<< tt << std::endl;
                    //annotations.emplace_back(tt);
                    annotations.insert(tt);
                }
			}
		}

		return annotations;
	}

	bool ASTScraper::IsReflected(const std::vector<std::string>& attrs)
	{
		if (m_Config.AnnotationRequired.size() == 0)
			return true;
		return std::find(attrs.begin(), attrs.end(), m_Config.AnnotationRequired) != attrs.end();
	}

	void ASTScraper::SetContext(clang::ASTContext* context)
	{
		m_Context = context;
    }

    std::string ASTScraper::getDeclLocation(clang::SourceLocation Loc) const {
        return SM->getFilename(Loc).str();
        std::ostringstream OSS;
        OSS << SM->getFilename(Loc).str() << ":"
            << SM->getSpellingLineNumber(Loc) << ":"
            << SM->getSpellingColumnNumber(Loc);
        return OSS.str();
    }
}
