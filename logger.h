///\file logger.h
#pragma once

#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <exception>

/// Helper class to allow log into cout/cerr and optional external file simultanously.
class TLogger
  {
  public:
    TLogger() = default;

    TLogger(const char* filename, bool verbose, bool quiet)
      : Verbose(verbose), Quiet(quiet)
      {
      Open(filename, verbose, quiet);
      }

    TLogger(const std::string& filename, bool verbose, bool quiet)
      : TLogger(filename.c_str(), verbose, quiet) {}

    bool Open(const char* filename, bool verbose, bool quiet)
      {
      Verbose = verbose;
      Quiet = quiet;

      if (filename && *filename != '\0')
        {
        LogFile.reset(new std::ofstream(filename));

        if (LogFile->good() == false)
          {
          std::cerr << "Cannot open log file " << filename << std::endl;
          return false;
          }
        }

      return true;
      }

    bool Open(const std::string& filename, bool verbose, bool quiet)
      { return Open(filename.c_str(), verbose, quiet); }

    bool IsVerboseMode() const { return Verbose; }

    bool IsQuiet() const { return Quiet; }

    void SetErrorMode(bool value) { Error = value; }

    template <typename TType>
    TLogger& operator << (const TType& v)
      {
      if (LogFile)
        *(LogFile.get()) << v;

      if (Quiet == false)
        {
        if (Error)
          std::cerr << v;
        else
          std::cout << v;
        }

      return *this;
      }

    TLogger& operator << (std::ostream& (*pf)(std::ostream&))
      {
      if (LogFile)
        *(LogFile.get()) << pf;

      if (Quiet == false)
        {
        if (Error)
          std::cerr << pf;
        else
          std::cout << pf;
        }

      return *this;
      }

    TLogger& operator << (std::ios& (*pf)(std::ios&))
      {
      if (LogFile)
        *(LogFile.get()) << pf;

      if (Quiet == false)
        {
        if (Error)
          std::cerr << pf;
        else
          std::cout << pf;
        }

      return *this;
      }

    TLogger& operator << (std::ios_base& (*pf)(std::ios_base&))
      {
      if (LogFile)
        *(LogFile.get()) << pf;

      if (Quiet == false)
        {
        if (Error)
          std::cerr << pf;
        else
          std::cout << pf;
        }

      return *this;
      }

  private:
    std::unique_ptr<std::ofstream>  LogFile;
    bool                            Verbose = false;
    bool                            Quiet = false;
    bool                            Error = false;
  };

#define LOG(ERROR, CONTENT) \
{ \
  Logger.SetErrorMode(ERROR); \
  Logger << CONTENT << std::endl; \
}

#define LOG_VERBOSE(CONTENT) \
{ \
  if (Logger.IsVerboseMode()) \
    LOG(false, CONTENT) \
}

#define LOG_INFO(CONTENT) \
  LOG(false, CONTENT)

#define LOG_NOTE(CONTENT) \
  LOG(false, "NOTE: " << CONTENT)

#define LOG_WARNING(CONTENT) \
  LOG(true, "WARNING: " << CONTENT)

#define LOG_ERROR(CONTENT) \
  LOG(true, "ERROR: " << CONTENT)
