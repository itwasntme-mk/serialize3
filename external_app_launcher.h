///\file external_app_launcher.h
#pragma once

#include <string>

/// Launch external console application and redirect stdout. Returns true if success.
bool LaunchExternalConsoleProcess(const char* commandLine, const char* stdOutFileName,
  std::string& errorString);
