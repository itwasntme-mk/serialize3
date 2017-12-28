///\file codegenerator.h

#include "logger.h"

#include <fstream>
#include <vector>
#include <string>

/// Helper class for generate serialization code.
class TCodeGenerator
  {
  public:
    explicit TCodeGenerator(TLogger& logger, const std::string& indent)
      : Logger(logger), Indent(indent) {}

    //file creation
    bool Open(const char* fileName, const char* fileDescription = 0);
    void Close();
  
    //preprocessor directives
    void AddDefine(const char* macro, const char* value);
    void AddSystemInclude(const char* includePath);
    void AddInclude(const char* includePath);

    void StartHeaderSentinel();
    void EndHeaderSentinel();

    //misc  
    void DoInlineInclude(const char* fileName);
    void AddComment(const char* comment);
    //declarations
    void WriteClassDeclaration(const char* className);

    //statements
    void AddLogCall(const char* logMsg,const char* logger);
    void AddLogMacro(const char* logMsg, const char* macroName);

  public:
    std::ofstream   Out;

  private:
    TLogger&            Logger;
    const std::string&  Indent;
    const char*         OutputFileName = nullptr;
    bool                AddLogging = true;
  };
