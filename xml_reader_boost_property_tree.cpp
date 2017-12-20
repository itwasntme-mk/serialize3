///\file xml_reader_boost_property_tree.cpp

#include "xml_reader_boost_property_tree.h"

namespace boost { namespace multi_index
{

TIdCntr TIdPool::GetIdList(const std::string& value)
  {
  TIdCntr result;

  if (value.empty())
    return result;

  assert(isspace(value[0]) == false);
  const char* _value = value.c_str();
  char buf[32];

  while (*_value != '\0')
    {
    // skip access keywords: public, protected, private
    if (*_value == 'p')
      {
      for (++_value; *_value != ':'; ++_value);
      ++_value;
      }

    size_t i = 0;
    for (; isspace(*_value) == false && *_value != '\0'; ++i, ++_value)
      buf[i] = *_value;

    buf[i] = '\0';
    result.push_back(GetId(buf));

    for (; isspace(*_value); ++_value);
    }

  return std::move(result);
  }

} } // namespace boost { namespace multi_index


namespace boost { namespace property_tree
{
std::string AXmlItemWrapper::EmptyString;
boost::multi_index::TIdPool AXmlItemWrapper::IdPool;

TMethodWrapper::TSerializeMethods TMethodWrapper::SerializeMethods =
  {
    AUTOMATIC_SERIALIZE_MARKER,
    AUTOMATIC_OBJECT_SERIALIZE_MARKER,
    MANUAL_SERIALIZE_MARKER,
    MANUAL_POINTER_SERIALIZE_MARKER,
    MANUAL_BUILD_SERIALIZE_MARKER,
    MANUAL_OBJECT_SERIALIZE_MARKER,
    MANUAL_FULL_MARKER,
    DO_NOT_SERIALIZE_MARKER,
  };

void AXmlItemWrapper::ReserveIdPool(size_t size)
  { IdPool.Reserve(size); }

} } // namespace boost { namespace property_tree
