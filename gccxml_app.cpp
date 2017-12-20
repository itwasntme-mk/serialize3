#include "application.h"
#include "external_app_launcher.h"

class TGccXMLApplication : public AApplication
  {
  public:
    virtual const char* GetXmlRootName() const override { return "GCCXML"; }

    virtual std::string ComposeXmlGeneratorCmdLine() override;

    virtual ~TGccXMLApplication() {}

  private:
    static bool Registered;
  };

bool TGccXMLApplication::Registered =
  AApplication::SelfRegister("gccxml", []() { return new TGccXMLApplication(); });

std::string TGccXMLApplication::ComposeXmlGeneratorCmdLine()
  {
  path castxml = XmlCreatorPath / "gccxml";
  LOG_INFO("GENERATE XML USING GCCXML ...")
  std::string command(castxml.generic_string() + " -o " + XmlFile.generic_string() +
    ' ' + XmlCreatorOptions + ' ' + PreprocessedFile.generic_string());

  return std::move(command);
  }
