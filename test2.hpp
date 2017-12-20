#include <serialize3/h/client_code/serialize_macros.h>

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
    int m2 : 3;
    int m3[3];
    int m4[3][4];

    virtual bool test() = 0;
  };

class TMyClass : protected ABase
  {
  SERIALIZABLE
  public:
    int get() { return m1; }

    virtual bool test() { return true; }

  public:
    enum TEnumType
      {
      VALUE1,
      VALUE2
      };
  };

template <typename TClass>
class TTemplate
  {
  SERIALIZABLE
  public:
    TClass m1;
  };

typedef TTemplate<int> TIntTemplate;
typedef TTemplate<TMyClass> TMyTemplate;

class TMyClass1 : public virtual ABase, private virtual TMyTemplate
  {
  SERIALIZABLE
  public:
    const TIntTemplate mm1;
    TMyTemplate  mm2;
    TMyClass mm3;

    virtual bool test() { return false; }
  };
} // namespace xtd
