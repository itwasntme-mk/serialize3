//Auto-generated by serialize3.exe
//Defines member functions injected into serializable objects
#include <cassert>
#define SERIALIZER_INHERITANCE_SWITCH(inhType) public
#include "test2.hpp"
#include <serialize3/h/gen_code/dumpertemplates.h>
#include <serialize3/h/gen_code/loadertemplates.h>
#include "test2_typeids.hpp"
#ifndef REGISTER_OBJECT
  #define REGISTER_OBJECT(_class_,_ptr_)
#endif
#ifndef WRAP
  #define WRAP(...) __VA_ARGS__
#endif

namespace itd
{
void* ABase::BuildForSerializer() { return 0; }
void* TBase::BuildForSerializer()
  {
  TBase* ptr = new TBase;
  REGISTER_OBJECT((WRAP(TBase)), ptr);
  return ptr;
  }
void* TClass::BuildForSerializer()
  {
  TClass* ptr = new TClass;
  REGISTER_OBJECT((WRAP(TClass)), ptr);
  return ptr;
  }
void ABase::Dump(ASerializeDumper& dumper) const
  {
  DPUSH_INDENT;
  DLOGMSG("Dump itd::ABase");
  dumper & m1;
  dumper & m2;
  dumper & m3;
  dumper & m4.m3;
  dumper & m13;
  dumper & m5.m1;
  dumper & m5.m2;
  dumper & m5.m3;
  dumper & m21;
  dumper & m22;
  dumper & m23;
  dumper & static_cast<const itd::ABase::TStruct&>(m7);
  dumper & m7.m1;
  dumper & m7.m2;
  dumper & m7.m3;
  DPOP_INDENT;
  }
void ABase::Load(ASerializeLoader& loader)
  {
  LPUSH_INDENT;
  LLOGMSG("Load itd::ABase");
  loader & m1;
  loader & m2;
  loader & m3;
  loader & m4.m3;
  loader & m13;
  loader & m5.m1;
  loader & m5.m2;
  loader & m5.m3;
  loader & m21;
  loader & m22;
  loader & m23;
  loader & static_cast<itd::ABase::TStruct&>(m7);
  loader & m7.m1;
  loader & m7.m2;
  loader & m7.m3;
  LPOP_INDENT;
  }
TTypeId ABase::GetTypeId() const { return -1; }
void ABase::DumpPointer(ASerializeDumper& dumper) const
  {
  DLOGMSG("Dump itd::ABase pointer");
  dumper.Dump(GetTypeId());
  dumper & *this;
  }
void* ABase::LoadPointer(ASerializeLoader& loader)
  {
  LLOGMSG("Load itd::ABase pointer");
  TTypeId objectTypeId;
  loader.Load(objectTypeId);
  itd::ABase* o;
  switch(objectTypeId)
    {
    case NULL_TYPE_ID:
      o = 0;
      break;
    case itd__TBase_TYPE_ID:
      {
      itd::TBase* d = (itd::TBase*)itd::TBase::BuildForSerializer();
      loader & *d;
      o = d;
      }
      break;
    case itd__TClass_TYPE_ID:
      o = (itd::TClass*)itd::TClass::BuildForSerializer();
      loader & (itd::TClass&)*o;
      break;
    default:
      assert(false);
      o = 0;
      throw 1;
    } //end switch
  return o;
  } //end LoadPointer
void TBase::Dump(ASerializeDumper& dumper) const
  {
  DPUSH_INDENT;
  DLOGMSG("Dump itd::TBase");
  dumper & static_cast<const itd::ABase&>(*this);
  DPOP_INDENT;
  }
void TBase::Load(ASerializeLoader& loader)
  {
  LPUSH_INDENT;
  LLOGMSG("Load itd::TBase");
  loader & static_cast<itd::ABase&>(*this);
  LPOP_INDENT;
  }
TTypeId TBase::GetTypeId() const { return itd__TBase_TYPE_ID; }
void TBase::DumpPointer(ASerializeDumper& dumper) const
  {
  DLOGMSG("Dump itd::TBase pointer");
  dumper.Dump(GetTypeId());
  dumper & *this;
  }
void* TBase::LoadPointer(ASerializeLoader& loader)
  {
  LLOGMSG("Load itd::TBase pointer");
  TTypeId objectTypeId;
  loader.Load(objectTypeId);
  itd::TBase* o;
  switch(objectTypeId)
    {
    case NULL_TYPE_ID:
      o = 0;
      break;
    case itd__TBase_TYPE_ID:
      {
      itd::TBase* d = (itd::TBase*)itd::TBase::BuildForSerializer();
      loader & *d;
      o = d;
      }
      break;
    case itd__TClass_TYPE_ID:
      o = (itd::TClass*)itd::TClass::BuildForSerializer();
      loader & (itd::TClass&)*o;
      break;
    default:
      assert(false);
      o = 0;
      throw 1;
    } //end switch
  return o;
  } //end LoadPointer
void TStruct::Dump(ASerializeDumper& dumper) const
  {
  DPUSH_INDENT;
  DLOGMSG("Dump itd::TStruct");
  dumper & m1;
  dumper & m2;
  dumper & m3;
  DPOP_INDENT;
  }
void TStruct::Load(ASerializeLoader& loader)
  {
  LPUSH_INDENT;
  LLOGMSG("Load itd::TStruct");
  loader & m1;
  loader & m2;
  loader & m3;
  LPOP_INDENT;
  }
void TClass::Dump(ASerializeDumper& dumper) const
  {
  DPUSH_INDENT;
  DLOGMSG("Dump itd::TClass");
  dumper & static_cast<const itd::TBase&>(*this);
  dumper & mm;
  DPOP_INDENT;
  }
void TClass::Load(ASerializeLoader& loader)
  {
  LPUSH_INDENT;
  LLOGMSG("Load itd::TClass");
  loader & static_cast<itd::TBase&>(*this);
  loader & mm;
  LPOP_INDENT;
  }
TTypeId TClass::GetTypeId() const { return itd__TClass_TYPE_ID; }
void TClass::DumpPointer(ASerializeDumper& dumper) const
  {
  DLOGMSG("Dump itd::TClass pointer");
  dumper.Dump(GetTypeId());
  dumper & *this;
  }
void* TClass::LoadPointer(ASerializeLoader& loader)
  {
  LLOGMSG("Load itd::TClass pointer");
  TTypeId objectTypeId;
  loader.Load(objectTypeId);
  itd::TClass* o;
  switch(objectTypeId)
    {
    case NULL_TYPE_ID:
      o = 0;
      break;
    case itd__TClass_TYPE_ID:
      o = (itd::TClass*)itd::TClass::BuildForSerializer();
      loader & (itd::TClass&)*o;
      break;
    default:
      assert(false);
      o = 0;
      throw 1;
    } //end switch
  return o;
  } //end LoadPointer
void ABase::TStruct::Dump(ASerializeDumper& dumper) const
  {
  DPUSH_INDENT;
  DLOGMSG("Dump itd::ABase::TStruct");
  dumper & m01;
  dumper & m02;
  dumper & m03;
  DPOP_INDENT;
  }
void ABase::TStruct::Load(ASerializeLoader& loader)
  {
  LPUSH_INDENT;
  LLOGMSG("Load itd::ABase::TStruct");
  loader & m01;
  loader & m02;
  loader & m03;
  LPOP_INDENT;
  }
} // namespace itd
