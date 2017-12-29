#include "vpi_api.h"

/// On unix, OWNER_API must be always defined to some export symbol 
#if defined(UNIX) || defined(__linux__)
  #define OWNER_API VPI_API
#else
  #define OWNER_API 
#endif 

#include <serialize3/h/client_code/serialize_macros.h>

#include <string>
#include <vector>
#include <map>

enum class TEnum1 : unsigned char
  {
  VALUE1, VALUE2
  };

enum class TEnum2 : short
  {
  VALUE1, VALUE2
  };

enum class TEnum3 : long long
  {
  VALUE1, VALUE2
  };

namespace itd
{

class ABase
  {
  private:
    SERIALIZABLE;

  public:
    ABase() = default;
    virtual void test() = 0;

  protected:
    TEnum1 m1;
    TEnum2 m2;
    TEnum3 m3;

    union
      {
      int m1;
      char m2;
      long long m3;
      } m4;

    union
      {
      int m11;
      char m12;
      long long m13;
      };
   
    struct
      {
      int m1;
      char m2;
      long long m3;
      } m5;

    struct
      {
      int m21;
      char m22;
      long long m23;
      };

    struct TStruct
      {
      private:
        SERIALIZABLE_OBJECT;
      
      public:
        int m01;
        char m02;
        long long m03;
      };

    struct : public TStruct
      {
      private:
        SERIALIZABLE_OBJECT;

      public:
        int m1;
        char m2;
        long long m3;
      } m7;

    struct : public TStruct
      {
      int m011;
      char m012;
      long long m013;
      };
  };

class TBase : public ABase
  {
  private:
    SERIALIZABLE;

  public:
    TBase() = default;
    virtual void test() override {}

  protected:
    std::vector<TStruct> m101;
    std::map<std::string, TStruct> m102;
  };

struct TStruct
  {
  private:
    SERIALIZABLE_OBJECT;

  public:
    int m1;
    char m2;
    long long m3;
    std::pair<int, char> m4;
    std::tuple<int, char, std::string> m5;
  };

class TClass final : public TBase
  {
  private:
    SERIALIZABLE;

  public:
    TClass() = default;

   itd::TStruct mm;   
 };
} // namespace itd
