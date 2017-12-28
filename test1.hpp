#include "vpi_api.h"

/// On unix, OWNER_API must be always defined to some export symbol 
#ifdef UNIX
  #define OWNER_API VPI_API
#else
  #define OWNER_API 
#endif 

#include <serialize3/h/client_code/serialize_macros.h>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/identity.hpp>

#include <boost/container/list.hpp>
#include <boost/container/vector.hpp>
#include <boost/container/deque.hpp>
#include <boost/container/set.hpp>
#include <boost/container/map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include <string>

enum TEnumType
  {
  VALUE1,
  VALUE2
  };

namespace xtd
{
enum TEnumType
  {
  VALUE1,
  VALUE2
  };

class ABase
  {
  SERIALIZABLE
  protected:
    int m1;
    //int m2 : 3;
    int m3[3];
    int m4[3][4];

    virtual bool test() = 0;

    ABase() : m1(0) {}
  };

class TMyClass : public ABase
  {
  SERIALIZABLE
  public:
    TMyClass() : ABase() {};
    int get() { return m1; }

    virtual bool test() { return true; }

  public:
    enum TEnumType
      {
      VALUE1,
      VALUE2
      };

    TEnumType   mm1;

    boost::multi_index::multi_index_container<
      std::string,
      boost::multi_index::indexed_by<
        boost::multi_index::random_access<>,
        boost::multi_index::ordered_unique<boost::multi_index::identity<std::string>>
      >
    > M1;

    boost::multi_index::multi_index_container<
      std::string,
      boost::multi_index::indexed_by<
        boost::multi_index::ordered_unique<boost::multi_index::identity<std::string>>
      >
    > M2;

    boost::multi_index::multi_index_container<
      std::string,
      boost::multi_index::indexed_by<
        boost::multi_index::sequenced<>,
        boost::multi_index::ordered_unique<boost::multi_index::identity<std::string>>
      >
    > M3;

    boost::multi_index::multi_index_container<
      std::string,
      boost::multi_index::indexed_by<
        boost::multi_index::hashed_non_unique<boost::multi_index::identity<std::string>>
      >
    > M4;
  };

template <typename TClass>
class TTemplate
  {
  SERIALIZABLE
  public:
    TClass m1;

#if !defined(_MSC_VER) || (_MSC_VER >= 1912)
    std::tuple<int, std::string, double> m2;
#endif
  };

typedef TTemplate<int> TIntTemplate;
typedef TTemplate<TMyClass> TMyTemplate;

class TMyClass1 : public TMyClass
  {
  SERIALIZABLE
  public:
    TMyClass1() : TMyClass() {}
    TIntTemplate mm1;
    TMyTemplate  mm2;
    TMyClass mm3;

    virtual bool test() { return false; }

    boost::container::list<std::string> M1;
    boost::container::vector<std::string> M2;
    boost::container::deque<std::string> M3;
    boost::container::flat_set<std::string> M4;
    boost::container::set<std::string> M5;
    boost::container::flat_multiset<std::string> M6;
    boost::container::multiset<std::string> M7;
    boost::container::map<std::string, int> M8;
    boost::container::flat_map<std::string, int> M9;
    boost::container::multimap<std::string, int> M10;
    boost::container::flat_multimap<std::string, int> M11;
    boost::unordered::unordered_set<std::string> M12;
    boost::unordered::unordered_multiset<std::string> M13;
    boost::unordered::unordered_map<std::string, int> M14;
    boost::unordered::unordered_multimap<std::string, int> M15;
  };
} // namespace xtd
