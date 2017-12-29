///\file dumpertemplates.h
#pragma once

//Overloads "operator &" for loading different serializable ojects.
//Primitive types are individually overloaded and passed to loader.
//STL and serialize_utils types are dumped via templated overloads.
//Any other type being dumped is redirected to the object's Load function.

//Most manually written Dump functions will need to include this file.

//Note: we could achieve some more template flexibility (partial specialization)
// by using one template function that forwards to class-based 
// static function templates, but this wasn't needed
// and we skip one level of function indirection this way.

#include <serialize3/h/gen_code/serializable_std_type_includes.h>
#include <serialize3/h/client_code/serialize_macros.h>
#include <serialize3/h/client_code/serialize_ptrwrapper.h>
#include <serialize3/h/client_code/serialize_utils.h>

#include <serialize3/h/storage/serializedumper.h>

#if defined(SERIALIZABLE_BOOST_CONTAINERS)
#include <serialize3/h/gen_code/serializable_boost_cntrs_includes.h>
#endif // #if defined(SERIALIZABLE_BOOST_CONTAINERS)

#include <type_traits>

const TTypeId NULL_TYPE_ID = 0;

//Catchall for any object type not overloaded.
//This redirects to the object itself for dumping. "Normal"
//class dumps go through this operator.
//  TType expects contract of:
//    void TType::Dump(ASerializeDumper& dumper);
//    void TType::Load(ASerializeDumper& loader);
template <class TType>
#if defined(GENERATE_ENUM_OPERATORS)
void
#else
typename std::enable_if<std::is_enum<TType>::value == false>::type
#endif
operator&(ASerializeDumper& dumper, const TType& o)
  {
  o.Dump(dumper);
  }

#if !defined(GENERATE_ENUM_OPERATORS)
// Enum types
template <typename TType>
typename std::enable_if<std::is_enum<TType>::value>::type
operator&(ASerializeDumper& dumper, const TType& o)
  {
  static_assert(sizeof(TType) <= 64, "Too big size of enum type");
  switch (sizeof(TType))
    {
    case 8:  dumper.Dump((const unsigned char&)o); break;
    case 16: dumper.Dump((const unsigned short&)o); break;
    case 32: dumper.Dump((const unsigned int&)o); break;
    case 64: dumper.Dump((const unsigned long long&)o); break;
    default:;
    }
  }
#endif

//--------------- dump primitive types

inline
void operator&(ASerializeDumper& dumper, const bool o)
  {
  dumper.Dump(o);
  }

inline
void operator&(ASerializeDumper& dumper, const char o)
  {
  dumper.Dump(o);
  }

inline
void operator&(ASerializeDumper& dumper, const unsigned char o)
  {
  dumper.Dump(o);
  }

inline
void operator&(ASerializeDumper& dumper, const short o)
  {
  dumper.Dump(o);
  }

inline
void operator&(ASerializeDumper& dumper, const unsigned short o)
  {
  dumper.Dump(o);
  }


inline
void operator&(ASerializeDumper& dumper, const int o)
  {
  dumper.Dump(o);
  }

inline
void operator&(ASerializeDumper& dumper, const unsigned int o)
  {
  dumper.Dump(o);
  }

inline
void operator&(ASerializeDumper& dumper, const long long& o)
  {
  dumper.Dump(o);
  }
  
inline
void operator&(ASerializeDumper& dumper, const unsigned long o)
  {
  dumper.Dump(o);
  }

inline
void operator&(ASerializeDumper& dumper, const unsigned long long& o)
  {
  dumper.Dump(o);
  }

inline
void operator&(ASerializeDumper& dumper, const double& o)
  {
  dumper.Dump(o);
  }

//--------------- dump stl types

template <class T1, class T2> 
void operator&(ASerializeDumper& dumper, const std::pair<T1,T2>& p)
  {
  DPUSH_INDENT;
  DLOGMSG("Dump(pair)");
  dumper & p.first;
  dumper & p.second;
  DPOP_INDENT;
  }

#if !defined(_MSC_VER) || (_MSC_VER >= 1912)
template <std::size_t I = 0, typename... TTypes>
typename std::enable_if<I == sizeof...(TTypes)>::type
Dump(ASerializeDumper& dumper, const std::tuple<TTypes...>& t) {}

template <std::size_t I = 0, typename... TTypes>
typename std::enable_if<I < sizeof...(TTypes)>::type
Dump(ASerializeDumper& dumper, const std::tuple<TTypes...>& t)
  {
  dumper & std::get<I>(t);
  Dump<I + 1>(dumper, t);
  }

