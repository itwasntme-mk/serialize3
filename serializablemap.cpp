///\file serializablemap.cpp

#include "serializablemap.h"
#include "constants.h"
#include "file_comparator.h"

#include <fstream>
#include <string>
#include <algorithm>
#include <stdlib.h> 
#include <limits.h>
#include <iostream>
#include <memory>
#include <stdexcept>


namespace bfs = boost::filesystem;


TSerializableMap::TSerializableMap(const TClasses& classes, const TEnums& enums, TLogger& logger,
  const std::vector<path>& inputs, const path& working_dir, const std::string& output_prefix,
#if defined(GENERATE_ENUM_OPERATORS)
  const std::vector<std::string>& ignoredNamespaces,
#endif
  int indent, bool check_for_changes)
  : Classes(classes), Enums(enums), Logger(logger), CheckForChanges(check_for_changes),
    CodeGenerator(logger, Indent), Inputs(inputs),
#if defined(GENERATE_ENUM_OPERATORS)
    IgnoredNamespaces(ignoredNamespaces),
#endif
    Indent(indent, ' '), Indent2(indent * 2, ' '), Indent3(indent * 3, ' ')
  {
  ParsedHeaderTypeIdsFileName = working_dir / (output_prefix + "_typeids");
  ParsedHeaderTypeIdsFileName.replace_extension(".hpp");
  InjectedFunctionsFileName = working_dir / (output_prefix + "_injected");
  InjectedFunctionsFileName.replace_extension(".cpp");
  }

AApplication::TPhaseResult TSerializableMap::Generate()
  {
  LOG_INFO("WRITE OUTPUT FILES ...");
  LOG_INFO("GENERATING " << ParsedHeaderTypeIdsFileName << " ...")
  TFileComparator hppComparator(ParsedHeaderTypeIdsFileName, CheckForChanges);
  bool changes = false;

  if (WriteParsedHeaderTypeIds() == false)
    return AApplication::TPhaseResult::ERROR;

  changes |= (hppComparator.Compare() == false);
  LOG_INFO("GENERATING " << InjectedFunctionsFileName << " ...")
  TFileComparator cppComparator(InjectedFunctionsFileName, CheckForChanges);

  if (WriteParsedHeaderInjectedFunctions() == false)
    return AApplication::TPhaseResult::ERROR;

  changes |= (cppComparator.Compare() == false);

  return changes ? AApplication::TPhaseResult::OK : AApplication::TPhaseResult::NO_CHANGES;
  }

bool TSerializableMap::WriteParsedHeaderInjectedFunctions()
  {
  if (CodeGenerator.Open(InjectedFunctionsFileName.generic_string().c_str(),
      "Defines member functions injected into serializable objects") == false)
    {
    LOG_INFO("... FAILED!");
    return false;
    }

  CodeGenerator.AddSystemInclude("cassert");
  //add macro that turns inheritace to public for serializer code
  CodeGenerator.Out << "#define SERIALIZER_INHERITANCE_SWITCH(inhType) public" << std::endl;

  for (auto& input : Inputs)
    CodeGenerator.AddInclude(input.generic_string().c_str());

  CodeGenerator.AddSystemInclude("serialize3/h/gen_code/dumpertemplates.h");
  CodeGenerator.AddSystemInclude("serialize3/h/gen_code/loadertemplates.h");
  CodeGenerator.AddInclude(ParsedHeaderTypeIdsFileName.generic_string().c_str());
  //add 'register macro' safeguard
  CodeGenerator.Out << "#ifndef REGISTER_OBJECT" << std::endl;
  CodeGenerator.Out << Indent << "#define REGISTER_OBJECT(_class_,_ptr_)" << std::endl;
  CodeGenerator.Out << "#endif" << std::endl;
  CodeGenerator.Out << "#ifndef WRAP" << std::endl;
  CodeGenerator.Out << Indent << "#define WRAP(...) __VA_ARGS__" << std::endl;
  CodeGenerator.Out << "#endif" << std::endl;

#if defined(GENERATE_ENUM_OPERATORS)
  CodeGenerator.Out << std::endl;
  WriteOperatorsForEnums();
#endif

  CodeGenerator.Out << std::endl;
  WriteFunctionsForClasses();

  CloseNamespaces();

  CodeGenerator.Close();

  bool result = (Errors == 0);

  if (result)
    LOG_INFO("... DONE")
  else
    LOG_INFO("... FAILED")

  return result;
  }

