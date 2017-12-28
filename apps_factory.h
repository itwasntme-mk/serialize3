///\file apps_factory.h
#pragma once

#include <map>
#include <string>
#include <functional>

class AApplication;

/// Build any kind of registered AApplication subclass.
class TApplicationsFactory
  {
  public:
    typedef std::function<AApplication*()> TCreateApplication;

  public:
    static TApplicationsFactory* GetInstance();

    void RegisterApplication(const std::string& name, TCreateApplication creator);

    /// Get AApplication instance for requested behavior (now: gccxml, castxml, castxml-gccxml).
    AApplication* CreateApplication(const std::string& name) const;

  private:
    TApplicationsFactory() = default;

  private:
    typedef std::map<std::string, TCreateApplication> TRegisteredApplications;
    TRegisteredApplications RegisteredApplications;

    static TApplicationsFactory*  Instance;
  };
