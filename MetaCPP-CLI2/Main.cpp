#include <iostream>
#include <fstream>
#include <string>

#include <llvm/Support/CommandLine.h>

#include "ASTScraper.hpp"
#include "MetaExporter.hpp"
#include "ScraperTool.hpp"
#include "MetaCPP/Storage.hpp"
#include <MetaCPP/JsonSerializer.hpp>
#include <MetaCPP/Runtime.hpp>
using namespace llvm;
using namespace std;

//static cl::opt<std::string> InputSource(cl::Positional, cl::Required, "in-source", cl::desc("Specify input filename"), cl::value_desc("filename"));
static cl::opt<std::string> OutputHeader("out-header", cl::desc("Specify headeroutput filename"), cl::value_desc("filename"), cl::init("output.hpp"));
//static cl::opt<std::string> InputSource("in-source", cl::desc("Specify headeroutput filename"), cl::value_desc("filename"), cl::init("/home/lotfi/MyCompany/DoodleJump/Classes/AppDelegate.cpp"));
//static cl::opt<std::string> InputSource("in-source", cl::desc("Specify headeroutput filename"), cl::value_desc("filename"),cl::init("/home/lotfi/MyCompany/Tank.io/Classes/ImportantCommon/MyWidget/MySprite.h"));


//static cl::opt<std::string> InputSource("in-source", cl::desc("Specify headeroutput filename"), cl::value_desc("filename"), cl::init("../MetaCPP/include/MetaCPP/all.hpp"));
static cl::opt<std::string> InputSource("in-source", cl::desc("Specify headeroutput filename"), cl::value_desc("filename"), cl::init("../Example/objects.hpp"));
//static cl::opt<std::string> InputSource("in-source", cl::desc("Specify headeroutput filename"), cl::value_desc("filename"), cl::init("/home/lotfi/programing/cpp/test/cppMetaTEst/main.cpp"));

static cl::opt<std::string> OutputSource("out-source", cl::desc("Specify source output filename"), cl::value_desc("filename"), cl::init("output.cpp"));
static cl::opt<std::string> OutputFile    ("out-meta", cl::desc("Specify source output filename"), cl::value_desc("filename"), cl::init("output.json"));

static cl::list<std::string> CompilerFlags("flag", cl::desc("Compiler flags"), cl::value_desc("flags"), cl::ZeroOrMore);
//static cl::list<std::string> CompilerFlags("flag",     cl::desc("Compiler flags")                , cl::value_desc("flags")   , {"-I../cocos2d -I../cocos2d/cocos"});

static cl::opt<std::string> ReflectionAnnotation("reflection-annotation", cl::desc("Only reflect types that contain this annotation"), cl::value_desc("name"));




vector<string> files={

    "Classes/GameCore/BoardObject",
    "Classes/GameCore/Player",
    "Classes/GameCore/units/Misle/Misle",
    "Classes/GameCore/units/Misle/Patriot",
    "Classes/GameCore/units/unit/Tank",
    "Classes/GameCore/units/unit/MachinGunTank",
    "Classes/GameCore/units/unit/Helicopter",
    "Classes/GameCore/units/unit/MisleTank",
    "Classes/GameCore/utils/Box",
    "Classes/GameCore/utils/CircleBlock",
    "Classes/GameCore/utils/GemGenerator",
    "Classes/GameCore/utils/PlatformLine",
    "Classes/GameCore/utils/PrizePoint",
    "Classes/GameCore/utils/Tree",
    "Classes/GameCore/GameCore",
    "Classes/Entity/Catalog",
    "Classes/Entity/UserData",
    "Classes/Entity/BuyPackage",
    "Classes/Entity/Match",
    "Classes/Grains/MatchGroup",


    "Classes/base_message/BaseMessage",
    "Classes/game_message/Messages",
    "Classes/game_message/GameMsgs",
    "Classes/game_message/societalgaming",/**/



};
string stringFormat(const string &s,map<string,string> mp){
    string res=s;
    for(auto t :mp){
        size_t pos = 0;
        string serach="{"+t.first+"}";
        while((pos = res.find(serach, pos)) != string::npos){
            res.replace(pos,serach.length(),t.second);
        }
    }
    return res;
}
int TypeIdC=1;
metacpp::Storage* storage = new metacpp::Storage();

