//Auto-generated by serialize3.exe
//Defines member functions injected into serializable objects
#include <cassert>
#define SERIALIZER_INHERITANCE_SWITCH(inhType) public
#include "test0.hpp"
#include <serialize3/h/gen_code/dumpertemplates.h>
#include <serialize3/h/gen_code/loadertemplates.h>
#include "test0_typeids.hpp"
#ifndef REGISTER_OBJECT
  #define REGISTER_OBJECT(_class_,_ptr_)
#endif
#ifndef WRAP
  #define WRAP(...) __VA_ARGS__
#endif

void* TClass::BuildForSerializer()
  {
  TClass* ptr = new TClass;
  REGISTER_OBJECT((WRAP(TClass)), ptr);
  return ptr;
  }
void TClass::Dump(ASerializeDumper& dumper) const
  {
  DPUSH_INDENT;
  DLOGMSG("Dump TClass");
  dumper & m1;
  dumper & m2;
  for (int i = 0; i < 3; ++i)
    {
    }
  DPOP_INDENT;
  }
void TClass::Load(ASerializeLoader& loader)
  {
  LPUSH_INDENT;
  LLOGMSG("Load TClass");
  loader & m1;
  loader & m2;
  for (int i = 0; i < 3; ++i)
    {
    }
  LPOP_INDENT;
  }
TTypeId TClass::GetTypeId() const { return -1; }
void TClass::DumpPointer(ASerializeDumper& dumper) const
  {
  DLOGMSG("Dump TClass pointer");
  dumper.Dump(GetTypeId());
  dumper & *this;
  }
void* TClass::LoadPointer(ASerializeLoader& loader)
  {
  LLOGMSG("Load TClass pointer");
  TTypeId objectTypeId;
  loader.Load(objectTypeId);
  TClass* o;
  switch(objectTypeId)
    {
    case NULL_TYPE_ID:
      o = 0;
      break;
    case -1:
      o = (TClass*)TClass::BuildForSerializer();
      loader & (TClass&)*o;
      break;
    default:
      assert(false);
      o = 0;
      throw 1;
    } //end switch
  return o;
  } //end LoadPointer
