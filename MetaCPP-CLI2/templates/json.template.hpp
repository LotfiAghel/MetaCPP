R"(/****************************************************************************
** MetaCPP
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <MetaCPP/Type.hpp>
#include <MetaCPP/Field.hpp>
#include <MetaCPP/Method.hpp>
#include <MetaCPP/Storage.hpp>
#include <MetaCPP/SequentialContainer.hpp>

{{#includes}}
#include "{{file}}"
{{/includes}}

namespace metacpp { namespace generated {
        void Load(Storage* storage) {
                //////////////////////////////////////////////
                // Types
                //////////////////////////////////////////////
                {{#types}}
                {
                        Type* type = new Type({{id}}, QualifiedName("{{qualifiedName}}"));
                        type->SetSize({{size}});
                        type->SetKind(static_cast<TypeKind>({{kind}}));
                        type->SetAccess(static_cast<AccessSpecifier>({{access}}));
                        type->SetPolymorphic({{polymorphic}});

                        // Base Types
                        {{#baseTypes}}
                        {
                                {{#qualifiedType}}
)" +
#include "source.qualifiedType.template"
+ R"(
                                {{/qualifiedType}}
                                type->AddBaseType(qualifiedType, static_cast<AccessSpecifier>({{access}}));
                        }
                        {{/baseTypes}}

                        // Derived Types
                        {{#derivedTypes}}
                        type->AddDerivedType({{derivedTypeId}});
                        {{/derivedTypes}}

                        // Template arguments
                        {{#templateArguments}}
                        {
)" +
#include "source.qualifiedType.template"
+ R"(
                                type->AddTemplateArgument(qualifiedType);
                        }
                        {{/templateArguments}}

                        // Fields
                        {{#fields}}
                        {
                                {{#qualifiedType}}
)" +
#include "source.qualifiedType.template"
+ R"(
                                {{/qualifiedType}}

                                Field field(qualifiedType, QualifiedName("{{qualifiedName}}"));
                                field.SetOffset({{offset}});
                                type->AddField(field);
                        }
                        {{/fields}}

#if ({{access}} == 0 && {{valid}} == 1) // public & valid
                        TypeInfo<{{qualifiedName}}>::ID = {{id}};
                        TypeInfo<{{qualifiedName}}>::TYPE = type;

#if ({{polymorphic}} == 1) // polymorphic
                        // Dynamic Casts
                        {{#derivedTypes}}
                        storage->AddDynamicCast({{id}}, {{derivedTypeId}}, [](void* ptr) -> void* {
                                {{qualifiedName}}* obj = reinterpret_cast<{{qualifiedName}}*>(ptr);
                                return dynamic_cast<{{derivedQualifiedName}}*>(obj);
                        });
                        {{/derivedTypes}}
#endif

#if ({{hasDefaultConstructor}} == 1) // hasDefaultConstructor
                        // Constructor
                        type->SetConstructor([](void* ptr) -> void* {
                                return new (ptr) {{qualifiedName}};
                        });
#endif

#if ({{isSequentialContainer}} == 1) // isSequentialContainer
                        // SequentialContainer
                        class SequentialContainerSpecialization : public SequentialContainer {
                        public:
                                SequentialContainerSpecialization(Type* type) : m_Type(type) {};

                                QualifiedType ValuesType() const override {
                                        return m_Type->GetTemplateArguments().at(0);
                                }

                                size_t Size(void* container) const override {
                                        return Cast(container).size();
                                }

                                void* At(void* container, size_t index) const override {
                                        return &Cast(container).at(index);
                                }

                                void PushBack(void* container, void* item_ptr) const override {
                                        {{containerValueQualifiedName}}* item_obj = reinterpret_cast<{{containerValueQualifiedName}}*>(item_ptr);
                                        Cast(container).push_back(*item_obj);
                                }

                        private:
                                {{qualifiedName}}& Cast(void* container) const {
                                        return *reinterpret_cast<{{qualifiedName}}*>(container);
                                }

                                Type* m_Type;
                        };
                        type->SetContainer(new SequentialContainerSpecialization(type));
#endif

#endif

                        storage->AddType(type);
                }
                {{/types}}

                //////////////////////////////////////////////
                // IDs
                //////////////////////////////////////////////
                {{#ids}}
                storage->AssignTypeID("{{qualifiedName}}", {{id}});
                {{/ids}}
        }
} }

)"