std::string humanReadbleName(std::string name){
    map<string,string> replaceStrs={{"std::__cxx11::basic_string<char,std::char_traits<char>,std::allocator<char>>","std::string"},
                                   {"long long","int64_t"},
                                   {"int32_t","int"},
                                   {",std::allocator<int64_t>>",">"}
                                   };

    bool ph=true;
    while(ph){
        ph=false;
        for(auto &t:replaceStrs){
            string f=t.first;
            int x=name.find(f);
            if(x>-1){
                name.replace(name.begin()+x,name.begin()+x+f.size(),t.second);
                ph=true;
            }
        }
    }

    return name;
}


std::string convertToFileName(std::string name){
    map<string,string> replaceStrs={{"::","_d_"},
                                   {"<","_l_"},
                                   {">","_g_"},
                                   {",","_v_"}
                                   };

    bool ph=true;
    while(ph){
        ph=false;
        for(auto &t:replaceStrs){
            string f=t.first;
            int x=name.find(f);
            if(x>-1){
                name.replace(name.begin()+x,name.begin()+x+f.size(),t.second);
                ph=true;
            }
        }
    }

    return name;

}

std::map<metacpp::TypeID,std::string>  fils;
std::map<string,int > typeIdz;
bool printClassMeta(metacpp::Type *class_,ostringstream &fout,ostringstream &fout2){
    if(class_->notSaved++)
        return false;
    cout<<class_->GetQualifiedName().FullQualified()<<" "<<class_->m_Methods.size()<<" "<<class_->m_Fields.size()<<endl;
    if(class_->GetQualifiedName().m_Name=="ASDF_ASDF"){
        cout<<"yaftam "<<endl;
    }
        if(class_->annotations.find("FS_IGNORE_FIELD")!=class_->annotations.end())
            return false;
        bool hasGetType=false;
        bool haveState=false;
        bool notPolyMorphic=false;
        for(auto &t:class_->m_Methods){
            if(t.GetQualifiedName().GetName()=="GET_TYPE"){
                hasGetType=true;
            }
            if(t.GetQualifiedName().GetName()=="set_data"){
                haveState=true;
            }
        }
        if(class_->annotations.find("FS_SYNC")!=class_->annotations.end())
            notPolyMorphic=true;
        //hasGetType=true;
        if(!hasGetType && !notPolyMorphic)
            return false;
        bool res=false;
        for(auto t:class_->GetBaseTypes()){
            auto bases=storage->GetType(t.type.GetTypeID());
            if(bases->file==class_->file)
               res|=printClassMeta(bases,fout,fout2);

        }
        if(class_->typeId==0){
            if(typeIdz[class_->GetQualifiedName().FullQualified()]==0){
               typeIdz[class_->GetQualifiedName().FullQualified()]=TypeIdC++;
            }
            class_->typeId=typeIdz[class_->GetQualifiedName().FullQualified()];
        }

        fout2<<stringFormat("int FS::MetaInfo<{class}>::TypeId={typeId};",{{"class", class_->GetQualifiedName().FullQualified()},{"typeId",to_string(class_->typeId)}})<<endl;
        //maps[TypeIdC] =class_;


        fout2<<stringFormat("FS::MetaInfo<{class}>   FS::MetaInfo<{class}> ::instance;",{{"class", class_->GetQualifiedName().FullQualified()},{"typeId",to_string(TypeIdC)}})<<endl;
         cout<<class_->GetQualifiedName().FullQualified()<<endl;
         std::vector<metacpp::Field> vars0=class_->m_Fields;
         std::vector<metacpp::Field> vars;
         bool syncer=true;
         for(auto t:vars0){
             if(t.GetQualifiedName().FullQualified()=="___syncable_object_stop")
                 break;
             if(t.GetQualifiedName().GetName().rfind("__",0)==0)
                 continue;
             if(t.annotations.find("FS_IGNORE_FIELD")!=t.annotations.end())
                 continue;
             if(syncer)
                vars.push_back(t);
             if(t.annotations.find("FS_SYNC_STOP")!=t.annotations.end())
                 syncer=false;


         }
         for(auto t:vars){
             auto bases=storage->GetType(t.GetType().GetTypeID());
             if(bases!=nullptr && bases->file==class_->file)
                res|=printClassMeta(bases,fout,fout2);

         }


         size_t j=0;
         for(size_t i=0; i<vars.size();++i){


             vars[j++]=vars[i];
             std::string cl=humanReadbleName(vars[i].GetType().GetQualifiedName(storage));
             if(fils.find(vars[i].GetType().m_Type)!=fils.end()){
                    fout<<stringFormat("#include\"{fn}\"",{{"fn",fils[vars[i].GetType().m_Type]}})<<endl;
             }else{
                 /*auto clf=convertToFileName(cl);
                ofstream fout3(clf);

                fout3.close();/**/
             }
         }
         vars.resize(j);



        if(haveState && false){

            fout2<<stringFormat("void {class}::set_data(const SyncObject *state){\n"
                "   if(last_update_from_server_time>state->my_step)\n"
                "        return;\n",{{"class",class_->GetQualifiedName().FullQualified()}})<<endl;

            if(class_->m_BaseTypes.size()>0)
                fout2<<stringFormat("   {parent}::set_data(state);\n"
                                ,{{"parent",class_->m_BaseTypes[0].type.GetQualifiedName(storage)}})<<endl;

            fout2<<stringFormat("   auto cs=static_cast<const {class}*>(state);\n",{{"class",class_->GetQualifiedName().FullQualified()}})<<endl;
            for(auto &item : vars)
                fout2<<stringFormat("    this->{item}=cs->{item}; ",{{"item", item.m_QualifiedName.FullQualified()}})<<endl;

            fout2<<"}\n"<<endl;

        }


        {
            string bases="";
            if(class_->m_BaseTypes.size()>0)
                bases= stringFormat(" MetaInfo<{class}>",{{"class",class_->m_BaseTypes[0].type.GetQualifiedName(storage)}});
            else
                bases= "classMeta";

            fout<<stringFormat("template<> \nstruct FS::MetaInfo<{class}>:public {bases}{",{{"class", class_->m_QualifiedName.FullQualified()},{"bases",bases}})<<endl;

            fout<<stringFormat("    typedef {class} T;",{{"class", class_->m_QualifiedName.FullQualified()}})<<endl;
            fout<<stringFormat("    static int TypeId ;",{{"class", class_->m_QualifiedName.FullQualified()}})<<endl;
            for(auto &item : vars)
                fout<<stringFormat("    FieldMeta {item}; ",{{"item", item.m_QualifiedName.GetName()}})<<endl;

            fout<<stringFormat("    static MetaInfo<{class}> instance;",{{"class", class_->m_QualifiedName.FullQualified()}})<<endl;

            {

                fout<<stringFormat("    struct Ofssets{",{})<<endl;

                string prv_name="0";
                if(class_->m_BaseTypes.size()>0)
                    prv_name=stringFormat("MetaInfo<{m_BaseTypes}>::getSizeStatic()",{{"m_BaseTypes",class_->m_BaseTypes[0].type.GetQualifiedName(storage)}});

                for(size_t i=0; i< vars.size() ; ++i){
                    if(i==0)
                        fout<<stringFormat("        static constexpr size_t {name}(){return {prv_name}; }",{{"prv_name",prv_name},{"name",vars[i].m_QualifiedName.GetName()}})<<endl;
                    else
                        fout<<stringFormat("        static constexpr size_t {name}(){return {prv_name}()+MetaInfo<decltype(T::{prv_name})>::getSizeStatic();} ",{
                                               {"prv_name",prv_name},
                                               {"name",vars[i].m_QualifiedName.GetName()},
                                               {"type", vars[i].m_Type.GetQualifiedName(storage)}
                                           })<<endl;
                    prv_name=vars[i].m_QualifiedName.GetName();
                }

                fout<<stringFormat("    };",{})<<endl;
            }
            {
                fout<<stringFormat("    constexpr static size_t getSizeStatic(){",{})<<endl;

                fout<<stringFormat("        return ",{});

                //if(class_->m_BaseTypes.size()>0)
                //    fout<<stringFormat("MetaInfo<{m_BaseTypes}>::getSizeStatic()+",{{"m_BaseTypes",class_->m_BaseTypes[0].type.GetQualifiedName(storage)}})<<endl;

                //for(auto &item : vars)
                //    fout<<stringFormat("            MetaInfo<decltype(T::{name})>::getSizeStatic()+ ",{{"name", item.m_QualifiedName.GetName()}})<<endl
                if(vars.size()>0){
                    std::string rr=vars[vars.size()-1].m_QualifiedName.GetName();
                    fout<<stringFormat("            Ofssets::{name}()+MetaInfo<decltype(T::{name})>::getSizeStatic() ;\n",{{"name", rr}})<<endl;
                }else if(class_->m_BaseTypes.size()>0){
                        fout<<stringFormat("MetaInfo<{m_BaseTypes}>::getSizeStatic();\n",{{"m_BaseTypes",class_->m_BaseTypes[0].type.GetQualifiedName(storage)}})<<endl;
                }else{
                    fout<<"0;\n";
                }
                //fout<<stringFormat("            ; ",{})<<endl;

                fout<<stringFormat("    }",{{"class", class_->m_QualifiedName.FullQualified()}})<<endl;
            }



            {

                fout<<stringFormat("    void getRecordLayoutStatic(){",{{"class", class_->m_QualifiedName.FullQualified()}})<<endl;
                fout<<"    __fieldsa.clear();"<<endl;

                //fout<<stringFormat("        MetaInfo<{class}>  *res=new  MetaInfo<{class}> ();",{{"class", class_->m_QualifiedName.FullQualified()}})<<endl;

                if(class_->m_BaseTypes.size()>0){
                    fout<<stringFormat("MetaInfo<{base}>::getRecordLayoutStatic();",{{"base",class_->m_BaseTypes[0].type.GetQualifiedName(storage)}})<<endl;
                    fout<<stringFormat("        MetaInfo<{m_BaseTypes}>::instance.addDrived(this);",{
                                           {"m_BaseTypes",class_->m_BaseTypes[0].type.GetQualifiedName(storage)}
                                       })<<endl;

                    fout<<stringFormat("        suberClass=&MetaInfo<{m_BaseTypes}>::instance;",{
                                           {"m_BaseTypes",class_->m_BaseTypes[0].type.GetQualifiedName(storage)}
                                       })<<endl;
                }

                fout<<stringFormat("        __name=\"{class}\";",{{"class", class_->m_QualifiedName.FullQualified()}})<<endl;
                for(auto &item : vars)
                    fout<<stringFormat("        {name}={Ofssets::{name}(),&MetaInfo<decltype(T::{name})>::instance,\"{name}\",offsetof({class},{name})};",{{"class", class_->m_QualifiedName.FullQualified()},{"name", item.m_QualifiedName.GetName()}})<<endl;

                for(auto &item : vars)
                    fout<<stringFormat("        addField(&({name}));",{{"name", item.m_QualifiedName.GetName()}})<<endl;


                fout<<stringFormat("         classMeta::size=getSizeStatic();",{})<<endl;

                //fout<<stringFormat("        return res; ",{})<<endl;

                fout<<stringFormat("    }",{})<<endl;
            }
            fout<<"static MetaInfo<T> *setRecordLayoutStatic(classMeta *res){\n"
                  "     auto res2= new MetaInfo<T>(); \n"
                  "     res2->getRecordLayoutStatic();\n"
                  "     res2->cloneFrom(res);\n"
                  "}\n"<<endl;

            {
                fout<<stringFormat("    static inline void toBinaryStatic(const {class} *thiz,DataContiner &data,size_t offset){",{{"class", class_->m_QualifiedName.FullQualified()}})<<endl;

                if(class_->m_BaseTypes.size()>0){
                    fout<<stringFormat("        MetaInfo<{m_BaseTypes}>::toBinaryStatic(thiz,data,offset);",{
                                           {"m_BaseTypes",class_->m_BaseTypes[0].type.GetQualifiedName(storage)}
                                       })<<endl;


                }

                for(auto &item : vars)
                    fout<<stringFormat("        WRITE(thiz->{name},data,offset+Ofssets::{name}());",{{"name", item.m_QualifiedName.GetName()}})<<endl;

                fout<<stringFormat("    }",{})<<endl;
            }

            {
                fout<<stringFormat("    static inline void fromBinaryStatic({class} *thiz,const DataContiner &data,size_t offset){",{{"class", class_->m_QualifiedName.FullQualified()}})<<endl;

                if(class_->m_BaseTypes.size()>0)
                    fout<<stringFormat("        MetaInfo<{m_BaseTypes}>::fromBinaryStatic(thiz,data,offset);",{{"m_BaseTypes",class_->m_BaseTypes[0].type.GetQualifiedName(storage)}})<<endl;

                for(auto &item : vars)
                    fout<<stringFormat("        READ(thiz->{name},data,offset+Ofssets::{name}());",{{"name", item.m_QualifiedName.GetName()}})<<endl;



                fout<<stringFormat("    }",{})<<endl;
            }

            {
                fout<<stringFormat("    static inline void toBinaryDynamic(const {class} *thiz,DataContiner &data,size_t offset){",{{"class", class_->m_QualifiedName.FullQualified()}})<<endl;
                if(class_->m_BaseTypes.size()>0)
                    fout<<stringFormat("        MetaInfo<{m_BaseTypes}>::toBinaryDynamic(thiz,data,offset);",{
                                           {"m_BaseTypes",class_->m_BaseTypes[0].type.GetQualifiedName(storage)}})<<endl;

                for(auto &item : vars)
                    fout<<stringFormat("        WRITE_DYNAMIC(thiz->{name},data,offset+MetaInfo<{class}>::instance.{name}.start);",{{"name", item.m_QualifiedName.GetName()},{"class", class_->m_QualifiedName.FullQualified()}})<<endl;

                fout<<stringFormat("    }",{})<<endl;
            }
            {
                fout<<stringFormat("    static inline void fromBinaryDynamic({class} *thiz,const DataContiner &data,size_t offset){",{{"class", class_->m_QualifiedName.FullQualified()}})<<endl;

                if(class_->m_BaseTypes.size()>0)
                    fout<<stringFormat("        MetaInfo<{m_BaseTypes}>::fromBinaryDynamic(thiz,data,offset);",{{"m_BaseTypes",class_->m_BaseTypes[0].type.GetQualifiedName(storage)}})<<endl;

                for(auto &item : vars)
                    fout<<stringFormat("        READ_DYNAMIC(thiz->{name},data,offset+MetaInfo<{class}>::instance.{name}.start);",{{"name", item.m_QualifiedName.GetName()},{"class", class_->m_QualifiedName.FullQualified()}})<<endl;

                fout<<stringFormat("    }",{})<<endl;
            }
            if(!class_->isAbstract){
                fout<<" virtual void* create()const override{\n\
                    return new T();\n\
                }"<<endl;
            }
        fout<<""
              " virtual classMeta* getMetaInfoV()const override {\n\
        return &MetaInfo<T>::instance;\n\
        }"<<endl;
            fout<<"    virtual void setTypeId(int t)override {\nTypeId=t;\n}"<<endl;

            fout<<"    virtual void fromBinaryStaticV(void  *thiz,const DataContiner &d,const size_t &offset)const override{\n"
                  "         fromBinaryStatic((T*)thiz,d,offset);\n"
                  "    }\n"<<endl;

            fout<<"    virtual void toBinaryStaticV(const void  *thiz,DataContiner &d,const size_t &offset)const override{\n"
                  "         toBinaryStatic((T*)thiz,d,offset);\n"
                  "    }\n"<<endl;
            fout<<"virtual size_t getSizeStaticV()const override{\n"
                          "     return getSizeStatic();\n"
                          "}\n"<<endl;

            fout<<"    virtual void fromBinaryDynamicV(void  *thiz,const DataContiner &d,const size_t &offset)const override{\n"
                  "        fromBinaryDynamic((T*)thiz,d,offset);\n"
                  "    }\n"
                  "    virtual void toBinaryDynamicV(const void  *thiz,DataContiner &d,const size_t &offset)const override{\n"
                  "        toBinaryDynamic((T*)thiz,d,offset);\n"
                  "    }\n"
                  "    virtual size_t getSizeDynamicV()const override{\n"
                  "        return classMeta::size;\n"
                  "    }\n"
               <<endl;

            fout<<stringFormat("};",{})<<endl;

            if(hasGetType){
                fout2<<stringFormat("FS::classMeta* {class}::GET_TYPE()const{\nreturn &FS::MetaInfo<{class}>::instance;\n}",{{"class",class_->m_QualifiedName.FullQualified()}})<<endl;
                fout2<<stringFormat("DataContiner* {class}::getData()const{  DataContiner *d=new DataContiner();\n"
                                "FS::MetaInfo<{class}>::toBinaryStatic(this,*d,0);"
                                "return d;}",{{"class",class_->m_QualifiedName.FullQualified()}})<<endl;
            }

            fout<<stringFormat("template<>\nstruct FS::MetaInfo<const {class}>:public MetaInfo<{class}>{};",{{"class",class_->m_QualifiedName.FullQualified()}})<<endl;


        }
            return true;
}