bool TSerializableMap::WriteParsedHeaderTypeIds()
  {
  for (auto _class : Classes)
    {
    if (_class->IsAbstract() == false && _class->IsPartOfHierarchy())
      _class->SetTypeId(TypeIdCounter++);
    }

  if (CodeGenerator.Open(ParsedHeaderTypeIdsFileName.generic_string().c_str(),
      "Defines type ids for classes serializable via a pointer (Note: could be directly added to injected file)") == false)
    {
    LOG_INFO("... FAILED!");
    return false;
    }

  CodeGenerator.StartHeaderSentinel();

  for (auto _class : Classes)
    {
    if (_class->IsAbstract() == false && _class->IsPartOfHierarchy())
      WriteTypeIdDeclaration(*_class);
    }

  CodeGenerator.EndHeaderSentinel();
  CodeGenerator.Close();

  bool result = (Errors == 0);

  if (result)
    LOG_INFO("... DONE")
  else
    LOG_INFO("... FAILED")

  return result;
  }

void TSerializableMap::WriteTypeIdDeclaration(const TClass& _class)
  {
  CurrentTypeIdName = GetTypeIdName(_class);

  CodeGenerator.Out << "const TTypeId " << CurrentTypeIdName << " = "
                    << _class.GetTypeId() << ';' << std::endl;
  }

#if defined(GENERATE_ENUM_OPERATORS)
void TSerializableMap::WriteOperatorsForEnums()
  {
  for (auto _enum : Enums)
    {
    const std::string& enumName = _enum->GetFullName();

    for (auto& ignoredNamespace : IgnoredNamespaces)
      {
      if (enumName.compare(0, ignoredNamespace.length(), ignoredNamespace) == 0)
        {
        _enum = nullptr;
        break;
        }
      }

    if (_enum->IsPublicAccess() == false)
      {
      LOG_VERBOSE("Enum with non-public access: " << enumName);
      _enum = nullptr;
      }

    if (_enum == nullptr)
      continue;

    const AXmlElement* parent = _enum->GetParent();

    while (parent)
      {
      switch (parent->GetElemKind())
        {
        case AXmlElement::TypeNamespace:
          parent = nullptr;
          break;
        case AXmlElement::TypeClass:
        case AXmlElement::TypeStruct:
          if (parent->IsPublicAccess() == false)
            {
            LOG_VERBOSE("Enum declared in class/struct with non-public access: " << enumName);
            parent = nullptr;
            _enum = nullptr;
            }
          else if (static_cast<const TClass*>(parent)->NeedGenerateSerializeCode() == false)
            {
            LOG_VERBOSE("Enum declared in non-serializable class/struct: " << enumName);
            parent = nullptr;
            _enum = nullptr;
            }
          else
            {
            parent = parent->GetParent();
            }
          break;
        }
      }

    if (_enum == nullptr)
      continue;

    const char* enumTypeCast = nullptr;
    std::string dumpEnumTypeCast(GetTypeEnumCast(*_enum, true));
    std::string loadEnumTypeCast(GetTypeEnumCast(*_enum, false));

    CodeGenerator.Out << "inline void operator&(ASerializeDumper& dumper, const "
                      << enumName << "& o) { dumper.Dump(" << dumpEnumTypeCast << "o); }" << std::endl;    
    CodeGenerator.Out << "inline void operator&(ASerializeLoader& loader, "
                      << enumName << "& o) { loader.Load(" << loadEnumTypeCast << "o); }" << std::endl;
    }
  }
#endif // #if defined(GENERATE_ENUM_OPERATORS)

void TSerializableMap::WriteFunctionsForClasses()
  {
  // first generate all BuildForSerializer
  for (auto _class : Classes)
    WriteBuildForSerializerMethods(*_class);

  for (auto _class : Classes)
    WriteMethodsForClass(*_class);
  }

