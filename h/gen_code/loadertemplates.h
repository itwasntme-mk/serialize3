///\file loadertemplates.h
#pragma once

//Overloads "operator &" for loading different serializable ojects.
//Primitive types are individually overloaded and passed to loader.
//STL and serialize_utils types are loaded via templated overloads.
//Any other type being loaded is redirected to the object's Load function.

//Most manually written Load functions will need to include this file.

#include <serialize3/h/client_code/serialize_utils.h>
#include <serialize3/h/gen_code/serializable_std_type_includes.h>
#include <serialize3/h/storage/serializeloader.h>  

#if defined(SERIALIZABLE_BOOST_CONTAINERS)
#include <serialize3/h/gen_code/serializable_boost_cntrs_includes.h>
#endif // #if defined(SERIALIZABLE_BOOST_CONTAINERS)

#include <type_traits>

//Catchall for any object type not overloaded.
//This redirects to the object itself for loading. "Normal"
//class loads go through this operator.
//  TType expects contract of:
//    void TType::Dump(ASerializeDumper& dumper);
//    void TType::Load(ASerializeDumper& loader);
template <class TType>
#if defined(GENERATE_ENUM_OPERATORS)
void
#else
typename std::enable_if<std::is_enum<TType>::value == false>::type
#endif
operator&(ASerializeLoader& loader, TType& o)
  {
  loader.Load(o);
  }

#if !defined(GENERATE_ENUM_OPERATORS)
// Enum types
template <typename TType>
typename std::enable_if<std::is_enum<TType>::value>::type
operator&(ASerializeLoader& loader, TType& o)
  {
  static_assert(sizeof(TType) <= 64, "Too big size of enum type");
  switch (sizeof(TType))
    {
    case 8:  loader.Load((unsigned char&)o); break;
    case 16: loader.Load((unsigned short&)o); break;
    case 32: loader.Load((unsigned int&)o); break;
    case 64: loader.Load((unsigned long long&)o); break;
    default:;
    }
  }
#endif

//-------------- load primitive types
  
inline
void operator&(ASerializeLoader& loader, bool& o)
  {
  loader.Load(o);
  }

inline
void operator&(ASerializeLoader& loader, char& o)
  {
  loader.Load(o);
  }

inline
void operator&(ASerializeLoader& loader, unsigned char& o)
  {
  loader.Load(o);
  }

inline
void operator&(ASerializeLoader& loader, short& o)
  {
  loader.Load(o);
  }

inline
void operator&(ASerializeLoader& loader, unsigned short& o)
  {
  loader.Load(o);
  }

inline
void operator&(ASerializeLoader& loader, int& o)
  {
  loader.Load(o);
  }

inline
void operator&(ASerializeLoader& loader, unsigned int& o)
  {
  loader.Load(o);
  }

inline
void operator&(ASerializeLoader& loader, long long& o)
  {
  loader.Load(o);
  }

inline
void operator&(ASerializeLoader& loader, unsigned long& o)
  {
  loader.Load(o);
  }

inline
void operator&(ASerializeLoader& loader, unsigned long long& o)
  {
  loader.Load(o);
  }

inline
void operator&(ASerializeLoader& loader, double& o)
  {
  loader.Load(o);
  }

//--------------- load stl types

template <class T1, class T2> 
void operator&(ASerializeLoader& loader, std::pair<T1,T2>& p)
  {
  LPUSH_INDENT;
  LLOGMSG("Load(pair)");
  loader & p.first;
  loader & p.second;
  LPOP_INDENT;
  }

#if !defined(_MSC_VER) || (_MSC_VER >= 1912)
template <std::size_t I = 0, typename... TTypes>
typename std::enable_if<I == sizeof...(TTypes)>::type
Load(ASerializeLoader& loader, std::tuple<TTypes...>& t) {}

template <std::size_t I = 0, typename... TTypes>
typename std::enable_if<I < sizeof...(TTypes)>::type
Load(ASerializeLoader& loader, std::tuple<TTypes...>& t)
  {
  loader & std::get<I>(t);
  Load<I + 1>(loader, t);
  }

template <typename... TTypes>
void operator&(ASerializeLoader& loader, std::tuple<TTypes...>& t)
  {
  DPUSH_INDENT;
  DLOGMSG("Load(tuple)");
  Load(loader, t);
  DPOP_INDENT;
  }
#endif // !defined(_MSC_VER)

inline
void operator&(ASerializeLoader& loader, std::string& s)
  {
  loader.Load(s);
  }

