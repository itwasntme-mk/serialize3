#include "apps_factory.h"

TApplicationsFactory* TApplicationsFactory::Instance = nullptr;

TApplicationsFactory* TApplicationsFactory::GetInstance()
  {
  if (TApplicationsFactory::Instance == nullptr)
    TApplicationsFactory::Instance = new TApplicationsFactory();

  return TApplicationsFactory::Instance;
  }

void TApplicationsFactory::RegisterApplication(const std::string& name, TCreateApplication creator)
  {
  RegisteredApplications.emplace(name, creator);
  }

AApplication* TApplicationsFactory::CreateApplication(const std::string& name) const
  {
  auto found = RegisteredApplications.find(name);

  return found != RegisteredApplications.end() ? found->second() : nullptr;
  }