bool TSerializableMap::AnalyzeMembers(const TClass& _class)
  {
  int errors = Errors;

  _class.ForEachMember([this](const TClassMember& member)
    {
    const TType* type = member.GetType();

start_type_check:
    if (type == nullptr)
      {
      LOG_ERROR("cannot serialize member: "
                << (member.GetName().empty() ? member.GetId() : member.GetFullName())
                << " of unknown type");
      ++Errors;
      return;
      }

    if (type->IsConst())
      {
      LOG_ERROR("cannot serialize const member: "
                << (member.GetName().empty() ? member.GetId() : member.GetFullName()));
      ++Errors;
      return;
      }

    if (member.IsBitfield())
      {
      LOG_WARNING("bitfield member: "
                  << (member.GetName().empty() ? member.GetId() : member.GetFullName())
                  << " serialized");
      }

    switch (type->GetTypeKind())
      {
      case TType::TypeClass:
      case TType::TypeStruct:
        {
        const TClass* _class = static_cast<const TClass*>(type);

        if (_class->GetName().empty() && _class->NeedGenerateSerializeCode())
          {
          LOG_ERROR("cannot serialize member: " << member.GetId()
                    << " of non-serializable and non-object-serializable unnamed class/struct type");
          ++Errors;
          }
        break;
        }

      case TType::TypeUnion:
        {
        const TClass* _class = static_cast<const TClass*>(type);

        if (_class->GetName().empty() && _class->NeedGenerateSerializeCode())
          {
          LOG_ERROR("cannot serialize member: " << member.GetId()
                    << " of non-serializable and non-object-serializable unnamed union type");
          ++Errors;
          break;
          }

        if (_class->IsDumpPointerNeeded())
          {
          LOG_ERROR("cannot serialize member: " << member.GetFullName()
                    << "; the only legal mark for union is SERIALIZABLE_OBJECT");
          ++Errors;
          break;
          }
        }
        break;

      case TType::TypeEnum:
      case TType::TypeFundamental:
        break;

      case TType::TypeArray:
        type = static_cast<const TArrayType*>(type)->GetElemType();
        goto start_type_check;
        break;

      case TType::TypePointer:
        LOG_ERROR("cannot serialize member: "
                  << (member.GetName().empty() ? member.GetId() : member.GetFullName())
                  << " of pointer type");
        ++Errors;
        break;

      case TType::TypeReference:
        LOG_ERROR("cannot serialize member: "
                  << (member.GetName().empty() ? member.GetId() : member.GetFullName())
                  << " of reference type");
        ++Errors;
        break;

      case TType::TypeCvQualified:
        LOG_ERROR("cannot serialize const member: "
                  << (member.GetName().empty() ? member.GetId() : member.GetFullName()));
        ++Errors;
        break;

      case TType::Typedef:
        throw std::runtime_error(std::string("Unexpected Typedef in ") + __FUNCTION__);

      case TType::TypeElaborated:
        throw std::runtime_error(std::string("Unexpected TypeElaborated in ") + __FUNCTION__);

      default:
        LOG_ERROR("cannot serialize member: "
                  << (member.GetName().empty() ? member.GetId() : member.GetFullName())
                  << " of unknown type");
        ++Errors;
        break;
      }
    });

  return errors == Errors;
  }

void TSerializableMap::WriteBuildForSerializerMethods(const TClass& _class)
  {
  if (_class.NeedGenerateSerializeCode() == false)
    return;

  if (_class.IsPointerSerializable())
    {
    if (_class.GetName().empty())
      {
      LOG_ERROR("cannot generate code for unnamed class/struct: " << _class.GetId());
      ++Errors;
      return;
      }

    OpenNamespaces(_class.GetNamespaceList());

    CurrentClassName = _class.GetFullNameWONamespaces();
    CurrentClassFullName = _class.GetFullName();
    CurrentTypeIdName = GetTypeIdName(_class);

    WriteBuildForSerializerFunction(_class);
    }
  }

