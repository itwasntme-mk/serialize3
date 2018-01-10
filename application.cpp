///\file application.cpp

#define _CRT_SECURE_NO_WARNINGS

#include "application.h"
#include "xml_element.h"
#include "constants.h"
#include "xml_reader_boost_property_tree.h"
#include "serializablemap.h"
#include "external_app_launcher.h"
#include "file_comparator.h"

#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>
#include <boost/scope_exit.hpp>
#include <boost/tokenizer.hpp>

#include <string.h>
#include <iostream>
#include <vector>
#include <set>
#include <map>

#if defined(_WIN32)
  #define strcasecmp _strcmpi
#endif


#define GET_ID_STR(id) bpt::AXmlItemWrapper::GetIdStr(id)

namespace bpo = boost::program_options;
namespace bfs = boost::filesystem;
namespace bpt = boost::property_tree;

#if defined(USE_SORTED_CLASSES)
inline void AApplication::OrderClass(const TClass& _class)
  {
  using namespace std::placeholders;

  if (_class.NeedGenerateSerializeCode() && _class.IsOrdered() == false)
    _class.ForEachDerived(std::bind(OrderClass, _1));
  }
#endif

int AApplication::Run(const bpo::variables_map& args)
  {
  std::string stop_when_no_changes;
  path logfile;
  TPhaseResult result = TPhaseResult::OK;

  if (args.count("check-changes"))
    stop_when_no_changes = args["check-changes"].as<std::string>();

  if (args.count("log"))
    {
    logfile = args["log"].as<std::string>();
    if (args.count("-d") && logfile.has_parent_path() == false)
      logfile = args["-d"].as<path>() / logfile;
    }

  Logger.Open(logfile.generic_string(), args.count("verbose") != 0, args.count("quiet") != 0);

  if (Initialize(args) == false)
    return -1;

  result = Preprocess(stop_when_no_changes == "preprocess");

  if (result == TPhaseResult::ERROR)
    return -2;
  if (result == TPhaseResult::NO_CHANGES)
    {
    LOG_INFO("FINISHED (no changes after preprocessing)");
    return 0;
    }

  result = GenerateXML(stop_when_no_changes == "xml");    
  if (result == TPhaseResult::ERROR)
    return -3;
  if (result == TPhaseResult::NO_CHANGES)
    {
    LOG_INFO("FINISHED (no changes after xml generation)");
    return 0;
    }

  BOOST_SCOPE_EXIT(this_)
    {
    TXmlElementsFactory* xmlElementsFactory = TXmlElementsFactory::GetInstance();
    xmlElementsFactory->DeleteElements();
    } BOOST_SCOPE_EXIT_END

  if (ParseXML() == false)
    return -4;

  result = GenerateSerializationCode(args);
  if (result == TPhaseResult::ERROR)
    return -5;
  if (result == TPhaseResult::NO_CHANGES)
    {
    LOG_INFO("FINISHED (no changes after in generated hpp/cpp files)");
    return 0;
    }

  LOG_INFO("FINISHED");

  return 0;
  }

bool AApplication::Initialize(const bpo::variables_map& args)
  {
  WorkingDir = args["-d"].as<path>();
  XmlCreatorPath = args["xml-generator-path"].as<std::string>();
  XmlCreatorOptions = args["xml-generator-options"].as<std::string>();
  Compiler = args["compiler"].as<std::string>();
  CompilerPath = args["compiler-path"].as<std::string>();
  CompilerOptions = args["compiler-options"].as<std::string>();
  CompilerIncludes = args["compiler-includes"].as<std::string>();
  InputFiles = args["input"].as<std::vector<path>>();

  if (WorkingDir.empty() == false)
    {
    for (auto& file : InputFiles)
      {
      if (file.has_parent_path() == false)
        file = WorkingDir / file;
      }
    }

  for (auto& file : InputFiles)
    {
    if (bfs::exists(file) == false)
      {
      LOG_ERROR("file " << file.generic_string() << " not exists");
      return false;
      }
    }

  PreprocessedFile = InputFiles[0];

  if (PreprocessedFile.has_parent_path() == false)
    PreprocessedFile = WorkingDir / PreprocessedFile;

  if (XmlCreatorOptions.find("c++") != 0)
    PreprocessedFile.replace_extension(".ii");
  else
    PreprocessedFile.replace_extension(".i");

  XmlFile = PreprocessedFile;
  XmlFile.replace_extension(".xml");

  if (args.count("output"))
    OutputFilePrefix = args["output"].as<std::string>();
  else
    OutputFilePrefix = XmlFile.stem().generic_string();

  return true;
  }