template <typename... TTypes>
void operator&(ASerializeDumper& dumper, const std::tuple<TTypes...>& t)
  {
  DPUSH_INDENT;
  DLOGMSG("Dump(tuple)");
  Dump(dumper, t);
  DPOP_INDENT;
  }

#endif // !defined(_MSC_VER)

inline
void operator&(ASerializeDumper& dumper, const std::string& s)
  {
  DPUSH_INDENT;
  DLOGMSG("Dump(std::string)");
  dumper.Dump(s);
  DPOP_INDENT;
  }

#define DUMP_CNTR_BODY(CNTR_NAME) \
  { \
  DPUSH_INDENT; \
  DLOGMSG(CNTR_NAME); \
  dumper.DumpSizeT(c.size()); \
  for (auto& i : c) \
    dumper & i; \
  DPOP_INDENT; \
  }

template <class T,class Alloc>
void operator&(ASerializeDumper& dumper, const std::vector<T,Alloc>& c)
  DUMP_CNTR_BODY("Dump(vector)")
   
template <class T,class Alloc>
void operator&(ASerializeDumper& dumper, const std::deque<T,Alloc>& c)
  DUMP_CNTR_BODY("Dump(deque)")
   
template <class T,class Alloc>
void operator&(ASerializeDumper& dumper, const std::list<T,Alloc>& c)
  DUMP_CNTR_BODY("Dump(list)")
   
template <class T, class Compare, class Alloc>
void operator&(ASerializeDumper& dumper, const std::set<T,Compare,Alloc>& c)
  DUMP_CNTR_BODY("Dump(set)")

template <class T, class Compare, class Alloc>
void operator&(ASerializeDumper& dumper, const std::multiset<T,Compare,Alloc>& c)
  DUMP_CNTR_BODY("Dump(multiset)")

template <class Key, class Value, class Compare, class Alloc>
void operator&(ASerializeDumper& dumper, const std::map<Key,Value,Compare,Alloc>& c)
  DUMP_CNTR_BODY("Dump(map)")

template <class Key, class Value, class Compare, class Alloc>
void operator&(ASerializeDumper& dumper, const std::multimap<Key,Value,Compare,Alloc>& c)
  DUMP_CNTR_BODY("Dump(multimap)")

template <class Key, class HashFcn, class EqualKey, class Alloc> 
void operator&(ASerializeDumper& dumper, const std::unordered_set<Key,HashFcn,EqualKey,Alloc>& c)
  DUMP_CNTR_BODY("Dump(unordered_set)")

template <class Key, class HashFcn, class EqualKey, class Alloc> 
void operator&(ASerializeDumper& dumper, const std::unordered_multiset<Key,HashFcn,EqualKey,Alloc>& c)
  DUMP_CNTR_BODY("Dump(unordered_multiset)")

template <class Key, class Value, class HashFcn, class EqualKey, class Alloc> 
void operator&(ASerializeDumper& dumper, const std::unordered_map<Key,Value,HashFcn,EqualKey,Alloc>& c)
  DUMP_CNTR_BODY("Dump(unordered_map)")

template <class Key, class Value, class HashFcn, class EqualKey, class Alloc> 
void operator&(ASerializeDumper& dumper, const std::unordered_multimap<Key,Value,HashFcn,EqualKey,Alloc>& c)
  DUMP_CNTR_BODY("Dump(unordered_multimap)")

//--------------- dumpe serialize_utils types
   
//Dump TypeId and contents of object (just dump null TypeId if pointer is null)
template <class T>
void operator&(ASerializeDumper& dumper, const TSingleRefPtr<T>& ptr)
  {
  DPUSH_INDENT;
  DLOGMSG("Dump(TSingleRefPtr<T>)");
  if (ptr.IsNotNull())
    ptr->DumpPointer(dumper);
  else
    dumper.Dump(NULL_TYPE_ID);
  DPOP_INDENT;
  }

//TNoSerializeWrapper is how we avoid serializing a data member
template <class T>
void operator&(ASerializeDumper& dumper, const TNoSerializeWrapper<T>&)
  {
  }

//TNoSerializePtrWrapper is how we avoid serializing a data member
template <class T>
void operator&(ASerializeDumper&, const TNoSerializePtrWrapper<T>&)
  {
  }