void TSerializableMap::WriteMethodsForClass(const TClass& _class)
  {
  if (_class.NeedGenerateSerializeCode() == false)
    return;

  if (_class.IsDumpNeeded() || _class.IsLoadNeeded())
    {
    if (AnalyzeMembers(_class) == false)
      return;
    }

  if (_class.GetName().empty())
    {
    LOG_ERROR("cannot generate code for unnamed class/struct: " << _class.GetId());
    ++Errors;
    return;
    }

  if (_class.GetTypeKind() == TType::TypeUnion && _class.IsObjectSerializable())
    {
    LOG_ERROR("cannot serialize member: " << _class.GetFullName()
              << "; the only legal mark for union is SERIALIZABLE_OBJECT");
    ++Errors;
    return;
    }

  OpenNamespaces(_class.GetNamespaceList());

  CurrentClassName = _class.GetFullNameWONamespaces();
  CurrentClassFullName = _class.GetFullName();
  CurrentTypeIdName = GetTypeIdName(_class);

  WriteDumpObjectFunction(_class);
  WriteLoadObjectFunction(_class);

  if (_class.IsPointerSerializable())
    {
    WriteTypeIdFunction(_class);
    WriteDumpObjectPointerFunction(_class);
    WriteLoadObjectPointerFunction(_class);
    }
  else
    {
    LOG_NOTE(_class.GetFullName() << " is object serialized");
    }
  }

void TSerializableMap::WriteBuildForSerializerFunction(const TClass& _class)
  {
  if (_class.IsBuildPointerNeeded())
    {
    std::ofstream& out = CodeGenerator.Out;

    if (_class.IsTemplate())
      out << "template <> ";
    out << "void* " << CurrentClassName << "::BuildForSerializer()";

    if (_class.IsAbstract())
      out << " { return 0; }" << std::endl;
    else
      {
      out << std::endl;
      out << Indent << "{" << std::endl;
      out << Indent << _class.GetName() << "* ptr = new " << _class.GetName() << ";" << std::endl;
      // Enclose class name into helper WRAP macro to support template instantiations
      out << Indent << "REGISTER_OBJECT((WRAP(" << _class.GetName() << ")), ptr);" << std::endl;
      out << Indent << "return ptr;" << std::endl;
      out << Indent << "}" << std::endl;
      }
    }
  }

void TSerializableMap::WriteDumpObjectFunction(const TClass& _class)
  {
  if (_class.IsDumpNeeded())
    {
    std::ofstream& out = CodeGenerator.Out;

    if (_class.IsTemplate())
      out << "template <> ";
    out << "void " << CurrentClassName << "::Dump(ASerializeDumper& dumper) const" << std::endl;
    out << Indent << "{" << std::endl;
    out << Indent << "DPUSH_INDENT;" << std::endl;
    std::string logMsg("Dump " + CurrentClassFullName);
    CodeGenerator.AddLogMacro(logMsg.c_str(), "DLOGMSG");

    if (_class.GetTypeKind() == TType::TypeUnion)
      {
      WriteDumpInplaceUnion(_class, Indent);
      }
    else
      {
      //Dump bases:   TBase::Dump(d);
      _class.ForEachBase([this, &out](const TClass& base)
        {
        if (base.NeedGenerateSerializeCode())
          out << Indent << "dumper & static_cast<const " << base.GetFullName() << "&>(*this);" << std::endl;
        });

      //Dump fields:
      _class.ForEachMember([this](const TClassMember& member)
        {
        WriteDumpCall(member);
        });

      out << Indent << "DPOP_INDENT;" << std::endl;
      }

     out << Indent << "}" << std::endl;
    }
  }

void TSerializableMap::WriteLoadObjectFunction(const TClass& _class)
  {
  if (_class.IsLoadNeeded())
    {
    std::ofstream& out = CodeGenerator.Out;

    if (_class.IsTemplate())
      out << "template <> ";
    out << "void " << CurrentClassName << "::Load(ASerializeLoader& loader)" << std::endl;
    out << Indent << "{" << std::endl;
    out << Indent << "LPUSH_INDENT;" << std::endl;
    std::string logMsg("Load " + CurrentClassFullName);
    CodeGenerator.AddLogMacro(logMsg.c_str(),"LLOGMSG");

    if (_class.GetTypeKind() == TType::TypeUnion)
      {
      WriteLoadInplaceUnion(_class, Indent);
      }
    else
      {
      //Load bases:   Load((TBase&)o);
      _class.ForEachBase([this, &out](const TClass& base)
        {
        if (base.NeedGenerateSerializeCode())
          out << Indent << "loader & static_cast<" << base.GetFullName() << "&>(*this);" << std::endl;
        });

      //Load fields:   Load(o.F1); or Load((int&)o.F1); if enum type
      _class.ForEachMember([this](const TClassMember& member)
        {
        WriteLoadCall(member);
        });

      out << Indent << "LPOP_INDENT;" << std::endl;
      }

    out << Indent << "}" << std::endl;
    }
  }

