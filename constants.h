/** \file constants.h - contains strings dependent on other outside modules.
                        Currently these are GCC_XML and boost::serialization.
*/
#pragma once

#include "h/client_code/serialize_macros.h"

typedef const char* TTagType;
//IVAN XML starting elements could be these kinds of types:
const char TAG_FIELD[] = "Field";
const char TAG_CLASS[] = "Class";
const char TAG_STRUCT[] = "Struct";
const char TAG_UNION[] = "Union";
const char TAG_NAMESPACE[] = "Namespace";
const char TAG_FUNDAMENTAL_TYPE[] = "FundamentalType";
const char TAG_TYPEDEF[] = "Typedef";
const char TAG_POINTER_TYPE[] = "PointerType";
const char TAG_REFERENCE_TYPE[] = "ReferenceType";
const char TAG_ARRAY_TYPE[] = "ArrayType";
const char TAG_ENUM_TYPE[] = "Enumeration";
const char TAG_CV_QUALIFIED_TYPE[] = "CvQualifiedType";
const char TAG_ELABORATED_TYPE[] = "ElaboratedType";
const char TAG_METHOD[] = "Method";
const char TAG_BASE[] = "Base";

//IVAN XML starting elements could have these attributes:
const char ATTRIBUTE_ID[] = "id";
const char ATTRIBUTE_CONTEXT[] = "context"; 
const char ATTRIBUTE_NAME[] = "name";
const char ATTRIBUTE_TYPE[] = "type";
const char ATTRIBUTE_BASES[] = "bases";
const char ATTRIBUTE_MEMBERS[] = "members";
const char ATTRIBUTE_FILE[] = "file";
const char ATTRIBUTE_LINE[] = "line";
const char ATTRIBUTE_ABSTRACT[] = "abstract";
const char ATTRIBUTE_VIRTUAL[] = "virtual";
const char ATTRIBUTE_BITS[] = "bits";
const char ATTRIBUTE_SIZE[] = "size";
const char ATTRIBUTE_INCOMPLETE[] = "incomplete";
const char ATTRIBUTE_MIN[] = "min";
const char ATTRIBUTE_MAX[] = "max";
const char ATTRIBUTE_CONST[] = "const";
const char ATTRIBUTE_ACCESS[] = "access";

//IVAN The attributes of starting elements can have these 
//     values:
const char VALUE_CHAR[] = "char";
const char VALUE_TCHAR[] = "TChar";
const char VALUE_PCHAR[] = "PChar";

#define STRINGIZE(param) DO_STRINGIZE(param)
#define DO_STRINGIZE(param) #param

typedef const char* TMethodType;
//IVAN XML attributes can have these values 
const char AUTOMATIC_SERIALIZE_MARKER[] = STRINGIZE(AUTOMATIC_SERIALIZE_MARKER_NAME);
const char AUTOMATIC_OBJECT_SERIALIZE_MARKER[] = STRINGIZE(AUTOMATIC_OBJECT_SERIALIZE_MARKER_NAME);
const char MANUAL_SERIALIZE_MARKER[] = STRINGIZE(MANUAL_SERIALIZE_MARKER_NAME);
const char MANUAL_POINTER_SERIALIZE_MARKER[] = STRINGIZE(MANUAL_POINTER_SERIALIZE_MARKER_NAME);
const char MANUAL_BUILD_SERIALIZE_MARKER[] = STRINGIZE(MANUAL_BUILD_SERIALIZE_MARKER_NAME);
const char MANUAL_OBJECT_SERIALIZE_MARKER[] = STRINGIZE(MANUAL_OBJECT_SERIALIZE_MARKER_NAME);
const char MANUAL_FULL_MARKER[] = STRINGIZE(MANUAL_FULL_MARKER_NAME);
const char DO_NOT_SERIALIZE_MARKER[] = STRINGIZE(DO_NOT_SERIALIZE_MARKER_NAME);


//IVAN Used to exchange type of serializable object between 
//     analysis funcs and factory object
enum TSerializeMethod { 
  TYPE_NOT_MARKED                  = 0x80000000,
  TYPE_DO_NOT_SERIALIZE            = 0x40000000,

  TYPE_OBJECT_SERIALIZE            = 0x00000000, //Dump/Load defined manually
  TYPE_SERIALIZE                   = 0x20000000, //all functions but GetTypeId defined manually
  TYPE_DUMP                        = 0x00000001, //Dump definition generated
  TYPE_LOAD                        = 0x00000002, //Load definition generated
  TYPE_DUMPPOINTER                 = 0x20000004, //DumpPointer definition generated
  TYPE_LOADPOINTER                 = 0x20000008, //LoadPointer definition generated
  TYPE_BUILDPOINTER                = 0x20000010, //BuildForSerializer definition generated
  TYPE_BUILDPOINTER_W_LOADER       = 0x10000000, //BuildForSerializer declared with 'loader' parameter

  TYPE_MANUAL_OBJECT_SERIALIZE     = TYPE_OBJECT_SERIALIZE,
  TYPE_MANUAL_SERIALIZE            = TYPE_DUMPPOINTER | TYPE_LOADPOINTER | TYPE_BUILDPOINTER,
  TYPE_MANUAL_POINTER_SERIALIZE    = TYPE_BUILDPOINTER,
  TYPE_MANUAL_BUILD_SERIALIZE      = TYPE_DUMP | TYPE_LOAD | TYPE_DUMPPOINTER | TYPE_LOADPOINTER,
  TYPE_MANUAL_FULL                 = TYPE_SERIALIZE | TYPE_BUILDPOINTER_W_LOADER,

  TYPE_AUTOMATIC_OBJECT_SERIALIZE  = TYPE_DUMP | TYPE_LOAD,
  TYPE_AUTOMATIC_SERIALIZE         = TYPE_AUTOMATIC_OBJECT_SERIALIZE |
                                     TYPE_DUMPPOINTER | TYPE_LOADPOINTER | TYPE_BUILDPOINTER,
};
