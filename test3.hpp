///\file test3.hpp
#pragma once

#include "vpi_api.h"

/// On unix, OWNER_API must be always defined to some export symbol 
#if defined(UNIX) || defined(__linux__)
  #define OWNER_API VPI_API
#else
  #define OWNER_API 
#endif 

#include <serialize3/h/client_code/serialize_macros.h>

enum class TEnum1 : char
  {
  VALUE1, VALUE2
  };

enum class TEnum2 : short
  {
  VALUE1, VALUE2
  };

enum class TEnum3 : int
  {
  VALUE1, VALUE2
  };

enum class TEnum4 : long long
  {
  VALUE1, VALUE2
  };

union TUnion1
  {
  private:
    SERIALIZABLE_OBJECT;

  public:
    int m1;
    TEnum1 m2;
    TEnum2 m3;
    TEnum3 m4;
    TEnum4 m5;
  };

union TUnion2
  {
  private:
    SERIALIZABLE_OBJECT;

  public:
    TUnion1 m1;
    TEnum2 m2;
  };

union TUnion3
  {
  private:
    NOT_SERIALIZABLE;

  public:
    TUnion1 m1;
    TUnion2 m2;
  };

struct TClass1
  {
  private:
    SERIALIZABLE;

  public:
    TClass1() = default;

  private:
    TUnion1 m1;
    TUnion2 m2;
    TUnion3 m3;
  };

class TClass2
  {
  private:
    SERIALIZABLE;

  public:
    TClass2() = default;

  private:
    TClass1 m;

    union
      {
      int m1;
      char m2;
      };

    union
      {
      int m1;
      char m2;
      } m3;

    struct
      {
      int m4;
      int m5;
      struct
        {
        int m6;
        int m7;
        };
      };

    struct
      {
      int m4;
      int m5;
      struct
        {
        int m6;
        int m7;
        } m6;
      } m8[3][2];
  };

class TUnion
  {
  private:
    //SERIALIZABLE;

  public:
    TUnion() = default;

  private:
    TClass1 m;
  };