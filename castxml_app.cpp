///\file castxml_app.cpp

#include "application.h"
#include "external_app_launcher.h"

class TCastXMLApplication : public AApplication
  {
  public:
    virtual const char* GetXmlRootName() const override { return "CastXML"; }

    virtual std::string ComposeXmlGeneratorCmdLine() override;

    virtual ~TCastXMLApplication() {}

  private:
    static bool Registered;
  };

bool TCastXMLApplication::Registered =
  AApplication::SelfRegister("castxml", []() { return new TCastXMLApplication(); });

std::string TCastXMLApplication::ComposeXmlGeneratorCmdLine()
  {
  path castxml = XmlCreatorPath / "castxml";
  LOG_INFO("GENERATE XML USING CASTXML ...")
  std::string command('\"' + castxml.generic_string() + "\" --castxml-output=1 -o " + XmlFile.generic_string() +
    ' ' + XmlCreatorOptions + ' ' + PreprocessedFile.generic_string());

  return std::move(command);
  }