void TSerializableMap::WriteTypeIdFunction(const TClass& _class)
  {
  std::ofstream& out = CodeGenerator.Out;

  //TYPEID_TYPE ClassName::GetTypeId() const { return ClassName_TYPE_ID; }
  if (_class.IsTemplate())
    out << "template <> ";
  out << "TTypeId " << CurrentClassName << "::GetTypeId() const { return "
      << CurrentTypeIdName << "; }" << std::endl;
  }

void TSerializableMap::WriteDumpObjectPointerFunction(const TClass& _class)
  {
  if (_class.IsDumpPointerNeeded())
    {
    std::ofstream& out = CodeGenerator.Out;

    if (_class.IsTemplate())
      out << "template <> ";
    out << "void " << CurrentClassName << "::DumpPointer(ASerializeDumper& dumper) const" << std::endl;
    out << Indent << "{" << std::endl;
    std::string logMsg("Dump " + CurrentClassFullName + " pointer");
    CodeGenerator.AddLogMacro(logMsg.c_str(),"DLOGMSG");
    out << Indent << "dumper.Dump(GetTypeId());" << std::endl;
    out << Indent << "dumper & *this;" << std::endl;
    out << Indent << "}" << std::endl;
    }
  }

void TSerializableMap::WriteLoadObjectPointerFunction(const TClass& _class)
  {
  if (_class.IsLoadPointerNeeded())
    {
    std::ofstream& out = CodeGenerator.Out;

    if (_class.IsTemplate())
      out << "template <> ";
    out << "void* " << CurrentClassName << "::LoadPointer(ASerializeLoader& loader)" << std::endl;
    out << Indent << "{" << std::endl;
    std::string logMsg("Load " + CurrentClassFullName + " pointer");
    CodeGenerator.AddLogMacro(logMsg.c_str(), "LLOGMSG");
    out << Indent << "TTypeId objectTypeId;" << std::endl
        << Indent << "loader.Load(objectTypeId);" << std::endl
        << Indent << CurrentClassFullName << "* o;" << std::endl;
    out << Indent << "switch(objectTypeId)" << std::endl;
    out << Indent2 << "{" << std::endl;
    out << Indent2 << "case NULL_TYPE_ID:" << std::endl;
    out << Indent3 << "o = 0;" << std::endl;
    out << Indent3 << "break;" << std::endl;

    TClassSet classCasesWritten; //keeps track of cases written to avoid duplicate cases
    WriteCasesForDerived(_class, classCasesWritten);

    out << Indent2 << "default:" << std::endl;
    out << Indent3 << "assert(false);" << std::endl;
    out << Indent3 << "o = 0;" << std::endl;
    out << Indent3 << "throw 1;" << std::endl;
    out << Indent2 << "} //end switch" << std::endl;
    out << Indent << "return o;" << std::endl;
    out << Indent << "} //end LoadPointer" << std::endl;
    }
  }

void TSerializableMap::WriteTypeIdCase(const TClass& _class)
  {
  std::ofstream& out = CodeGenerator.Out;
  std::string fullName(_class.GetFullName());

  out << Indent2 << "case " << GetTypeIdName(_class) << ":" << std::endl;

  if (_class.IsVirtuallyDerived())
    {
    out << Indent3 << "{" << std::endl;
    out << Indent3 << fullName << "* d = (" << fullName << "*)" << fullName;
    if (_class.IsBuildForSerializerWithLoader())
      out << "::BuildForSerializer(loader);";
    else
      out << "::BuildForSerializer();";
    out << std::endl;
    out << Indent3 << "loader & *d;" << std::endl;
    out << Indent3 << "o = d;" << std::endl;
    out << Indent3 << "}" << std::endl;
    }
  else
    {
    out    << Indent3 << "o = (" << fullName << "*)" << fullName;
    if (_class.IsBuildForSerializerWithLoader())
      out << "::BuildForSerializer(loader);";
    else
      out << "::BuildForSerializer();";
    out << std::endl;
    out << Indent3 << "loader & (" << fullName << "&)*o;" << std::endl;
    }
  out << Indent3 << "break;" << std::endl;
  }

