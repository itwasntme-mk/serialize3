#pragma once

#include <serialize3/h/storage/serializeloader.h>

#include <iostream>
#include <iomanip>
#include <fstream>

#ifdef DEBUG_SERIALIZER
  #define LLOGMSG(msg) loader.Log(msg)
  #define LPUSH_INDENT loader.PushIndent()
  #define LPOP_INDENT loader.PopIndent()
#else
  #define LLOGMSG(msg)
  #define LPUSH_INDENT
  #define LPOP_INDENT
#endif

class TPrimitiveLoader : public std::ifstream,
                         public ASerializeLoader
  {
  public:
    explicit TPrimitiveLoader(const char* filename) : std::ifstream(filename,std::ios_base::binary) {}
    virtual ~TPrimitiveLoader() {}

  /// ASerializeLoader reimplementation:
    virtual void Log(const char* msg) override
      {
      std::cout << std::setw(10);
      std::cout << this->tellg() << " ";
      for (int i = IndentLevel; i > 0; --i)
        std::cout << " ";
      std::cout << msg << std::endl;
      }
    
    virtual void ReadBuffer(unsigned char* buffer, size_t bufferLen) override
      {
      read(reinterpret_cast<char*>(buffer), bufferLen);
      }

  }; //TPrimitiveLoader
