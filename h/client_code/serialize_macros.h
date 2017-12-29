#pragma once

class ASerializeDumper;
class ASerializeLoader;

/// Storage type used for each non-abstract class type-id.
typedef int TTypeId;

// Macro to be used when nonpublic inheritance is needed for serializable class
// it is redefined to public just for serializer generated code
#ifndef SERIALIZER_INHERITANCE_SWITCH
#define SERIALIZER_INHERITANCE_SWITCH(inhType) inhType
#endif

// INTERNAL MACROS: Do not use directly, only used for macros below.
#define AUTOMATIC_SERIALIZE_MARKER_NAME         _AutofAkE_
#define AUTOMATIC_OBJECT_SERIALIZE_MARKER_NAME  _AutoObjfAkE_
#define MANUAL_SERIALIZE_MARKER_NAME            _ManfAkE_
#define MANUAL_POINTER_SERIALIZE_MARKER_NAME    _ManPtrfAkE_
#define MANUAL_BUILD_SERIALIZE_MARKER_NAME      _ManNewfAkE_
#define MANUAL_FULL_MARKER_NAME                 _ManFullfAkE_
#define MANUAL_OBJECT_SERIALIZE_MARKER_NAME     _ManObjectfAkE_
#define DO_NOT_SERIALIZE_MARKER_NAME            _DoNotfAkE_

#define COMMON_OBJECT_SERIALIZABLE                                 \
  public:                                                          \
  void Dump(ASerializeDumper& dumper) const;                       \
  void Load(ASerializeLoader& loader);
           
#define COMMON_SERIALIZABLE                                        \
  COMMON_OBJECT_SERIALIZABLE                                       \
  virtual void DumpPointer(ASerializeDumper& dumper) const;        \
  static  void* LoadPointer(ASerializeLoader& loader);             \
  static  void* BuildForSerializer();                              \
private:                                                           \
  virtual TTypeId GetTypeId() const;                          

#define COMMON_SERIALIZABLE_1                                      \
  COMMON_OBJECT_SERIALIZABLE                                       \
  virtual void DumpPointer(ASerializeDumper& dumper) const;        \
  static  void* LoadPointer(ASerializeLoader& loader);             \
  static  void* BuildForSerializer(ASerializeLoader& loader);      \
private:                                                           \
  virtual TTypeId GetTypeId() const;

//USAGE: Put in the beginning of class definition for
//       AUTOMATIC serialization. These objects can be serialized
//       directly or thru pointers. All serialization functions
//       will be auto-generated.
//       Access scope after macro is private.
#define SERIALIZABLE                                         \
  static void AUTOMATIC_SERIALIZE_MARKER_NAME (void) {}      \
  COMMON_SERIALIZABLE 

//USAGE: Put in the beginning of class definition for
//       AUTOMATIC serialization of "object-only" objects
//       (objects never serialized thru a pointer).
//       Access scope after macro is private.
#define SERIALIZABLE_OBJECT                                       \
  static void AUTOMATIC_OBJECT_SERIALIZE_MARKER_NAME (void) {}    \
  COMMON_OBJECT_SERIALIZABLE                                      \
private:

//USAGE: Put in the beginning of class definition for
//       MANUAL serialization. Access scope after 
//       macro is private. You need to define Dump/Load
//       functions manually, rest will be auto-generated.
#define MANUALLY_SERIALIZABLE                                      \
  static void MANUAL_SERIALIZE_MARKER_NAME (void) {}               \
  COMMON_SERIALIZABLE                                              \
private:

//USAGE: Put in the beginning of class definition for
//       MANUAL serialization. Access scope after 
//       macro is private. You need to define Dump/Load as well as
//       DumpPointer/LoadPointer manually, rest will be auto-generated.
#define MANUALLY_SERIALIZABLE_POINTER                              \
  static void MANUAL_POINTER_SERIALIZE_MARKER_NAME (void) {}       \
  COMMON_SERIALIZABLE                                              \
private:

//USAGE: Put in the beginning of class definition for
//       MANUAL serialization.  You need to define BuildForSerializer
//       manually, rest will be auto-generated.
//       Access scope after macro is private.
#define MANUALLY_SERIALIZABLE_BUILDING                             \
  static void MANUAL_BUILD_SERIALIZE_MARKER_NAME (void) {}         \
  COMMON_SERIALIZABLE                                              \
private:

//USAGE: Put in the beginning of class definition for
//       MANUAL serialization. Access scope after 
//       macro is private. You need to define all functions
//       except for GetTypeId (which is auto-generated).
//       In other words, define: Dump/Load, DumpPointer/LoadPointer,
//       and BuildForSerializer. Note that this BuildForSerializer
//       is declared with 'loader' parameter unlike others.
#define MANUALLY_SERIALIZABLE_FULL                                \
  static void MANUAL_FULL_MARKER_NAME (void) {}                   \
  COMMON_SERIALIZABLE_1


//USAGE: Put in the beginning of class definition for
//       MANUAL serialization of objects that will never be
//       serialized via a pointer (requires no virtual fcns or TypeIds).
//       You need to define Dump/Load functions.
#define MANUALLY_SERIALIZABLE_OBJECT                        \
  static void MANUAL_OBJECT_SERIALIZE_MARKER_NAME (void) {} \
public:                                                     \
  void Dump(ASerializeDumper& dumper) const;                \
  void Load(ASerializeLoader& loader);                      \
private:                                                           


//USAGE: Put in any class whose data should not be serialized when the
//       class is a data member or base class of a serialized class.
#define NOT_SERIALIZABLE                                           \
  static void DO_NOT_SERIALIZE_MARKER_NAME (void) {}