#define LOAD_CNTR_SEQ_BODY(CNTR_NAME) \
  {                                   \
  LPUSH_INDENT;                       \
  LLOGMSG(CNTR_NAME);                 \
  size_t size;                        \
  loader.LoadSizeT(size);             \
  size_t i = c.size();                \
  size += i;                          \
  c.resize(size);                     \
  for (; i < size; ++i)               \
    loader & c[i];                    \
  LPOP_INDENT;                        \
  }

#define LOAD_LIST_BODY(CNTR_NAME)   \
  {                                 \
  LPUSH_INDENT;                     \
  LLOGMSG(CNTR_NAME);               \
  size_t size;                      \
  loader.LoadSizeT(size);           \
  for (size_t i = 0; i < size; ++i) \
    {                               \
    T t;                            \
    loader & t;                     \
    c.emplace_back(std::move(t));   \
    }                               \
  LPOP_INDENT;                      \
  }

#define LOAD_SET_BODY(CNTR_NAME)              \
  {                                           \
  LPUSH_INDENT;                               \
  LLOGMSG(CNTR_NAME);                         \
  size_t size;                                \
  loader.LoadSizeT(size);                     \
  if (c.empty())                              \
    {                                         \
    for (size_t i = 0; i < size; ++i)         \
      {                                       \
      Key t;                                  \
      loader & t;                             \
      c.emplace_hint(c.end(), std::move(t));  \
      }                                       \
    }                                         \
  else                                        \
    {                                         \
    for (size_t i = 0; i < size; ++i)         \
      {                                       \
      Key t;                                  \
      loader & t;                             \
      c.emplace(std::move(t));                \
      }                                       \
    }                                         \
  LPOP_INDENT;                                \
  }

#define LOAD_MAP_BODY(CNTR_NAME)                                        \
  {                                                                     \
  LPUSH_INDENT;                                                         \
  LLOGMSG(CNTR_NAME);                                                   \
  size_t size;                                                          \
  loader.LoadSizeT(size);                                               \
  if (c.empty())                                                        \
    {                                                                   \
    for (size_t i = 0; i < size; ++i)                                   \
      {                                                                 \
      std::pair<Key, Value> p;                                          \
      loader & p;                                                       \
      c.emplace_hint(c.end(), std::move(p.first), std::move(p.second)); \
      }                                                                 \
    }                                                                   \
  else                                                                  \
    {                                                                   \
    for (size_t i = 0; i < size; ++i)                                   \
      {                                                                 \
      std::pair<Key, Value> p;                                          \
      loader & p;                                                       \
      c.emplace(std::move(p.first), std::move(p.second));               \
      }                                                                 \
    }                                                                   \
  LPOP_INDENT;                                                          \
  }

template <class T,class Alloc>
void operator&(ASerializeLoader& loader, std::vector<T,Alloc>& c)
  LOAD_CNTR_SEQ_BODY("Load(vector)")

template <class T,class Alloc>
void operator&(ASerializeLoader& loader, std::deque<T,Alloc>& c)
  LOAD_CNTR_SEQ_BODY("Load(deque)")

template <class T,class Alloc>
void operator&(ASerializeLoader& loader, std::list<T,Alloc>& c)
  LOAD_LIST_BODY("Load(list)")

template <class Key, class Compare, class Alloc>
void operator&(ASerializeLoader& loader, std::set<Key,Compare,Alloc>& c)
  LOAD_SET_BODY("Load(set)")

template <class Key, class Compare, class Alloc>
void operator&(ASerializeLoader& loader, std::multiset<Key,Compare,Alloc>& c)
  LOAD_SET_BODY("Load(multiset)")

template <class Key, class Value, class Compare, class Alloc>
void operator&(ASerializeLoader& loader, std::map<Key,Value,Compare,Alloc>& c)
  LOAD_MAP_BODY("Load(map)")

template <class Key, class Value, class Compare, class Alloc>
void operator&(ASerializeLoader& loader, std::multimap<Key,Value,Compare,Alloc>& c)
  LOAD_MAP_BODY("Load(multimap)")

template <class Key, class HashFcn, class EqualKey, class Alloc> 
void operator&(ASerializeLoader& loader, std::unordered_set<Key,HashFcn,EqualKey,Alloc>& c)
  LOAD_SET_BODY("Load(unordered_set)")

