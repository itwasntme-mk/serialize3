#pragma once

#include <serialize3/h/storage/serializedumper.h>

#include <iostream>
#include <fstream>
#include <iomanip>

//---------- TDumper
//Dump of primitive types is inlined
class TPrimitiveDumper : public std::ofstream,
                         public ASerializeDumper
  {
  public:
    explicit TPrimitiveDumper(const char* filename) : std::ofstream(filename,std::ios_base::binary) {}
    
  /// ASerializeDumper reimplementation:
    virtual void Log(const char* msg)
      {
      std::cout << std::setw(10);
      std::cout << this->tellp() << " ";
      for (int i = IndentLevel; i > 0; --i)
        std::cout << " ";
      std::cout << msg << std::endl;;
      }

    virtual void WriteBuffer(const unsigned char* buffer, size_t bufferLen)
      {
      write(reinterpret_cast<const char*>(buffer), bufferLen);
      }

    virtual void Dump(const std::string& s)
      {
      size_t length = s.size();
      ASerializeDumper* _this = this;
      _this->DumpSizeT(length);
      WriteBuffer(reinterpret_cast<const unsigned char*>(s.c_str()), length);
      }

  }; //TPrimitiveDumper
