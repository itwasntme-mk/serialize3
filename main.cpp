///\file main.cpp

#include "apps_factory.h"
#include "application.h"
#include "logger.h"

#include <iostream>
#include <string>
#include <memory>

namespace bpo = boost::program_options;

int main(int argc, const char *argv[])
  {
  bpo::options_description desc{"Usage"};

  try
    {
    typedef boost::filesystem::path path;
    std::string xml_creator;

    desc.add_options()
      ("help,h", "Help screen")
      ("xml-generator", bpo::value<std::string>(&xml_creator)->required(), "Specify xml generator (gccxml|castxml|castxml-gccxml).")
      ("xml-generator-path", bpo::value<path>()->default_value(""), "Path to xml generator program.")
      ("xml-generator-options", bpo::value<std::string>()->default_value(""), "Additional options for xml generator.")
      ("compiler", bpo::value<std::string>()->required(), "Compiler used for input preprocessing (msvc(cl)|gcc(gcc++)|clang(clang++)).")
      ("compiler-path", bpo::value<path>()->default_value(""), "Path to compiler used for input preprocessing.")
      ("compiler-options", bpo::value<std::string>()->default_value(""), "Additional options for compiler while preprocessing.")
      ("output,o", bpo::value<std::string>(), "Output file prefix.")
      (",d", bpo::value<path>()->default_value(""), "Working directory.")
      ("log,l", bpo::value<std::string>(), "Output log file.")
      ("quiet", "Quiet mode.")
      ("verbose", "Verbose mode.")
      ("indent", bpo::value<int>()->default_value(2), "Indentation in generated code. Default is 2.")
      ("stop-when-no-changes", bpo::value<std::string>(), "Stop when no changes after one of phases:\n"
                                                          "preprocess - file after preprocessing is the same,\n"
                                                          "xml - generated xml is the same,\n"
                                                          "cpp - generating output files are the same.")
      ("input", bpo::value<std::vector<path>>()->required(), "Input file(s).")
      ;

    bpo::variables_map vm;
    bpo::positional_options_description pos;
    pos.add("input", -1);
    bpo::store(bpo::command_line_parser(argc, argv).options(desc).positional(pos).run(), vm);
    //bpo::store(bpo::parse_command_line(argc, argv, desc), vm);
    bpo::notify(vm);

    if (vm.count("help"))
      std::cout << desc << std::endl;
    else
      {
      if (vm.count("input") == 0)
        throw bpo::error("Missing input file(s)");

      if (vm["indent"].as<int>() <= 0)
        throw bpo::error("--indent must be positive");

      const TApplicationsFactory* appsFactory = TApplicationsFactory::GetInstance();
      std::unique_ptr<AApplication> app(appsFactory->CreateApplication(xml_creator));

      if (!app)
        throw bpo::error(std::string("Not supported xml creator: ") + xml_creator);

      int result = app->Run(vm);

      return result;
      }
    }
  catch (const bpo::error& ex)
    {
    std::cerr << ex.what() << std::endl;
    std::cout << desc << std::endl;
    return -10;
    }
  catch (const std::exception& ex)
    {
    std::cerr << "Exception: " << ex.what() << std::endl;
    return -11;
    }
  catch (...)
    {
    std::cerr << "Unexpected error" << std::endl;
    return -12;
    }

  return 0;
  }