template <class Key, class HashFcn, class EqualKey, class Alloc> 
void operator&(ASerializeLoader& loader, std::unordered_multiset<Key,HashFcn,EqualKey,Alloc>& c)
  LOAD_SET_BODY("Load(unordered_multiset)")

template <class Key, class Value, class HashFcn, class EqualKey, class Alloc> 
void operator&(ASerializeLoader& loader, std::unordered_map<Key,Value,HashFcn,EqualKey,Alloc>& c)
  LOAD_MAP_BODY("Load(unordered_map)")

template <class Key, class Value, class HashFcn, class EqualKey, class Alloc> 
void operator&(ASerializeLoader& loader, std::unordered_multimap<Key,Value,HashFcn,EqualKey,Alloc>& c)
  LOAD_MAP_BODY("Load(unordered_multimap)")

//--------------- load serialize_utils types

//Loads "single-reference pointers" (actual object data is loaded)
template <class T>
void operator&(ASerializeLoader& loader, TSingleRefPtr<T>& ptr)
  {
  LPUSH_INDENT;
  LLOGMSG("Load(TSingleRefPtr<T>)");
  ptr = (T*)T::LoadPointer(loader);
  LPOP_INDENT;
  }

//TNoSerializeWrapper is how we avoid serializing a data member
template <class T>
void operator&(ASerializeLoader& loader, TNoSerializeWrapper<T>&)
  {
  }

//TNoSerializePtrWrapper is how we avoid serializing a data member
template <class T>
void operator&(ASerializeLoader& loader, TNoSerializePtrWrapper<T>&)
  {
  }

template <class T>
void operator & (ASerializeLoader& loader, TSerializedObjectRegistry<T>& reg)
  {
  typedef typename TSerializedObjectRegistry<T>::TStorage TStorage;
  TStorage& storage = reg.GetStorage();

  assert(!storage.empty()); //storage always has one nul element
  //supplement existing data - should load like vector
  LPUSH_INDENT;
  LLOGMSG("Load(TSerializedObjectRegistry)");
  size_t i, size;
  loader.LoadSizeT(size);
  i = storage.size();
  size += i;
  //storage.clear();
  storage.resize(size);
#ifdef DEBUG_SERIALIZER
  char buffer[15] = "MI: ";
#endif
  for (; i < size; ++i)
    {
#ifdef DEBUG_SERIALIZER
    itoa(i,buffer+4,10);
    LLOGMSG(buffer);
#endif
    loader & storage[i];
    }
  LPOP_INDENT;
  }


#if defined(SERIALIZABLE_BOOST_CONTAINERS)

namespace bc = boost::container;
namespace bu = boost::unordered;
namespace bmi = boost::multi_index;

template <class T,class Alloc>
void operator&(ASerializeLoader& loader, bc::vector<T,Alloc>& c)
  LOAD_CNTR_SEQ_BODY("Load(boost::vector)")

template <class T,class Alloc>
void operator&(ASerializeLoader& loader, bc::deque<T,Alloc>& c)
  LOAD_CNTR_SEQ_BODY("Load(boost::deque)")

template <class T,class Alloc>
void operator&(ASerializeLoader& loader, bc::list<T,Alloc>& c)
  LOAD_LIST_BODY("Load(boost::list)")

template <class Key, class Compare, class Allocator, class SetOptions>
void operator&(ASerializeLoader& loader, bc::set<Key,Compare,Allocator,SetOptions>& c)
  LOAD_SET_BODY("Load(boost::set)")

template <class Key, class Compare, class Allocator>
void operator&(ASerializeLoader& loader, bc::flat_set<Key,Compare,Allocator>& c)
  LOAD_SET_BODY("Load(boost::flat_set)")

template <class Key, class Compare, class Allocator, class MultiSetOptions>
void operator&(ASerializeLoader& loader, bc::multiset<Key,Compare,Allocator,MultiSetOptions>& c)
  LOAD_SET_BODY("Load(boost::multiset)")

template <class Key, class Compare, class Allocator>
void operator&(ASerializeLoader& loader, bc::flat_multiset<Key,Compare,Allocator>& c)
  LOAD_SET_BODY("Load(boost::flat_multiset)")

template <class Key, class Value, class Compare, class Allocator, class MapOptions>
void operator&(ASerializeLoader& loader, bc::map<Key,Value,Compare,Allocator,MapOptions>& c)
  LOAD_MAP_BODY("Load(boost::map)")

template <class Key, class Value, class Compare, class Allocator>
void operator&(ASerializeLoader& loader, bc::flat_map<Key,Value,Compare,Allocator>& c)
  LOAD_MAP_BODY("Load(boost::flat_map)")

