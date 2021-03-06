//Auto-generated by serialize3.exe
//Defines member functions injected into serializable objects
#include <cassert>
#define SERIALIZER_INHERITANCE_SWITCH(inhType) public
#include "test1.hpp"
#include <serialize3/h/gen_code/dumpertemplates.h>
#include <serialize3/h/gen_code/loadertemplates.h>
#include "test1_typeids.hpp"
#ifndef REGISTER_OBJECT
  #define REGISTER_OBJECT(_class_,_ptr_)
#endif
#ifndef WRAP
  #define WRAP(...) __VA_ARGS__
#endif

namespace xtd
{
void* ABase::BuildForSerializer() { return 0; }
void* TMyClass::BuildForSerializer()
  {
  TMyClass* ptr = new TMyClass;
  REGISTER_OBJECT((WRAP(TMyClass)), ptr);
  return ptr;
  }
template <> void* TTemplate<int>::BuildForSerializer()
  {
  TTemplate<int>* ptr = new TTemplate<int>;
  REGISTER_OBJECT((WRAP(TTemplate<int>)), ptr);
  return ptr;
  }
template <> void* TTemplate<xtd::TMyClass>::BuildForSerializer()
  {
  TTemplate<xtd::TMyClass>* ptr = new TTemplate<xtd::TMyClass>;
  REGISTER_OBJECT((WRAP(TTemplate<xtd::TMyClass>)), ptr);
  return ptr;
  }
void* TMyClass1::BuildForSerializer()
  {
  TMyClass1* ptr = new TMyClass1;
  REGISTER_OBJECT((WRAP(TMyClass1)), ptr);
  return ptr;
  }
void ABase::Dump(ASerializeDumper& dumper) const
  {
  DPUSH_INDENT;
  DLOGMSG("Dump xtd::ABase");
  dumper & m1;
  for (int i = 0; i < 3; ++i)
    dumper & m3[i];
  for (int i = 0; i < 3; ++i)
    for (int ii = 0; ii < 4; ++ii)
      dumper & m4[i][ii];
  DPOP_INDENT;
  }
void ABase::Load(ASerializeLoader& loader)
  {
  LPUSH_INDENT;
  LLOGMSG("Load xtd::ABase");
  loader & m1;
  for (int i = 0; i < 3; ++i)
    loader & m3[i];
  for (int i = 0; i < 3; ++i)
    for (int ii = 0; ii < 4; ++ii)
      loader & m4[i][ii];
  LPOP_INDENT;
  }
TTypeId ABase::GetTypeId() const { return -1; }
void ABase::DumpPointer(ASerializeDumper& dumper) const
  {
  DLOGMSG("Dump xtd::ABase pointer");
  dumper.Dump(GetTypeId());
  dumper & *this;
  }
void* ABase::LoadPointer(ASerializeLoader& loader)
  {
  LLOGMSG("Load xtd::ABase pointer");
  TTypeId objectTypeId;
  loader.Load(objectTypeId);
  xtd::ABase* o;
  switch(objectTypeId)
    {
    case NULL_TYPE_ID:
      o = 0;
      break;
    case xtd__TMyClass_TYPE_ID:
      {
      xtd::TMyClass* d = (xtd::TMyClass*)xtd::TMyClass::BuildForSerializer();
      loader & *d;
      o = d;
      }
      break;
    case xtd__TMyClass1_TYPE_ID:
      o = (xtd::TMyClass1*)xtd::TMyClass1::BuildForSerializer();
      loader & (xtd::TMyClass1&)*o;
      break;
    default:
      assert(false);
      o = 0;
      throw 1;
    } //end switch
  return o;
  } //end LoadPointer
void TMyClass::Dump(ASerializeDumper& dumper) const
  {
  DPUSH_INDENT;
  DLOGMSG("Dump xtd::TMyClass");
  dumper & static_cast<const xtd::ABase&>(*this);
  dumper & mm1;
  dumper & M1;
  dumper & M2;
  dumper & M3;
  dumper & M4;
  DPOP_INDENT;
  }
void TMyClass::Load(ASerializeLoader& loader)
  {
  LPUSH_INDENT;
  LLOGMSG("Load xtd::TMyClass");
  loader & static_cast<xtd::ABase&>(*this);
  loader & mm1;
  loader & M1;
  loader & M2;
  loader & M3;
  loader & M4;
  LPOP_INDENT;
  }
TTypeId TMyClass::GetTypeId() const { return xtd__TMyClass_TYPE_ID; }
void TMyClass::DumpPointer(ASerializeDumper& dumper) const
  {
  DLOGMSG("Dump xtd::TMyClass pointer");
  dumper.Dump(GetTypeId());
  dumper & *this;
  }
void* TMyClass::LoadPointer(ASerializeLoader& loader)
  {
  LLOGMSG("Load xtd::TMyClass pointer");
  TTypeId objectTypeId;
  loader.Load(objectTypeId);
  xtd::TMyClass* o;
  switch(objectTypeId)
    {
    case NULL_TYPE_ID:
      o = 0;
      break;
    case xtd__TMyClass_TYPE_ID:
      {
      xtd::TMyClass* d = (xtd::TMyClass*)xtd::TMyClass::BuildForSerializer();
      loader & *d;
      o = d;
      }
      break;
    case xtd__TMyClass1_TYPE_ID:
      o = (xtd::TMyClass1*)xtd::TMyClass1::BuildForSerializer();
      loader & (xtd::TMyClass1&)*o;
      break;
    default:
      assert(false);
      o = 0;
      throw 1;
    } //end switch
  return o;
  } //end LoadPointer
template <> void TTemplate<int>::Dump(ASerializeDumper& dumper) const
  {
  DPUSH_INDENT;
  DLOGMSG("Dump xtd::TTemplate<int>");
  dumper & m1;
  dumper & m2;
  DPOP_INDENT;
  }
template <> void TTemplate<int>::Load(ASerializeLoader& loader)
  {
  LPUSH_INDENT;
  LLOGMSG("Load xtd::TTemplate<int>");
  loader & m1;
  loader & m2;
  LPOP_INDENT;
  }
template <> TTypeId TTemplate<int>::GetTypeId() const { return -1; }
template <> void TTemplate<int>::DumpPointer(ASerializeDumper& dumper) const
  {
  DLOGMSG("Dump xtd::TTemplate<int> pointer");
  dumper.Dump(GetTypeId());
  dumper & *this;
  }
template <> void* TTemplate<int>::LoadPointer(ASerializeLoader& loader)
  {
  LLOGMSG("Load xtd::TTemplate<int> pointer");
  TTypeId objectTypeId;
  loader.Load(objectTypeId);
  xtd::TTemplate<int>* o;
  switch(objectTypeId)
    {
    case NULL_TYPE_ID:
      o = 0;
      break;
    case -1:
      o = (xtd::TTemplate<int>*)xtd::TTemplate<int>::BuildForSerializer();
      loader & (xtd::TTemplate<int>&)*o;
      break;
    default:
      assert(false);
      o = 0;
      throw 1;
    } //end switch
  return o;
  } //end LoadPointer
template <> void TTemplate<xtd::TMyClass>::Dump(ASerializeDumper& dumper) const
  {
  DPUSH_INDENT;
  DLOGMSG("Dump xtd::TTemplate<xtd::TMyClass>");
  dumper & m1;
  dumper & m2;
  DPOP_INDENT;
  }
template <> void TTemplate<xtd::TMyClass>::Load(ASerializeLoader& loader)
  {
  LPUSH_INDENT;
  LLOGMSG("Load xtd::TTemplate<xtd::TMyClass>");
  loader & m1;
  loader & m2;
  LPOP_INDENT;
  }
template <> TTypeId TTemplate<xtd::TMyClass>::GetTypeId() const { return -1; }
template <> void TTemplate<xtd::TMyClass>::DumpPointer(ASerializeDumper& dumper) const
  {
  DLOGMSG("Dump xtd::TTemplate<xtd::TMyClass> pointer");
  dumper.Dump(GetTypeId());
  dumper & *this;
  }
template <> void* TTemplate<xtd::TMyClass>::LoadPointer(ASerializeLoader& loader)
  {
  LLOGMSG("Load xtd::TTemplate<xtd::TMyClass> pointer");
  TTypeId objectTypeId;
  loader.Load(objectTypeId);
  xtd::TTemplate<xtd::TMyClass>* o;
  switch(objectTypeId)
    {
    case NULL_TYPE_ID:
      o = 0;
      break;
    case -1:
      o = (xtd::TTemplate<xtd::TMyClass>*)xtd::TTemplate<xtd::TMyClass>::BuildForSerializer();
      loader & (xtd::TTemplate<xtd::TMyClass>&)*o;
      break;
    default:
      assert(false);
      o = 0;
      throw 1;
    } //end switch
  return o;
  } //end LoadPointer
void TMyClass1::Dump(ASerializeDumper& dumper) const
  {
  DPUSH_INDENT;
  DLOGMSG("Dump xtd::TMyClass1");
  dumper & static_cast<const xtd::TMyClass&>(*this);
  dumper & mm1;
  dumper & mm2;
  dumper & mm3;
  dumper & M1;
  dumper & M2;
  dumper & M3;
  dumper & M4;
  dumper & M5;
  dumper & M6;
  dumper & M7;
  dumper & M8;
  dumper & M9;
  dumper & M10;
  dumper & M11;
  dumper & M12;
  dumper & M13;
  dumper & M14;
  dumper & M15;
  DPOP_INDENT;
  }
void TMyClass1::Load(ASerializeLoader& loader)
  {
  LPUSH_INDENT;
  LLOGMSG("Load xtd::TMyClass1");
  loader & static_cast<xtd::TMyClass&>(*this);
  loader & mm1;
  loader & mm2;
  loader & mm3;
  loader & M1;
  loader & M2;
  loader & M3;
  loader & M4;
  loader & M5;
  loader & M6;
  loader & M7;
  loader & M8;
  loader & M9;
  loader & M10;
  loader & M11;
  loader & M12;
  loader & M13;
  loader & M14;
  loader & M15;
  LPOP_INDENT;
  }
TTypeId TMyClass1::GetTypeId() const { return xtd__TMyClass1_TYPE_ID; }
void TMyClass1::DumpPointer(ASerializeDumper& dumper) const
  {
  DLOGMSG("Dump xtd::TMyClass1 pointer");
  dumper.Dump(GetTypeId());
  dumper & *this;
  }
void* TMyClass1::LoadPointer(ASerializeLoader& loader)
  {
  LLOGMSG("Load xtd::TMyClass1 pointer");
  TTypeId objectTypeId;
  loader.Load(objectTypeId);
  xtd::TMyClass1* o;
  switch(objectTypeId)
    {
    case NULL_TYPE_ID:
      o = 0;
      break;
    case xtd__TMyClass1_TYPE_ID:
      o = (xtd::TMyClass1*)xtd::TMyClass1::BuildForSerializer();
      loader & (xtd::TMyClass1&)*o;
      break;
    default:
      assert(false);
      o = 0;
      throw 1;
    } //end switch
  return o;
  } //end LoadPointer
} // namespace xtd
