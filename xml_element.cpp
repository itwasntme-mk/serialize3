///\file xml_element.cpp

#include "xml_element.h"
#include "str_less.h"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

struct by_name;
struct by_value;
struct by_tag;
struct by_kind;

namespace boost { namespace multi_index
{

typedef std::pair<TMethodType, TSerializeMethod> TMethodSerializeValue;

typedef multi_index_container<
  TMethodSerializeValue,
  indexed_by<
    ordered_unique<tag<by_name>,
      member<TMethodSerializeValue, const char*, &TMethodSerializeValue::first>,
      std::str_less>,
    ordered_unique<tag<by_value>,
      member<TMethodSerializeValue, TSerializeMethod, &TMethodSerializeValue::second>>
      >
    > TMethodName2SerializeMethod;
  
typedef std::pair<TTagType, TType::TTypeKind> TTagTypeKind;

typedef multi_index_container<
  TTagTypeKind,
  indexed_by<
    ordered_unique<tag<by_tag>,
      member<TTagTypeKind, const char*, &TTagTypeKind::first>,
      std::str_less>,
    ordered_unique<tag<by_kind>,
      member<TTagTypeKind, TType::TTypeKind, &TTagTypeKind::second>>
      >
    > TTagType2ElemKind;
  
} } // namespace boost { namespace multi_index

namespace
{

const boost::multi_index::TMethodName2SerializeMethod MethodName2SerializeMethod =
  {
    { AUTOMATIC_SERIALIZE_MARKER, TYPE_AUTOMATIC_SERIALIZE },
    { AUTOMATIC_OBJECT_SERIALIZE_MARKER, TYPE_AUTOMATIC_OBJECT_SERIALIZE },
    { MANUAL_SERIALIZE_MARKER, TYPE_MANUAL_SERIALIZE },
    { MANUAL_POINTER_SERIALIZE_MARKER, TYPE_MANUAL_POINTER_SERIALIZE },
    { MANUAL_BUILD_SERIALIZE_MARKER, TYPE_MANUAL_BUILD_SERIALIZE },
    { MANUAL_OBJECT_SERIALIZE_MARKER, TYPE_MANUAL_OBJECT_SERIALIZE },
    { MANUAL_FULL_MARKER, TYPE_MANUAL_FULL },
    { DO_NOT_SERIALIZE_MARKER, TYPE_DO_NOT_SERIALIZE }
  };

const boost::multi_index::TTagType2ElemKind TagType2ElemKind =
  {
    { TAG_CLASS, TType::TypeClass },
    { TAG_STRUCT, TType::TypeStruct },
    { TAG_UNION, TType::TypeUnion },
    { TAG_FUNDAMENTAL_TYPE, TType::TypeFundamental },
    { TAG_TYPEDEF, TType::Typedef },
    { TAG_ELABORATED_TYPE, TType::TypeElaborated },
    { TAG_POINTER_TYPE, TType::TypePointer },
    { TAG_REFERENCE_TYPE, TType::TypeReference },
    { TAG_ARRAY_TYPE, TType::TypeArray },
    { TAG_ENUM_TYPE, TType::TypeEnum },
    { TAG_CV_QUALIFIED_TYPE, TType::TypeCvQualified },
    { TAG_FIELD, AXmlElement::TypeField },
    { TAG_NAMESPACE, AXmlElement::TypeNamespace }
  };

} // namespace

AXmlElement::AXmlElement(const std::string& name, const std::string& id, TTagType tagKind)
  : Name(name), Id(id)
  {
  auto& index = TagType2ElemKind.get<by_tag>();
  auto found = index.find(tagKind);
  assert(found != index.end());
  ElemKind = found->second;
  }

TNamespaces AXmlElement::GetNamespaceList() const
  {
  TNamespaces result;

  if (Parent)
    {
    result = Parent->GetNamespaceList();
    if (Parent->GetElemKind() == TypeNamespace)
      result.push_back(static_cast<const TNamespace*>(Parent));
    }

  return std::move(result);
  }

void TClass::SetSerializeMethod(TMethodType serializeType)
  {
  assert(SerializeMethod == TYPE_NOT_MARKED && "SetSerializeType called more than once!");
  auto& index = MethodName2SerializeMethod.get<by_name>();
  auto found = index.find(serializeType);
  assert(found != index.end());
  SerializeMethod = found->second;
  }

TMethodType TClass::GetSerializableMarker() const
  {
  auto& index = MethodName2SerializeMethod.get<by_value>();
  auto found = index.find(SerializeMethod);
  return found != index.end() ? found->first : nullptr;
  }

bool TClass::IsTemplate() const
  { return Name.find('<') != std::string::npos; }

void TClass::EvaluateSizeof() const
  {
  if (GetTypeKind() != TypeUnion || BiggestMember != nullptr)
    return;

  std::pair<int, const TClassMember*> biggest { 0, nullptr };
  ForEachMember([&biggest](const TClassMember& member)
    {
    const TType* type = member.GetType();
    int size = 1;

    while (type)
      {
      switch (type->GetTypeKind())
        {
        case TypeArray:
          size *= type->GetSizeof();
          type = static_cast<const TArrayType*>(type)->GetElemType();
          break;
        case TypeUnion:
          static_cast<const TClass*>(type)->EvaluateSizeof();
          size *= type->GetSizeof();
          type = nullptr;
          break;
        default:
          size *= type->GetSizeof();
          type = nullptr;
          break;
        }
      }

    biggest = std::max(biggest, std::make_pair(size, &member));
    });

  Sizeof = biggest.first;
  BiggestMember = biggest.second;
  }


TXmlElementsFactory* TXmlElementsFactory::GetInstance()
  {
  return new TXmlElementsFactory();
  }