std::string AApplication::ConfigureGenericCompiler() const
  {
  return " --preprocess -o " + PreprocessedFile.generic_string();
  }

std::string AApplication::ConfigureClang() const
  {
  return " --preprocess -o " + PreprocessedFile.generic_string();
  }

std::string AApplication::ConfigureGcc() const
  {
  return " --preprocess -o " + PreprocessedFile.generic_string();
  }

std::string AApplication::ConfigureMsvc() const
  {
  return " -showIncludes -P -TP -Fi: " + PreprocessedFile.generic_string();
  }

std::string AApplication::PrepareIncludes() const
  {
  if (CompilerIncludes.empty())
    return "";

  std::string result;
  result.reserve(CompilerIncludes.size() * 2);
  boost::tokenizer<boost::char_separator<char>>
    tokenizer(CompilerIncludes, boost::char_separator<char>(";"));
  
  for (auto& include : tokenizer)
    {
    if (include.empty() == false)
      result += " -I \"" + include + "\"";
    }

  return std::move(result);
  }

void AApplication::RewriteFileToLog(const path& filename)
  {
  if (Logger.IsQuiet())
    return;

  std::ifstream file(filename.generic_string());

  if (file.good() == false)
    {
    LOG_ERROR("cannot open log file: " << filename);
    return;
    }

  LOG_INFO(file.rdbuf());

  file.close();
  bfs::remove(filename);
  }

AApplication::TPhaseResult AApplication::Preprocess(bool check_for_changes)
  {
  LOG_INFO("PREPROCESSING ...");

  TFileComparator fileComparator(PreprocessedFile, check_for_changes);
  path compiler(Compiler);
  std::string compilerName(compiler.stem().generic_string());

#if defined(_WIN32)
  std::transform(compilerName.begin(), compilerName.end(), compilerName.begin(), [] (unsigned char c)
    {
    return std::tolower(c);
    });
#endif

  enum { GENERIC, GCC, CLANG, MSVC };
  int compilerType = GENERIC;

  if (compilerName.find("gcc") == 0 || compilerName.find("g++") == 0)
    compilerType = GCC;
  else if (compilerName.find("clang") == 0)
    compilerType = CLANG;
  else if (compilerName.find("cl") == 0 || compilerName.find("msvc") == 0)
    {
    compilerType = MSVC;
    compiler = compiler.parent_path() / "cl";
    }
  else // GENERIC
    {
    LOG_WARNING("Uknown compiler: " << compilerName);
    }

  path compilerFullName;

  if (compiler.has_parent_path())
    compilerFullName = compiler;
  else
    compilerFullName = CompilerPath / compiler;

#if defined(_WIN32)
  std::string command('\"' + compilerFullName.string() + '\"');
#else
  std::string command(compilerFullName.string());
#endif

  command += ' ' + CompilerOptions;

  switch (compilerType)
  {
  case GENERIC:
    command += ConfigureGenericCompiler();
    break;
  case GCC:
    command += ConfigureGcc();
    break;
  case CLANG:
    command += ConfigureClang();
    break;
  case MSVC:
    command += ConfigureMsvc();
    break;
  default: assert(false && "Uknown compiler type!");
    break;
  }

  command += ' ' + PrepareIncludes();

  for (auto& input : InputFiles)
    command += ' ' + input.generic_string();

  LOG_VERBOSE("Run cmd: " << command);

  path logFileName(WorkingDir / "preprocessing.log");
  std::string errorString;
  bool result =
    LaunchExternalConsoleProcess(command.c_str(), logFileName.generic_string().c_str(), errorString) &&
    bfs::exists(PreprocessedFile);

  if (result)
    {
    bfs::remove(logFileName);
    LOG_INFO("... DONE");
    return fileComparator.Compare() ? TPhaseResult::NO_CHANGES : TPhaseResult::OK;
    }
  else
    {
    if (errorString.empty() == false)
      LOG_ERROR(errorString);
    RewriteFileToLog(logFileName);
    LOG_INFO("... FAILED");
    return TPhaseResult::ERROR;
    }
  }