int main(int argc, const char** argv) {

    cout<<"-------------------"<<endl;
    cl::ParseCommandLineOptions(argc, argv, "MetaCPP");



	// Generate Metadata
	{
		metacpp::ASTScraper::Configuration configuration;
		configuration.AnnotationRequired = ReflectionAnnotation;

		metacpp::ASTScraper* scraper = new metacpp::ASTScraper(storage, configuration);

        //files.clear();

         std::string dir="/home/lotfi/MyCompany/Tank.io/";
         vector<std::string> CompilerFlags={"-DSYNC_META_PROCCESS ",
                                            "-DEDITOR_MODE",
                                            "-DLINUX",
                                            "-DSOURCE_DIR=\"/home/lotfi/MyCompany/Tank.io\"",
                                            //"-DCHECK_LLVM_FLAG",
                                            "-DRAPIDJSON_HAS_STDSTRING=1",
                                            "-DUSE_BULLET=FALSE",
                                            "-DUSE_CHIPMUNK=FALSE",
                                            "-I/home/lotfi/MyCompany/Tank.io/cocos2d",
                                            "-I/home/lotfi/MyCompany/Tank.io/cocos2d/cocos",
                                            "-I/home/lotfi/MyCompany/Tank.io/cocos2d/deprecated",
                                            "-I/home/lotfi/MyCompany/Tank.io/cocos2d/cocos/platform",
                                            "-I/home/lotfi/MyCompany/Tank.io/cocos2d/extensions",
                                            "-I/home/lotfi/MyCompany/Tank.io/cocos2d/external",
                                            "-I/home/lotfi/MyCompany/Tank.io/cocos2d/external/glfw3/include/linux",
                                            "-I/home/lotfi/MyCompany/Tank.io/cocos2d/cocos/audio/include",
                                            "-I/home/lotfi/MyCompany/Tank.io/Classes",
                                            "-I/home/lotfi/MyCompany/Tank.io/Classes/ImportantCommon/MyWidget",
                                            "-I/home/lotfi/MyCompany/Tank.io/Classes/ImportantCommon",
                                            "-I/home/lotfi/MyCompany/Tank.io/Classes/FSerilizer",
                                            "-I/home/lotfi/MyCompany/Tank.io/Classes/GameCore",
                                            "-I/home/lotfi/MyCompany/Tank.io/Classes/ImportantCommon/UiCreator",
                                            "-I/home/lotfi/MyCompany/Tank.io",
                                            "-I/home/lotfi/MyCompany/Tank.io/Classes/ImportantCommon/MathShapeUtils",
                                            "-I.",
                                            "-isystem",
                                            "-isystem",
                                            "-std=c++11",
                                            "-Wall",
                                            "-std=gnu++11"};
         if(false){
             CompilerFlags.clear();
            CompilerFlags.push_back("-std=c++11");
         }
         for(auto t: CompilerFlags)
             cout<<"--"<<t<<"--"<<endl;
        std::cout<<"---------------------------- 1"<<endl;
         //metacpp::ScraperTool tool("/home/lotfi/MyCompany/Tank.io/Classes/","/home/lotfi/MyCompany/Tank.io/Classes/baseMessage/include_all.hpp", CompilerFlags);

        metacpp::ScraperTool tool("/home/lotfi/MyCompany/Tank.io/Classes/","/home/lotfi/MyCompany/Tank.io/Classes/AppDelegate.cpp", CompilerFlags);


        //metacpp::ScraperTool tool("/home/lotfi/untitled12/","/home/lotfi/untitled12/main.cpp", CompilerFlags);

        //metacpp::ScraperTool tool("/home/lotfi/MyCompany/Tank.io/Classes/","/home/lotfi/MyCompany/Tank.io/Classes/include_all.hpp", CompilerFlags);
         //metacpp::ScraperTool tool("/home/lotfi/MyCompany/Tank.io/Classes/","/home/lotfi/MyCompany/Tank.io/Classes/GameCore/units/unit/MachinGunTank.h", CompilerFlags);
        //metacpp::ScraperTool tool("/home/lotfi/MyCompany/Tank.io/Classes/","/home/lotfi/programing/cpp/MetaCPP/Example/objects.hpp", CompilerFlags);
         tool.Run(scraper);



         std::cout<<"---------------------------- 1"<<endl;




		delete scraper;
	}

	// Export Metadata
	{
        //metacpp::MetaExporter exporter(storage);
        //exporter.Export(InputSource, OutputHeader, OutputSource,OutputFile);
	}


{



        std::set<std::string> fns={""};


        ifstream fin("class2.txt");
        while(true ){
            string typeName;
            int typeId;
            if(!(fin>>typeName>>typeId))
                break;
            if(typeId>TypeIdC)
                TypeIdC=typeId+1;
            typeIdz[typeName]=typeId;
        }
        fin.close();
        std::string dir="/home/lotfi/MyCompany/Tank.io/Classes/";
        vector<std::string> dirs={
            "/home/lotfi/MyCompany/Tank.io/Classes/",
            "/home/lotfi/MyCompany/Tank.io/cocos2d/"
        };

        vector<std::string> dirNot={"/home/lotfi/MyCompany/Tank.io/Classes/FSerilizer"};
        //dir="/home/lotfi/programing/cpp/MetaCPP/Example/";
        for(auto f:storage->m_files){
            std::string fn=f.first;
            size_t max=0;
            for(auto dir:dirs ){
                if(fn.rfind(dir,0)==0 && max<dir.size() )
                    max=dir.size();
            }
            for(auto dir:dirNot ){
                if(fn.rfind(dir,0)==0 && max<dir.size() )
                    max=0;
            }
            if(max==0)
                continue;

            std::cout<<"go to print class of "<<fn<<std::endl;
            auto dotp=fn.rfind('.');
            auto fn0=fn.substr(dir.size(),dotp-dir.size());


            std::ostringstream inlineFileWriter;
            std::ostringstream cppFileWriter;


            //std::c

            string FILE_NAME="";
            for(auto c:fn0)
                if(c=='/')
                  FILE_NAME+="__";
                else
                  FILE_NAME+=c;



            inlineFileWriter<<stringFormat("#ifndef {FILE_NAME}_STATE_H",{{"FILE_NAME",FILE_NAME}})<<endl;
            inlineFileWriter<<stringFormat("#define {FILE_NAME}_STATE_H",{{"FILE_NAME",FILE_NAME}})<<endl;
            inlineFileWriter<<stringFormat("#include\"FSerilizer/base.hpp\"",{})<<endl;


            cppFileWriter<<"#ifndef FOR_SYNC"<<endl;
            cppFileWriter<<stringFormat("#include\"{fn}.h\"",{{"fn",fn0}})<<endl;

            bool haveClassForWrite=false;
            for(auto &t:f.second->types)
                haveClassForWrite|=printClassMeta(t,inlineFileWriter,cppFileWriter);

            cppFileWriter<<"#endif"<<endl;
            inlineFileWriter<<"#endif"<<endl;

            std::cout<<f.first<<" "<<f.second->types.size()<<endl;
            if(haveClassForWrite){
                auto x=dir+fn0+".State.inl";
                auto cppfn=dir+fn0+".State.cpp";



                auto str2=inlineFileWriter.str();


                std::ifstream t(x);
                std::string str((std::istreambuf_iterator<char>(t)),
                                 std::istreambuf_iterator<char>());
                t.close();

                if(str2!=str){
                    std::ofstream inlineFileWriter0(x);
                    inlineFileWriter0<<str2;
                    inlineFileWriter0.close();
                }

                {
                    std::ifstream t(cppfn);
                    std::string str((std::istreambuf_iterator<char>(t)),
                                     std::istreambuf_iterator<char>());
                    t.close();
                    str2=cppFileWriter.str();
                    if(str2!=str){

                        std::ofstream cppFileWriter0(cppfn);

                        cppFileWriter0<<cppFileWriter.str();

                        cppFileWriter0.close();
                    }
                }
            }


        }




        /*for(auto f:storage->m_IDs){
            metacpp::Type* tmp=storage->m_Types[f.second];
            std::string json = serializer.Serialize(tmp, true);
            std::cout<<f.second<<" " <<f.first<<" "<< json << std::endl;
            for(auto &field:tmp->m_Fields){
                for(auto &nm:field.m_QualifiedName.m_Namespace)
                    cout<<"::"<<nm;
                cout<<"::"<<field.m_QualifiedName.m_Name<<endl;
            }
        }
        cout<<"class es "<<storage->m_IDs.size()<<endl;/**/
    }


    {
        ofstream fout("class.txt");
        for(auto &t:storage->m_Types){
            fout<<t.second->GetQualifiedName().FullQualified2(storage)<<" "<<t.second->typeId<<"\n\n"<<endl;
        }
        fout.close();
    }


    {
        ofstream fout("class2.txt");
        for(auto &t:storage->m_Types)if(t.second->typeId){
            fout<<t.second->GetQualifiedName().FullQualified()<<" "<<t.second->typeId<<endl;
        }
        fout.close();
    }




	return 0;
}



