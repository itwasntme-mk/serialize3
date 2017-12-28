///\file castxml-gccxml_app.cpp

#include "application.h"
#include "external_app_launcher.h"

class TCastXMLGccXMLApplication : public AApplication
  {
  public:
    virtual const char* GetXmlRootName() const override { return "GCC_XML"; }

    virtual std::string ComposeXmlGeneratorCmdLine() override;

    virtual ~TCastXMLGccXMLApplication() {}

  private:
    static bool Registered;
  };

bool TCastXMLGccXMLApplication::Registered =
  AApplication::SelfRegister("castxml-gccxml", []() { return new TCastXMLGccXMLApplication(); });

std::string TCastXMLGccXMLApplication::ComposeXmlGeneratorCmdLine()
  {
  path castxml = XmlCreatorPath / "castxml";
  LOG_INFO("GENERATE XML USING CASTXML-GCCXML ...")
  std::string command(castxml.generic_string() + " --castxml-gccxml -o " + XmlFile.generic_string() +
    ' ' + XmlCreatorOptions + ' ' + PreprocessedFile.generic_string());

  return std::move(command);
  }