void TSerializableMap::WriteCasesForDerived(const TClass& _class, TClassSet& classCasesWritten)
  {
  if (_class.NeedGenerateSerializeCode() == false)
    return;

  auto insertInfo = classCasesWritten.insert(&_class);

  //Avoid duplicate case if already written (can happen in multiple-base hierarchies)
  if (insertInfo.second == false)
    return;

  if (_class.IsAbstract() == false)
    WriteTypeIdCase(_class);

  //case for each derived type
  _class.ForEachDerived([this,&classCasesWritten](const TClass& derived)
    {
    WriteCasesForDerived(derived, classCasesWritten);
    });
  }

bool TSerializableMap::HandleArray(std::ofstream& out, std::string& indent, std::string iterator,
  std::string& reference, const TArrayType& arrayType, const TType** type)
  {
  int size = arrayType.GetSize();
  out << indent << "for (int " << iterator << " = 0; " << iterator << " < " << size
    << "; ++" << iterator << ")" << std::endl;
  reference += '[' + iterator + ']';
  indent += Indent;

  const TType* elemType = arrayType.GetElemType();

  if (elemType->GetTypeKind() == TType::TypeArray)
    {
    iterator += 'i';
    const TArrayType* arrayType = static_cast<const TArrayType*>(elemType);

    if (HandleArray(out, indent, iterator, reference, *arrayType, type) == false)
      return false;
    }

  return true;
  }

template <bool DUMP_CALL>
void TSerializableMap::WriteCall(const TClassMember& member, const std::string& prefix)
  {
  std::ofstream& out = CodeGenerator.Out;
  const TType* type = member.GetType();
  assert(type == nullptr || type->GetTypeKind() != TType::Typedef);
  std::string indent(Indent);
  std::string reference(prefix + member.GetName());
  bool is_array = false;
  const char* dumper_loader = DUMP_CALL ? "dumper & " : "loader & ";

  if (type)
    {
    TType::TTypeKind typeKind = type->GetTypeKind();

    if (typeKind == TType::TypeArray)
      {
      std::string iterator("i");
      const TArrayType* arrayType = static_cast<const TArrayType*>(type);

      if (HandleArray(out, indent, iterator, reference, *arrayType, &type) == false)
        return;

      is_array = true;
      }

    if (DUMP_CALL == false && member.IsBitfield())
      {
      out << indent << '{' << std::endl;
      out << indent << type->GetFullName() << " dummy;" << std::endl;
      out << indent << "loader & dummy;" << std::endl;
      out << indent << reference << " = dummy;" << std::endl;
      out << indent << '}' << std::endl;
      return;
      }
    }

  if (type->GetName().empty() == false)
    {
    switch (type->GetTypeKind())
      {
      case TType::TypeClass:
      case TType::TypeStruct:
      case TType::TypeUnion:
        if (static_cast<const TClass*>(type)->IsSerializable() == TYPE_DO_NOT_SERIALIZE)
          return;

      default:
        break;
      }

    // class, struct, enum, fundamental
    out << indent << dumper_loader << reference << ";" << std::endl;
    return;
    }

  const TClass* _class = static_cast<const TClass*>(type);

  if (is_array)
    out << indent << '{' << std::endl;

  switch (type->GetTypeKind())
    {
    case TType::TypeClass:
    case TType::TypeStruct:
      WriteInplaceStruct<DUMP_CALL>(*_class, reference.empty() ? reference : (reference + '.'));
      break;

    case TType::TypeUnion:
      WriteInplaceUnion<DUMP_CALL>(*_class, reference.empty() ? reference : (reference + '.'));
      break;

    default:
      LOG_ERROR("cannot generate code for member: " << member.GetName() << " of unnamed type");
      ++Errors;
      break;
    }

  if (is_array)
    out << indent << '}' << std::endl;
  }

