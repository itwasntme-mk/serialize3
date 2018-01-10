///\file application.h
#pragma once

#include "apps_factory.h"
#include "logger.h"

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <string>
#include <vector>
#include <forward_list>

//#define USE_SORTED_CLASSES

class TClass;
class TEnum;

/** Main class of application. Control whole process, fire phases:
    - preprocess input files with given compiler;
    - generate xml intermediate file using given xml-generator;
    - read and analyze xml file;
    - generate serialization code for all marked classes and enum types.
*/
class AApplication
  {
  public:
    typedef std::vector<const TEnum*> TEnums;

#if defined(USE_SORTED_CLASSES)
    typedef std::forward_list<const TClass*> TClasses;
#else
    typedef std::vector<const TClass*> TClasses;
#endif

    typedef boost::program_options::variables_map variables_map;
    typedef boost::filesystem::path path;

    enum class TPhaseResult : char
      {
      OK,
      NO_CHANGES,
      ERROR
      };

  public:
    /** Main method. Returns 0 if success, or negative value if failed:
        -1 for Initialize
        -2 for PreProcess
        -3 for GenerateXML
        -4 for ParseXML
        -5 for GenerateSerializationCode
    */
    int Run(const variables_map& args);

    virtual ~AApplication() {};

  protected:
    virtual bool Initialize(const variables_map& args);

    virtual const char* GetXmlRootName() const = 0;

    /// Preprocess file with main compiler.
    virtual TPhaseResult Preprocess(bool check_for_changes);

    /// Generate xml from input file using external tool (gccxml, castxml etc.).
    virtual TPhaseResult GenerateXML(bool check_for_changes);

    /// Prepare cmd line for xml-generator.
    virtual std::string ComposeXmlGeneratorCmdLine() = 0;

    /** Read generated xml file into memory and create internal representation.
        Now it is common for all supported xml AST formats.
    */
    virtual bool ParseXML();

    /// Generate final code for serialization.
    virtual TPhaseResult GenerateSerializationCode(const variables_map& args);

    static bool SelfRegister(const std::string& name, TApplicationsFactory::TCreateApplication creator);

    void RewriteFileToLog(const path& filename);

  private:
    std::string ConfigureGenericCompiler() const;

    std::string ConfigureClang() const;

    std::string ConfigureGcc() const;

    std::string ConfigureMsvc() const;

    std::string PrepareIncludes() const;

  private:
#if defined(USE_SORTED_CLASSES)
    static inline void OrderClass(const TClass& _class);
#endif

  protected:
    path              XmlCreatorPath;
    std::string       XmlCreatorOptions;
    std::string       Compiler;
    path              CompilerPath;
    std::string       CompilerOptions;
    std::string       CompilerIncludes;
    std::vector<path> InputFiles;
    path              PreprocessedFile;
    path              XmlFile;
    std::string       OutputFilePrefix;
    path              WorkingDir;

    TLogger           Logger;
    TEnums            Enums;
    TClasses          SerializableClasses;
  };
