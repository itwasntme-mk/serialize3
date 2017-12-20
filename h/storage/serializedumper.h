#pragma once

#include <string>

/// Base abstract class for all implementations of dumpers used for storing serialization data.
class ASerializeDumper
  {
  public:
    /// Common method for dumping all primitive types.
    template <class TSimpleDataType>
    void Dump(TSimpleDataType value)
      {
      DumpSizedBuffer<sizeof(TSimpleDataType)>(reinterpret_cast<const unsigned char*>(&value));
      }

    void DumpSizeT(size_t v)
      {
      unsigned long long buffer = v;
      Dump(buffer);
      }

    /// Debug logging support.
    virtual void PushIndent() { ++IndentLevel; }
    virtual void PopIndent()  { --IndentLevel; }
    virtual void Log(const char* msg) = 0;
    /// Common method to dump memory buffer.
    virtual void WriteBuffer(const unsigned char* buffer, size_t bufferLen) = 0;
    /// Specialized method to dump string. 
    virtual void Dump(const std::string& s)
      {
      size_t length = s.size();
      DumpSizeT(length);
      WriteBuffer(reinterpret_cast<const unsigned char *>(s.c_str()), length);
      }

  protected:
    ASerializeDumper() : IndentLevel(0) {}
    virtual ~ASerializeDumper() {}

    template <size_t S>
    void DumpSizedBuffer(const unsigned char* buffer)
      {
      WriteBuffer(buffer, S);
      }

    virtual void DumpBuffer2(const unsigned char* buffer)
      {
      WriteBuffer(buffer, 2);
      }

    virtual void DumpBuffer4(const unsigned char* buffer)
      {
      WriteBuffer(buffer, 4);
      }

    virtual void DumpBuffer8(const unsigned char* buffer)
      {
      WriteBuffer(buffer, 8);
      }

  /// Class attributes:
  protected:
    unsigned int IndentLevel;
  };

template <>
inline void ASerializeDumper::Dump<long>(long value)
  {
  long long buffer = value;
  return this->Dump(buffer);
  }
template <>
inline void ASerializeDumper::Dump<unsigned long>(unsigned long value)
  {
  unsigned long long buffer = value;
  return this->Dump(buffer);
  }

template <>
inline void ASerializeDumper::DumpSizedBuffer<2>(const unsigned char* buffer)
  {
  DumpBuffer2(buffer);
  }

template <>
inline void ASerializeDumper::DumpSizedBuffer<4>(const unsigned char* buffer)
  {
  DumpBuffer4(buffer);
  }

template <>
inline void ASerializeDumper::DumpSizedBuffer<8>(const unsigned char* buffer)
  {
  DumpBuffer8(buffer);
  }

#ifdef DEBUG_SERIALIZER
  #define DLOGMSG(msg) dumper.Log(msg)
  #define DPUSH_INDENT dumper.PushIndent()
  #define DPOP_INDENT dumper.PopIndent()
#else
  #define DLOGMSG(msg)
  #define DPUSH_INDENT
  #define DPOP_INDENT
#endif
