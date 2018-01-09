#pragma once

#define OWNER_API
#include <serialize3/h/client_code/serialize_macros.h>

typedef int type1;
typedef type1 type2;
typedef type2 type3[3];

class TClass
  {
  SERIALIZABLE;
  public:
    TClass() = default;

  protected:
    type1 m1;
    type2 m2;
    type3 m3;
    int   m4 : 4;
  };