template <bool DUMP_LOAD> // true for DUMP, false for LOAD
void TSerializableMap::WriteInplaceStruct(const TClass& _class, const std::string& prefix)
  {
  if (_class.IsSerializable() == TYPE_DO_NOT_SERIALIZE)
    return;

  _class.ForEachBase([this, _class, prefix] (const TClass& base)
    {
    if (base.IsSerializable() == TYPE_DO_NOT_SERIALIZE)
      return;

    if (base.IsSerializable() == TYPE_NOT_MARKED)
      {
      LOG_ERROR("cannot generate code for inplace struct with non-serializable base: "
                << (_class.GetName().empty() ? _class.GetId() : _class.GetFullName()));
      ++Errors;
      return;
      }

    if (prefix.empty())
      {
      LOG_ERROR("cannot generate code for inplace unnamed struct with base: "
                << (_class.GetName().empty() ? _class.GetId() : _class.GetFullName()));
      ++Errors;
      return;
      }

    std::string name(prefix);
    name.pop_back();

    const char* loader_dumper =
      DUMP_LOAD ? "dumper & static_cast<const " : "loader & static_cast<";

    CodeGenerator.Out << Indent << loader_dumper << base.GetFullName() << "&>(" << name << ");" << std::endl;
    });

  _class.ForEachMember([this, prefix](const TClassMember& member)
    {
    if (member.IsPublicAccess() == false)
      {
      LOG_ERROR("cannot generate code for non-public struct member: "
                << (member.GetName().empty() ? member.GetId() : member.GetFullName()));
      ++Errors;
      return;
      }

    WriteCall<DUMP_LOAD>(member, prefix);
    });
  }

template <bool DUMP_LOAD> // true for DUMP, false for LOAD
void TSerializableMap::WriteInplaceUnion(const TClass& _class, const std::string& prefix)
  {
  if (_class.IsSerializable() == TYPE_DO_NOT_SERIALIZE)
    return;

  typedef std::pair<int, const TClassMember*> TMemberInfo;
  std::pair<int, const TClassMember*> biggest {0, nullptr};

  _class.ForEachMember([this, &biggest](const TClassMember& member)
    {
    if (member.IsPublicAccess() == false)
      {
      LOG_ERROR("cannot generate code for non-public union member: "
                << (member.GetName().empty() ? member.GetId() : member.GetFullName()));
      ++Errors;
      return;
      }

    const TType* type = member.GetType();
    assert(type != nullptr);
    int size = std::max(type->GetSizeof(), member.IsBitfield());
    biggest = std::max(biggest, std::make_pair(size, &member), [](const TMemberInfo& m1, const TMemberInfo& m2)
      {
      return m1.first < m2.first;
      });
    });

  if (biggest.second)
    WriteCall<DUMP_LOAD>(*biggest.second);
  }

#if defined(GENERATE_ENUM_OPERATORS)
std::string TSerializableMap::GetTypeEnumCast(const TEnum& _enum, bool dump)
  {
  std::string result(dump ? "(const " : "(");

  switch (_enum.GetSizeof())
    {
    case 8:
      result += "unsigned char&)"; break;
    case 16:
      result += "unsigned short&)"; break;
    case 0: // default sizeof
    case 32:
      result += "unsigned int&)"; break;
    case 64:
      result += "unsigned long long&)"; break;
    default:
      LOG_ERROR("too big Enum " << _enum.GetName() << " sizeof: " << _enum.GetSizeof());
      ++Errors;
      result.clear();
      break;
    }

  return std::move(result);
  }
#endif // #if defined(GENERATE_ENUM_OPERATORS)

void TSerializableMap::OpenNamespaces(TNamespaces&& namespaces)
  {
  if (CurrentOpenedNamespaces != namespaces)
    {
    CloseNamespaces();
    for (auto& _namespace : namespaces)
      CodeGenerator.Out << "namespace " << _namespace->GetName() << std::endl << '{' << std::endl;
    CurrentOpenedNamespaces = std::move(namespaces);
    }
  }

void TSerializableMap::CloseNamespaces()
  {
  for (auto& _namespace : CurrentOpenedNamespaces)
    CodeGenerator.Out << "} // namespace " << _namespace->GetName() << std::endl;

  CurrentOpenedNamespaces.clear();
  }

std::string TSerializableMap::GetTypeIdName(const TClass& _class)
  {
  if (_class.IsAbstract() || _class.IsPartOfHierarchy() == false)
    return "-1";

  std::string result(_class.GetFullName());
  const char* illegal_chars = ":<>, *";

  for (size_t pos = result.find_first_of(illegal_chars, 0); pos != -1;
       pos = result.find_first_of(illegal_chars, pos))
    {
    result[pos] = '_';
    }

  return result + "_TYPE_ID";
  }
