#pragma once

#include <string>

/// Base abstract class for all implementations of dumpers used for loading serialization data.
class ASerializeLoader
  {
  public:
    /// Common method for loading all primitive types.
    template <class TSimpleDataType>
    void Load(TSimpleDataType& value)
      {
      LoadSizedBuffer<sizeof(TSimpleDataType)>(reinterpret_cast<unsigned char*>(&value));
      }

    void LoadSizeT(size_t& value)
      {
      unsigned long long buffer = 0;
      Load(buffer);
      value = static_cast<size_t>(buffer);
      }

    virtual void Load(std::string& s)
      {
      //Using local buffer if string is smaller than 1024, else allocate it	 
      //on the heap. Could be optimized to use alloca instead, if we're comfortable with that.

      const int BUFFER_SIZE = 1024;
      char buffer[BUFFER_SIZE];
      char* readBuffer = nullptr;
      size_t length = 0;
      LoadSizeT(length);
      bool useStackBuffer = (length < BUFFER_SIZE);
      if(useStackBuffer)
        readBuffer = buffer;
      else
        readBuffer = new char[length + 1];
      ReadBuffer((unsigned char*)readBuffer, length);
      *(readBuffer + length) = 0;
      s = buffer;
      if(!useStackBuffer)
        delete[] readBuffer;
      }

    /// Debug logging support.
    virtual void PushIndent() { ++IndentLevel; }
    virtual void PopIndent()  { --IndentLevel; }
    virtual void Log(const char* msg) = 0;
    /// Common method to dump memory buffer.
    virtual void ReadBuffer(unsigned char* buffer, size_t bufferLen) = 0;

  protected:
    ASerializeLoader() : IndentLevel(0) {}
    virtual ~ASerializeLoader() {}

    template <size_t S>
    void LoadSizedBuffer(unsigned char* buffer)
      {
      ReadBuffer(buffer, S);
      }

    virtual void LoadBuffer2(unsigned char* buffer)
      {
      ReadBuffer(buffer, 2);
      }

    virtual void LoadBuffer4(unsigned char* buffer)
      {
      ReadBuffer(buffer, 4);
      }

    virtual void LoadBuffer8(unsigned char* buffer)
      {
      ReadBuffer(buffer, 8);
      }

  /// Class attributes:
  protected:
    unsigned int IndentLevel;
  };

template <>
inline void ASerializeLoader::Load(long& value)
  {
  long long buffer;
  this->Load(buffer);
  value = static_cast<long>(buffer);
  }

template <>
inline void ASerializeLoader::Load(unsigned long& value)
  {
  unsigned long long buffer;
  this->Load(buffer);
  value = static_cast<unsigned long>(buffer);
  }

template <>
inline void ASerializeLoader::LoadSizedBuffer<2>(unsigned char* buffer)
  {
  LoadBuffer2(buffer);
  }

template <>
inline void ASerializeLoader::LoadSizedBuffer<4>(unsigned char* buffer)
  {
  LoadBuffer4(buffer);
  }

template <>
inline void ASerializeLoader::LoadSizedBuffer<8>(unsigned char* buffer)
  {
  LoadBuffer8(buffer);
  }


#ifdef DEBUG_SERIALIZER
  #define LLOGMSG(msg) loader.Log(msg)
  #define LPUSH_INDENT loader.PushIndent()
  #define LPOP_INDENT loader.PopIndent()
#else
  #define LLOGMSG(msg)
  #define LPUSH_INDENT
  #define LPOP_INDENT
#endif