AApplication::TPhaseResult AApplication::GenerateXML(bool check_for_changes)
  {
  TFileComparator fileComparator(XmlFile, check_for_changes);
  std::string command(ComposeXmlGeneratorCmdLine());

  LOG_VERBOSE("Run cmd: " << command);

  path logFileName(WorkingDir / "xml-generator.log");
  std::string errorString;
  bool result =
    LaunchExternalConsoleProcess(command.c_str(), logFileName.generic_string().c_str(), errorString) &&
    bfs::exists(XmlFile);

  if (result)
    {
    bfs::remove(logFileName);
    LOG_INFO("... DONE");
    return fileComparator.Compare() ? TPhaseResult::NO_CHANGES : TPhaseResult::OK;
    }
  else
    {
    if (errorString.empty() == false)
      LOG_ERROR(errorString);
    RewriteFileToLog(logFileName);
    LOG_INFO("... FAILED");
    return TPhaseResult::ERROR;
    }
  }

bool AApplication::ParseXML()
  {
  bpt::ptree pt;

  LOG_INFO("READ XML FILE ...");

  try
    {
    bpt::read_xml(XmlFile.generic_string(), pt);
    }
  catch (const boost::exception& ex)
    {
    LOG_INFO("... FAILED");
    LOG_ERROR(boost::diagnostic_information(ex));
    LOG_ERROR("error while trying to parse " << XmlFile << " file");
    return false;
    }

  LOG_INFO("... DONE")
  LOG_INFO("ANALYZE XML LIST ...");
  const char* xmlRootName = GetXmlRootName();
  const bpt::ptree* content = nullptr;

  try
    {
    content = &pt.get_child(xmlRootName);
    }
  catch (const boost::exception& ex)
    {
    LOG_INFO("... FAILED");
    LOG_ERROR(boost::diagnostic_information(ex));
    LOG_ERROR("cannot find root " << xmlRootName);
    return false;
    }

  // class+baseClasses
  std::vector<std::pair<int, TIdCntr>> baseClasses;
  std::vector<TClass*> classes;
  // typedef type1 type2; // pair: type2+type1
  std::map<int, int> typedefs;
  // member+type
  std::vector<std::pair<TClassMember*, int>> members;
  std::set<int> virtuallyInheritedClasses;
  TXmlElementsFactory* xmlElementsFactory = TXmlElementsFactory::GetInstance();

  // pair of element+its parent
  int xmlSize = static_cast<int>(content->size());
  std::unique_ptr<AXmlElement*[]> elements(new AXmlElement*[xmlSize]);
  memset(elements.get(), 0, xmlSize * sizeof(AXmlElement*));

  bpt::AXmlItemWrapper::ReserveIdPool(xmlSize);

  typedef std::function<void(const bpt::ptree::value_type&, TTagType)> THandleXmlItem;
  typedef std::map<TTagType, THandleXmlItem, std::str_less> THandleXmlItemIndex;

  THandleXmlItem handle_field = [&] (const bpt::ptree::value_type& item, TTagType tag)
    {
    bpt::TFieldWrapper wrapper(item);
    auto const& name = wrapper.GetName();
    int id = wrapper.GetId();
    int typeId = wrapper.GetTypeId();
    int context = wrapper.GetContextId();
    assert(context >= 0);
    bool publicAccess = wrapper.IsPublicAccess();
    int bitfield = wrapper.IsBitfield();
    const std::string& idStr = GET_ID_STR(id);
    TClassMember* member = xmlElementsFactory->CreateClassMember(name, idStr, publicAccess, bitfield);
    member->SetParent(elements[context]);
    static_cast<TClass*>(elements[context])->AddMember(member);

    members.emplace_back(member, typeId);
    elements[id] = member;
    };

  THandleXmlItem handle_namespace = [&] (const bpt::ptree::value_type& item, TTagType tag)
    {
    bpt::TNamespaceWrapper wrapper(item);
    auto const& name = wrapper.GetName();

    if (name == "::")
      return;

    int id = wrapper.GetId();
    int context = wrapper.GetContextId();
    const std::string& idStr = GET_ID_STR(id);
    TNamespace* _namespace = xmlElementsFactory->CreateNamespace(name, idStr);
    if (context >= 0)
      _namespace->SetParent(elements[context]);

    elements[id] = _namespace;
    };

  THandleXmlItem handle_enum_type = [&] (const bpt::ptree::value_type& item, TTagType tag)
    {
    bpt::TEnumWrapper wrapper(item);
    auto const& name = wrapper.GetName();
    int id = wrapper.GetId();

    if (name.empty() || name.compare(0, 2, "$_", 2) == 0)
      {
#if defined(GENERATE_ENUM_OPERATORS)
    LOG_VERBOSE("Unnamed Enum skipped (id: " << wrapper.GetIdStr() << ")");
#endif
      return;
      }

    int _sizeof = wrapper.GetSizeof();
    int context = wrapper.GetContextId();
    assert(context >= 0);
    bool publicAccess = wrapper.IsPublicAccess();
    const std::string& idStr = GET_ID_STR(id);
    TEnum* _enum = xmlElementsFactory->CreateEnum(name, idStr, tag, publicAccess, _sizeof);
    _enum->SetParent(elements[context]);

    //if (wrapper.IsPublicAccess() == false)
      //LOG_WARNING("Enum with non-public access (id: " << wrapper.GetIdStr() << ")");

    elements[id] = _enum;
    Enums.push_back(_enum);
    };

  THandleXmlItem handle_class = [&] (const bpt::ptree::value_type& item, TTagType tag)
    {
    bpt::TClassWrapper wrapper(item);
    auto const& name = wrapper.GetName();

    if (wrapper.IsIncomplete())
      {
      LOG_VERBOSE("Incomplete class skipped (id: " << wrapper.GetIdStr() << ", name: " << name << ')');
      return;
      }

    int id = wrapper.GetId();
    auto bases = wrapper.GetBases();
    auto members = wrapper.GetMembers();
    int context = wrapper.GetContextId();
    assert(context >= 0);
    bool publicAccess = wrapper.IsPublicAccess();
    const std::string& idStr = GET_ID_STR(id);
    TClass* _class = xmlElementsFactory->CreateClass(name, idStr, tag, publicAccess,
                                                     bases.size(), members.size());

    elements[id] = _class;
    classes.push_back(_class);
    _class->SetParent(elements[context]);

    if (wrapper.IsAbstract())
      _class->SetAbstract();

    if (bases.empty() == false)
      {
      baseClasses.emplace_back(id, std::move(bases));

      auto attrI = item.second.begin();
      ++attrI;
      auto endI = item.second.end();
      assert(attrI != endI);
      for (; attrI != endI; ++attrI)
        {
        if (attrI->first == TAG_BASE)
          {
          bpt::TBaseClassWrapper wrapper(*attrI);

          if (wrapper.IsVirtuallyDerived())
            {
            int typeId = wrapper.GetTypeId();
            virtuallyInheritedClasses.insert(typeId);
            }
          }
        }
      }
    };

  THandleXmlItem handle_type = [&] (const bpt::ptree::value_type& item, TTagType tag)
    {
    bpt::TTypeWrapper wrapper(item);

    auto const& name = wrapper.GetName();
    int id = wrapper.GetId();
    int typeId = wrapper.GetTypeId();
    bool _const = wrapper.IsConst();
    TType* _type = nullptr;
    bool publicAccess = wrapper.IsPublicAccess();
    int _sizeof = wrapper.GetSizeof();
    const std::string& idStr = GET_ID_STR(id);

    if (tag != TAG_ARRAY_TYPE)
      _type = xmlElementsFactory->CreateType(name, idStr, tag, publicAccess, _sizeof);
    else
      {
      int _min, _max;
      if (wrapper.GetMin(_min) == false || wrapper.GetMax(_max) == false)
        {
        LOG_VERBOSE("Array type with invalid min/max skipped (id: " << idStr << " name: " << name << ')');
        return;
        }
      int size = _min > _max ? 0 : _max - _min + 1;
      _type = xmlElementsFactory->CreateArrayType(name, idStr, tag, publicAccess, size);
      }

    int context = wrapper.GetContextId();
    if (context >= 0)
      _type->SetParent(elements[context]);

    elements[id] = _type;

    if (typeId != -1)
      typedefs.emplace(id, typeId);
    };

  THandleXmlItem handle_method = [&] (const bpt::ptree::value_type& item, TTagType tag)
    {
    bpt::TMethodWrapper wrapper(item);
    TMethodType methodType = wrapper.IsSerializeMethod();

    if (methodType)
      {
      int context = wrapper.GetContextId();
      assert(context >= 0);
      static_cast<TClass*>(elements[context])->SetSerializeMethod(methodType);
      }
    };

  const THandleXmlItemIndex handleXmlItemIndex =
    {
      { TAG_FIELD, handle_field },
      { TAG_NAMESPACE, handle_namespace },
      { TAG_ENUM_TYPE, handle_enum_type },
      { TAG_CLASS, handle_class },
      { TAG_STRUCT, handle_class },
      { TAG_UNION, handle_class },
      { TAG_ARRAY_TYPE, handle_type },
      { TAG_FUNDAMENTAL_TYPE, handle_type },
      { TAG_TYPEDEF, handle_type },
      { TAG_POINTER_TYPE, handle_type },
      { TAG_REFERENCE_TYPE, handle_type },
      { TAG_CV_QUALIFIED_TYPE, handle_type },
      { TAG_ELABORATED_TYPE, handle_type },
      { TAG_METHOD, handle_method }
    };

  // main loop
  try
    {
    for (auto const& item : *content)
      {
      if (item.first.empty())
        continue;

      auto found = handleXmlItemIndex.find(item.first.c_str());

      if (found != handleXmlItemIndex.end())
        found->second(item, found->first);
      }
    }
  catch (const boost::exception& ex)
    {
    LOG_INFO("... FAILED");
    LOG_ERROR(boost::diagnostic_information(ex));
    return false;
    }

  bool error = false;

  // touch virtually derived classes
  for (auto id : virtuallyInheritedClasses)
    static_cast<TClass*>(elements[id])->SetVirtuallyDerived();

  // typedef type1 type2; we want to type2 point to type1 and type1 replace type2 in elements table
  std::function<TType*(int)> get_origin_type = [&] (int typeId)
    {
    TType* type1 = static_cast<TType*>(elements[typeId]);

    if (type1 == nullptr || type1->IsTypedef() == false)
      return type1;

    auto found = typedefs.find(typeId);
    assert(found != typedefs.end());
    typeId = found->second;
    type1 = get_origin_type(typeId);
    elements[typeId] = type1;
    return type1;
    };

  /*for (auto& data : typedefs)
    {
    TType* type2 = static_cast<TType*>(elements[data.first]);
    TType* type1 = static_cast<TType*>(elements[data.second]);

    std::cout << "typedef " << (type1 ? type1->GetName() : "<null>") << ' ' << type2->GetName() << std::endl;
    }*/

  for (auto& data : typedefs)
    {
    // typedef type1 type2;
    TType* type2 = static_cast<TType*>(elements[data.first]);

    if (type2 && type2->IsTypedef())
      {
      TType* type1 = get_origin_type(data.second);
      elements[data.first] = type1;
      }
    }

  /*for (auto& data : typedefs)
    {
    TType* type2 = static_cast<TType*>(elements[data.first]);
    TType* type1 = static_cast<TType*>(elements[data.second]);

    std::cout << "typedef " << (type1 ? type1->GetName() : "<null>") << ' ' << type2->GetName() << std::endl;
    }*/

#if defined(_DEBUG)
  for (auto& data : typedefs)
    {
    TType* type1 = static_cast<TType*>(elements[data.second]);
    assert(type1 == nullptr || type1->IsTypedef() == false);
    }
#endif // #if defined(_DEBUG)

  // complete type connections
  for (auto& data : typedefs)
    {
    TType* type1 = static_cast<TType*>(elements[data.second]);
    TType* type2 = static_cast<TType*>(elements[data.first]);

    if (type2 != nullptr && type1 != type2)
      type2->SetPointedType(type1);
    }

  // to be sure buffers are released, not only cleared
  typedefs = decltype(typedefs)();

  // complete members' types
  for (auto& data : members)
    data.first->SetType(static_cast<TType*>(elements[data.second]));

  // to be sure buffers are released, not only cleared
  members = decltype(members)();

  // check if serializable condition passed between base & derived classes
  for (auto& data : baseClasses)
    {
    int id = data.first;
    TClass* _class = static_cast<TClass*>(elements[id]);
    TMethodType serializableType = _class->GetSerializableMarker();

    for (auto const& base : data.second)
      {
      TClass* baseClass = static_cast<TClass*>(elements[base]);

      if (baseClass == nullptr)
        {
        LOG_VERBOSE("Base class (id: " << GET_ID_STR(base) << ") not exists");
        continue;
        }

      baseClass->AddDerived(_class);
      _class->AddBase(baseClass);
      /*TMethodType baseSerializableType = baseClass->GetSerializableMarker();

      if (baseSerializableType && baseClass->IsSerializable() != TYPE_DO_NOT_SERIALIZE &&
          serializableType == nullptr)
        {
        LOG_WARNING("base class " << baseClass->GetFullName() <<
                    " (id: " << GET_ID_STR(base) << ") is serializable, but derived class " <<
                    _class->GetFullName() << " (id: " << GET_ID_STR(id) <<
                    ") is not (possible runtime errors)");
        }
      else if (baseSerializableType == nullptr && serializableType &&
               _class->IsSerializable() != TYPE_DO_NOT_SERIALIZE)
        {
        LOG_ERROR("base class " << baseClass->GetFullName() << " (id: " << GET_ID_STR(base) <<
                  ") is not serializable, but derived class " << _class->GetFullName() <<
                  " (id: " << GET_ID_STR(id) << ") is serializable (generated code could not compile)");
        error = true;
        }*/
      }
    }

  // to be sure buffers are released, not only cleared
  baseClasses = decltype(baseClasses)();

#if defined(USE_SORTED_CLASSES)
  // sort classes: base as first (at front of fwd list)
  for (auto& _class : classes)
    OrderClass(*_class);
#else
  for (auto& _class : classes)
    {
    TSerializeMethod method = _class->IsSerializable();

    if (_class->GetElemKind() == AXmlElement::TypeUnion)
      _class->ChooseBiggestMember();

    if (method != TYPE_NOT_MARKED && method != TYPE_MANUAL_OBJECT_SERIALIZE)
      SerializableClasses.push_back(_class);
    }
#endif

#if defined(_DEBUG)

  for (auto& data : Enums)
    std::cout << "Enum " << data->GetFullName() << " (sizeof: " << data->GetSizeof() << ")" << std::endl;

  //for (auto& _class : classes)
  for (auto& _class : SerializableClasses)
    {
    TMethodType serializable = _class->GetSerializableMarker();
    std::cout << "Class: " << _class->GetFullName();

    if (serializable)
      std::cout << " (serialization: " << serializable << ')';
    
    std::cout << std::endl;

    std::cout << "  Derived:";
    _class->ForEachDerived([](const TClass& base_class)
      {
      std::cout << ' ' << base_class.GetFullName();
      });
    std::cout << std::endl;

    std::cout << "  Members:";
    _class->ForEachMember([](const TClassMember& member)
      {
      std::cout << ' ' << member.GetName();
      });
    std::cout << std::endl;
    }

#endif // #if defined(_DEBUG)

  if (error == false)
    LOG_INFO("... DONE")
  else
    LOG_INFO("... FAILED");

  return (error == false);
  }

AApplication::TPhaseResult AApplication::GenerateSerializationCode(const boost::program_options::variables_map& args)
  {
  std::vector<std::string> ignoredNamespaces;
  bool check_for_changes = (args.count("stop-when-no-changes") && args["stop-when-no-changes"].as<std::string>() == "xml");

  if (args.count("ignore-namespace"))
    ignoredNamespaces = args["ignore-namespace"].as<std::vector<std::string>>();

  TSerializableMap serializableMap(SerializableClasses, Enums, Logger, InputFiles,
    WorkingDir, OutputFilePrefix,
#if defined(GENERATE_ENUM_OPERATORS)
    ignoredNamespaces,
#endif
    args["indent"].as<int>(), check_for_changes);

  return serializableMap.Generate();
  }

bool AApplication::SelfRegister(const std::string& name, TApplicationsFactory::TCreateApplication creator)
  {
  TApplicationsFactory* appsFactory = TApplicationsFactory::GetInstance();
  appsFactory->RegisterApplication(name, creator);
  return true;
  }