template <class Key, class Value, class Compare, class Allocator, class MultiMapOptions>
void operator&(ASerializeLoader& loader, bc::multimap<Key,Value,Compare,Allocator,MultiMapOptions>& c)
  LOAD_MAP_BODY("Load(boost::multimap)")

template <class Key, class Value, class Compare, class Allocator>
void operator&(ASerializeLoader& loader, bc::flat_multimap<Key,Value,Compare,Allocator>& c)
  LOAD_MAP_BODY("Load(boost::flat_multimap)")

template <class Key, class H, class P, class A>
void operator&(ASerializeLoader& loader, bu::unordered_set<Key,H,P,A>& c)
  LOAD_SET_BODY("Load(boost::unordered_set)")

template <class Key, class H, class P, class A>
void operator&(ASerializeLoader& loader, bu::unordered_multiset<Key,H,P,A>& c)
  LOAD_SET_BODY("Load(boost::unordered_multiset)")

template <class Key, class Value, class H, class P, class A>
void operator&(ASerializeLoader& loader, bu::unordered_map<Key, Value, H, P, A>& c)
  LOAD_MAP_BODY("Load(boost::unordered_map)")

template <class Key, class Value, class H, class P, class A>
void operator&(ASerializeLoader& loader, bu::unordered_multimap<Key, Value, H, P, A>& c)
  LOAD_MAP_BODY("Load(boost::unordered_multimap)")

template <typename TIndex>
struct is_sequential_index : public std::false_type {};

template <typename... TArgs>
struct is_sequential_index<bmi::random_access<TArgs...>> : public std::true_type {};

template <typename... TArgs>
struct is_sequential_index<bmi::sequenced<TArgs...>> : public std::true_type {};

template <typename TMultiIndexContainer>
struct is_first_sequential_index
  : public is_sequential_index<
      typename boost::mpl::front<
        typename TMultiIndexContainer::index_specifier_type_list>::type> {};

template <typename TIndex>
struct has_reservable_size : public std::false_type {};

template <typename... TArgs>
struct has_reservable_size<bmi::random_access<TArgs...>> : public std::true_type {};

template <typename TMultiIndexContainer>
struct has_first_index_reservable_size
  : public has_reservable_size<
      typename boost::mpl::front<
        typename TMultiIndexContainer::index_specifier_type_list>::type> {};

template <typename TMultiIndexContainer>
void Reserve(TMultiIndexContainer& c, size_t size, std::true_type has_reserve)
  {
  c.reserve(c.size() + size);
  }

template <typename TMultiIndexContainer>
void Reserve(TMultiIndexContainer& c, size_t size, std::false_type has_reserve) {}

template <typename TMultiIndexContainer>
void Load(ASerializeLoader& loader, TMultiIndexContainer& c, std::true_type is_sequential)
  {
  size_t size;
  loader.LoadSizeT(size);
  typedef typename TMultiIndexContainer::value_type Value;
  Reserve(c, size, has_first_index_reservable_size<TMultiIndexContainer>());
  for (size_t i = 0; i < size; ++i)
    {
    Value value;
    loader & value;
    c.emplace_back(std::move(value));
    }
  }

template <typename TMultiIndexContainer>
void Load(ASerializeLoader& loader, TMultiIndexContainer& c, std::false_type is_sequential)
  {
  size_t size;
  loader.LoadSizeT(size);
  typedef typename TMultiIndexContainer::value_type Value;
  if (c.empty())
    {
    for (size_t i = 0; i < size; ++i)
      {
      Value value;
      loader & value;
      c.emplace_hint(c.end(), std::move(value));
      }
    }
  else
    {
    for (size_t i = 0; i < size; ++i)
      {
      Value value;
      loader & value;
      c.emplace(std::move(value));
      }
    }
  }

template <typename Value, typename IndexSpecifierList, typename Allocator>
void operator&(ASerializeLoader& loader, bmi::multi_index_container<Value, IndexSpecifierList, Allocator>& c)
  {
  LPUSH_INDENT;
  LLOGMSG("Load(boost::multi_index_container)");
  typedef bmi::multi_index_container<Value, IndexSpecifierList, Allocator> multi_index_container;
  Load(loader, c, is_first_sequential_index<multi_index_container>());
  LPOP_INDENT;
  }

#endif // #if defined(SERIALIZABLE_BOOST_CONTAINERS)