template <class T>
void operator & (ASerializeDumper& dumper, const TSerializedObjectRegistry<T>& reg)
  {
  DPUSH_INDENT;
  DLOGMSG("Dump(TSerializedObjectRegistry)");
  typedef typename TSerializedObjectRegistry<T>::TStorage TStorage;
  const TStorage& storage = reg.GetStorage();
  size_t size = storage.size();
  assert(size > 0); //storage always has one nul element
  --size;
  dumper.DumpSizeT(size);
  typename TStorage::const_iterator i = storage.begin();
  ++i; //skip first element and dump rest like regular vector
#ifdef DEBUG_SERIALIZER
  char buffer[15] = "MI: ";
#endif
  while (i != storage.end())
    {
#ifdef DEBUG_SERIALIZER
    itoa(i-storage.begin(),buffer+4,10);
    DLOGMSG(buffer);
#endif
    dumper & *i;
    ++i;
    }
  DPOP_INDENT;
  }

#if defined(SERIALIZABLE_BOOST_CONTAINERS)

namespace bc = boost::container;
namespace bu = boost::unordered;
namespace bmi = boost::multi_index;

template <class T, class Allocator>
void operator & (ASerializeDumper& dumper, const bc::list<T, Allocator>& c)
  DUMP_CNTR_BODY("Dump(boost::list)")

template <class T,class Alloc>
void operator&(ASerializeDumper& dumper, const bc::vector<T,Alloc>& c)
  DUMP_CNTR_BODY("Dump(boost::vector)")
   
template <class T,class Alloc>
void operator&(ASerializeDumper& dumper, const bc::deque<T,Alloc>& c)
  DUMP_CNTR_BODY("Dump(boost::deque)")
   
template <class Key, class Compare, class Allocator, class SetOptions>
void operator&(ASerializeDumper& dumper, const bc::set<Key,Compare,Allocator,SetOptions>& c)
  DUMP_CNTR_BODY("Dump(boost::set)")

template <class Key, class Compare, class Allocator>
void operator&(ASerializeDumper& dumper, const bc::flat_set<Key,Compare,Allocator>& c)
  DUMP_CNTR_BODY("Dump(boost::flat_set)")

template <class Key, class Compare, class Allocator, class MultiSetOptions>
void operator&(ASerializeDumper& dumper, const bc::multiset<Key,Compare,Allocator,MultiSetOptions>& c)
  DUMP_CNTR_BODY("Dump(boost::multiset)")

template <class Key, class Compare, class Allocator>
void operator&(ASerializeDumper& dumper, const bc::flat_multiset<Key,Compare,Allocator>& c)
  DUMP_CNTR_BODY("Dump(boost::flat_multiset)")

template <class Key, class T, class Compare, class Allocator, class MapOptions>
void operator&(ASerializeDumper& dumper, const bc::map<Key,T,Compare,Allocator,MapOptions>& c)
  DUMP_CNTR_BODY("Dump(boost::map)")

template <class Key, class T, class Compare, class Allocator>
void operator&(ASerializeDumper& dumper, const bc::flat_map<Key,T,Compare,Allocator>& c)
  DUMP_CNTR_BODY("Dump(boost::flat_map)")

template <class Key, class T, class Compare, class Allocator, class MultiMapOptions>
void operator&(ASerializeDumper& dumper, const bc::multimap<Key,T,Compare,Allocator,MultiMapOptions>& c)
  DUMP_CNTR_BODY("Dump(boost::multimap)")

template <class Key, class T, class Compare, class Allocator>
void operator&(ASerializeDumper& dumper, const bc::flat_multimap<Key,T,Compare,Allocator>& c)
  DUMP_CNTR_BODY("Dump(boost::flat_multimap)")

template <class T, class H, class P, class A>
void operator&(ASerializeDumper& dumper, const bu::unordered_set<T,H,P,A>& c)
  DUMP_CNTR_BODY("Dump(boost::unordered_set)")

template <class T, class H, class P, class A>
void operator&(ASerializeDumper& dumper, const bu::unordered_multiset<T,H,P,A>& c)
  DUMP_CNTR_BODY("Dump(boost::unordered_multiset)")

template <class K, class T, class H, class P, class A>
void operator&(ASerializeDumper& dumper, const bu::unordered_map<K, T, H, P, A>& c)
  DUMP_CNTR_BODY("Dump(boost::unordered_map)")

template <class K, class T, class H, class P, class A>
void operator&(ASerializeDumper& dumper, const bu::unordered_multimap<K, T, H, P, A>& c)
  DUMP_CNTR_BODY("Dump(boost::unordered_multimap)")

template<typename Value,typename IndexSpecifierList,typename Allocator>
void operator&(ASerializeDumper& dumper, const bmi::multi_index_container<Value, IndexSpecifierList, Allocator>&c)
  DUMP_CNTR_BODY("Dump(boost::multi_index_container)")

#endif // #if defined(SERIALIZABLE_BOOST_CONTAINERS)
