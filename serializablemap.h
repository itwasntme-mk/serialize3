///\file serializablemap.h
#pragma once

#include "constants.h"
#include "logger.h"
#include "xml_element.h"
#include "application.h"
#include "codegenerator.h"

#include <boost/filesystem.hpp>

#include <string>

/// Main class responsible for serialization code generation based on objects created while xml analysis.
class TSerializableMap
  {
  private:
    typedef AApplication::TEnums TEnums;
    typedef AApplication::TClasses TClasses;
    typedef boost::filesystem::path path;
    typedef std::set<const TClass*> TClassSet;
    typedef const std::vector<std::string> TStringCntr;

  public:
    TSerializableMap(const TClasses& classes, const TEnums& enums, TLogger& logger,
      const std::vector<path>& inputs, const path& working_dir, const std::string& output_prefix,
#if defined(GENERATE_ENUM_OPERATORS)
      const std::vector<std::string>& ignoredNamespaces,
#endif
      int indent, bool check_for_changes);

    AApplication::TPhaseResult Generate();

  private:
    bool WriteParsedHeaderInjectedFunctions();
    bool WriteParsedHeaderTypeIds();

    void WriteTypeIdDeclaration(const TClass& _class);

#if defined(GENERATE_ENUM_OPERATORS)
    void WriteOperatorsForEnums();
#endif

    void WriteFunctionsForClasses();

    /** Analyze members of the class and check if any of them is not serializable.
        Note: if any member is of named struct type, operators '&' will be generated.
    */
    bool AnalyzeMembers(const TClass& _class);
    void WriteBuildForSerializerMethods(const TClass& _class);
    void WriteMethodsForClass(const TClass& _class);
    void WriteBuildForSerializerFunction(const TClass& _class);
    void WriteDumpObjectFunction(const TClass& _class);
    void WriteLoadObjectFunction(const TClass& _class);
    void WriteTypeIdFunction(const TClass& _class);
    void WriteDumpObjectPointerFunction(const TClass& _class);
    void WriteLoadObjectPointerFunction(const TClass& _class);
    void WriteTypeIdCase(const TClass& _class);
    void WriteCasesForDerived(const TClass& _class, TClassSet& classCasesWritten);

    template <bool DUMP_LOAD> // true for DUMP, false for LOAD
    void WriteCall(const TClassMember& member, const std::string& prefix = "",
                   const std::string& _indent = Indent);
    void WriteDumpCall(const TClassMember& member)
      { WriteCall<true>(member); }
    void WriteLoadCall(const TClassMember& member)
      { WriteCall<false>(member); }

    template <bool DUMP_LOAD> // true for DUMP, false for LOAD
    void WriteInplaceStruct(const TClass& _class, const std::string& prefix, const std::string& indent);
    void WriteDumpInplaceStruct(const TClass& _class, const std::string& prefix, const std::string& indent)
      { WriteInplaceStruct<true>(_class, prefix, indent); }
    void WriteLoadInplaceStruct(const TClass& _class, const std::string& prefix, const std::string& indent)
      { WriteInplaceStruct<false>(_class, prefix, indent); }

    template <bool DUMP_LOAD> // true for DUMP, false for LOAD
    void WriteInplaceUnion(const TClass& _class, const std::string& prefix);
    void WriteDumpInplaceUnion(const TClass& _class, const std::string& prefix)
      { WriteInplaceUnion<true>(_class, prefix); }
    void WriteLoadInplaceUnion(const TClass& _class, const std::string& prefix)
      { WriteInplaceUnion<false>(_class, prefix); }

    bool HandleArray(std::ofstream& out, std::string& indent, std::string iterator,
                     std::string& reference, const TArrayType& arrayType, const TType** type);

#if defined(GENERATE_ENUM_OPERATORS)
    std::string GetTypeEnumCast(const TEnum& _enum, bool dump);
#endif

    void OpenNamespaces(TNamespaces&& namespaces);
    void CloseNamespaces();

    static std::string GetTypeIdName(const TClass& _class);

  private:
    const TClasses&     Classes; //contains both auto and manually serialized classes
    const TEnums&       Enums;
    int                 TypeIdCounter = 1;
    TLogger&            Logger;
    bool                CheckForChanges = false;

    int                 Errors = 0;
    TCodeGenerator      CodeGenerator; //writes to output files
  
    //input file names
    const std::vector<path>& Inputs; 

#if defined(GENERATE_ENUM_OPERATORS)
    //ignored namespaces
    const std::vector<std::string>& IgnoredNamespaces;
#endif

    //output file names
    path                ParsedHeaderTypeIdsFileName;
    path                InjectedFunctionsFileName;

    TNamespaces         CurrentOpenedNamespaces;
    std::string         CurrentClassName;
    std::string         CurrentClassFullName;
    std::string         CurrentTypeIdName;

    static std::string  Indent;
    static std::string  Indent2;
    static std::string  Indent3;
  };
