///\file xml_reader_boost_property_tree.h - xml reader based on boost xml reader and property tree
#pragma once

#include "constants.h"
#include "str_less.h"

#include <boost/property_tree/xml_parser.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/mem_fun.hpp>

#include <cassert>
#include <vector>
#include <set>

typedef std::vector<size_t> TIdCntr;

namespace boost { namespace multi_index
{

struct by_id;
struct by_content;

class TIdPool
  {
  public:
    void Reserve(size_t size)
      { Pool.reserve(size); }

    size_t GetId(const std::string& id)
      { return GetId(id.c_str()); }

    size_t GetId(const char* id)
      {
      if (*id == '\0')
        return -1;
      auto insert_info = Pool.get<by_content>().emplace(id);
      auto _id = std::distance(Pool.begin(), project<by_id>(Pool, insert_info.first));
      return _id;
      }

    const std::string* GetIdStr(size_t id) const
      {
      if (id >= Pool.size())
        return nullptr;
      return &Pool[id];
      }

    TIdCntr GetIdList(const std::string& value);

  private:
    typedef multi_index_container<
      std::string,
      indexed_by<
        random_access<tag<by_id>>,
        ordered_unique<
          tag<by_content>,
          const_mem_fun<std::string, const char*, &std::string::c_str>,
          std::str_less>
        >
      > string_pool_t;

    string_pool_t Pool;
  };

} } // namespace boost { namespace multi_index

namespace boost { namespace property_tree
{

// Helpers to retrieve xml attributies.
class AXmlItemWrapper
  {
  public:
    static void ReserveIdPool(size_t size);

    const std::string& GetName() const
      { return get_attr_value(ATTRIBUTE_NAME); }

    const std::string& GetIdStr() const
      { return get_attr_value(ATTRIBUTE_ID); }

    size_t GetId() const
      { return IdPool.GetId(GetIdStr()); }

    size_t GetContextId() const
      { return IdPool.GetId(get_attr_value(ATTRIBUTE_CONTEXT)); }

    bool IsPublicAccess() const
      {
      const std::string& access = get_attr_value(ATTRIBUTE_ACCESS);
      return access.empty() || access == "public";
      }

    static const std::string& GetIdStr(size_t id)
      {
      const std::string* id_str = IdPool.GetIdStr(id);
      return id_str ? *id_str : EmptyString;
      }

  protected:
    const std::string& get_attr_value(const char* attr_name) const
      {
      auto const& value = XmlItem.get_child_optional(attr_name);
      return value ? value.get().data() : EmptyString;
      };

  protected:
    explicit AXmlItemWrapper(const ptree::value_type& xmlItem) : XmlItem(xmlItem.second.begin()->second) {}

  protected:
    const ptree&        XmlItem;

    static boost::multi_index::TIdPool  IdPool;
    static std::string                  EmptyString;
  };

class TFieldWrapper : public AXmlItemWrapper
  {
  public:
    explicit TFieldWrapper(const ptree::value_type& xmlItem) : AXmlItemWrapper(xmlItem) {}

    size_t GetTypeId() const
      { return IdPool.GetId(get_attr_value(ATTRIBUTE_TYPE)); }

    int IsBitfield() const
      {
      const std::string& bitfield = get_attr_value(ATTRIBUTE_BITS);
      return bitfield.empty() ? 0 : std::stoi(bitfield);
      }
  };

class TEnumWrapper : public AXmlItemWrapper
  {
  public:
    explicit TEnumWrapper(const ptree::value_type& xmlItem) : AXmlItemWrapper(xmlItem) {}

    // sizeof in bits
    int GetSizeof() const
      {
      const std::string& _sizeof = get_attr_value(ATTRIBUTE_SIZE);
      return _sizeof.empty() ? 0 : std::stoi(_sizeof);
      }
  };

class TNamespaceWrapper : public AXmlItemWrapper
  {
  public:
    explicit TNamespaceWrapper(const ptree::value_type& xmlItem) : AXmlItemWrapper(xmlItem) {}

    TIdCntr GetMembers() const
      { return IdPool.GetIdList(get_attr_value(ATTRIBUTE_MEMBERS)); }
  };

class TClassWrapper : public TNamespaceWrapper
  {
  public:
    explicit TClassWrapper(const ptree::value_type& xmlItem) : TNamespaceWrapper(xmlItem) {}

    TIdCntr GetBases() const
      { return IdPool.GetIdList(get_attr_value(ATTRIBUTE_BASES)); }

    bool IsIncomplete() const
      { return get_attr_value(ATTRIBUTE_INCOMPLETE).empty() == false; }

    bool IsAbstract() const
      { return get_attr_value(ATTRIBUTE_ABSTRACT).empty() == false; }
  };

class TBaseClassWrapper : public AXmlItemWrapper
  {
  public:
    explicit TBaseClassWrapper(const ptree::value_type& xmlItem) : AXmlItemWrapper(xmlItem) {}

    bool IsVirtuallyDerived() const
      { return get_attr_value(ATTRIBUTE_VIRTUAL).empty() == false; }

    size_t GetTypeId() const
      {
      auto _type = get_attr_value(ATTRIBUTE_TYPE);
      assert(_type.empty() == false);
      return IdPool.GetId(_type);
      }
  };

class TTypeWrapper : public AXmlItemWrapper
  {
  public:
    explicit TTypeWrapper(const ptree::value_type& xmlItem) : AXmlItemWrapper(xmlItem) {}

    size_t GetTypeId() const
      {
      auto _type = get_attr_value(ATTRIBUTE_TYPE);
      return _type.empty() ? -1 : IdPool.GetId(_type);
      }

    bool IsConst() const
      { return get_attr_value(ATTRIBUTE_CONST).empty() == false; }

    bool GetMin(int& minValue) const
      {
      std::string value(get_attr_value(ATTRIBUTE_MIN));

      if (value.empty() == false)
        {
        minValue = std::stoi(value);
        return true;
        }

      return false;
      }

    bool GetMax(int& maxValue) const
      {
      std::string value(get_attr_value(ATTRIBUTE_MAX));

      if (value.empty() == false)
        {
        maxValue = std::stoi(value);
        return true;
        }

      return false;
      }

    size_t GetElemTypeId() const
      { return GetTypeId(); }
  };

class TMethodWrapper : public AXmlItemWrapper
  {
  public:
    explicit TMethodWrapper(const ptree::value_type& xmlItem) : AXmlItemWrapper(xmlItem) {}

    TMethodType IsSerializeMethod() const
      {
      auto found = SerializeMethods.find(GetName().c_str());
      return found != SerializeMethods.end() ? *(found) : nullptr;
      }

  private:
    typedef std::set<TMethodType, std::str_less> TSerializeMethods;
    static TSerializeMethods SerializeMethods;
  };

} } // namespace boost { namespace property_tree
