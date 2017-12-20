///\file xml_element.h
#pragma once

#include "constants.h"

#include <string>
#include <vector>
#include <forward_list>
#include <functional>
#include <cassert>

class TXmlElementsFactory;
class TNamespace;
typedef std::vector<const TNamespace*> TNamespaces;

class AXmlElement
  {
  public:
    enum TElemKind : unsigned char
      {
      TypeUnknown,
      TypeClass,
      TypeStruct,
      TypeUnion,
      TypeFundamental,
      Typedef,
      TypePointer,
      TypeReference,
      TypeArray,
      TypeEnum,
      TypeCvQualified,
      TypeField,
      TypeNamespace
      };

  public:
    TElemKind GetElemKind() const { return ElemKind; }

    void SetParent(const AXmlElement* parent) { Parent = parent; }

    const AXmlElement* GetParent() const { return Parent; }

    const std::string& GetName() const { return Name; }

    std::string GetFullName() const
      {
      if (Parent == nullptr)
        return GetName();
      return Parent->GetFullName() + "::" + GetName();
      }

    std::string GetFullNameWONamespaces() const
      {
      if (Parent == nullptr || Parent->GetElemKind() == TypeNamespace)
        return GetName();
      return Parent->GetFullNameWONamespaces() + "::" + GetName();
      }

    TNamespaces GetNamespaceList() const;

    bool IsPublicAccess() const { return PublicAccess != 0; }

  protected:
    AXmlElement(const std::string& name, TTagType tagKind);
    ~AXmlElement() {}

  protected:
    std::string         Name;
    const AXmlElement*  Parent = nullptr;
    TElemKind           ElemKind = TypeUnknown;
    unsigned char       Const = false;
    unsigned char       PublicAccess = true;

    friend class TXmlElementsFactory;
  };

class TNamespace final : public AXmlElement
  {
  public:
    TNamespace(const std::string& name) : AXmlElement(name, TAG_NAMESPACE) {}
  };

class TType : public AXmlElement
  {
  public:
    typedef AXmlElement::TElemKind TTypeKind;

  public:
    TType(const std::string& name, TTagType typeKind, bool publicAccess)
      : AXmlElement(name, typeKind)
      { PublicAccess = publicAccess; }

    TTypeKind GetTypeKind() const { return ElemKind; }

    void SetPointedType(const TType* _type) { Type = _type; }

    const TType* GetPointedType() const { return Type; }

    void SetConst() { Const = true; }

    bool IsConst() const { return Const != 0; }

  protected:
    const TType*  Type = nullptr; // could be null if fundamental type
  };

class TEnum final : public TType
  {
  public:
    TEnum(const std::string& name, TTagType typeKind, bool publicAccess, size_t _sizeof)
      : TType(name, typeKind, publicAccess), Sizeof(_sizeof) {}

    size_t GetSizeof() const { return Sizeof; }

  private:
    size_t  Sizeof = 0;
  };

class TArrayType final : public TType
  {
  public:
    TArrayType(const std::string& name, TTagType typeKind, bool publicAccess, int size)
      : TType(name, typeKind, publicAccess), Size(size) {}

    int GetSize() const { return Size; }

    const TType* GetElemType() const { return GetPointedType(); }

  private:
    int   Size = 0;
  };

class TClassMember final : public AXmlElement
  {
  public:
    explicit TClassMember(const std::string& name, bool publicAccess, int bitfield)
      : AXmlElement(name, TAG_FIELD), Bitfield(bitfield)
      { PublicAccess = publicAccess; }

    void SetType(const TType* _type) { Type = _type; }

    const TType* GetType() const { return Type; }

    int IsBitfield() const { return Bitfield; }

  private:
    const TType*  Type = nullptr;
    int           Bitfield = 0;
  };

class TClass final : public TType
  {
  public:
    TClass(const std::string& name, TTagType typeKind, bool publicAccess, int bases, int members)
      : TType(name, typeKind, publicAccess)
      {
      Bases.reserve(bases);
      Members.reserve(members);
      }

    void AddBase(TClass* base) { Bases.push_back(base); }
    bool HasBases() const { return Bases.empty() == false; }

    void AddDerived(TClass* base) { Derived.push_back(base); }
    bool HasDerived() const { return Derived.empty() == false; }

    bool IsPartOfHierarchy() const { return HasBases() || HasDerived(); }

    void AddMember(TClassMember* member) { Members.push_back(member); }
    bool HasMembers() const { return Members.empty() == false; }

    void SetSerializeMethod(TMethodType serializeType);

    TMethodType GetSerializableMarker() const;

    TSerializeMethod IsSerializable() const
      { return SerializeMethod != TYPE_DO_NOT_SERIALIZE ? SerializeMethod : TYPE_NOT_MARKED; }

    bool IsTemplate() const;

    void SetAbstract() { Abstract = true; }
    bool IsAbstract() const { return Abstract; }

    void SetVirtuallyDerived() { VirtuallyDerived = true; }
    bool IsVirtuallyDerived() const { return VirtuallyDerived; }

    void SetOrdered() const { Ordered = true; }
    bool IsOrdered() const { return Ordered; }

    void SetTypeId(int typeId) const { TypeId = typeId; }
    int GetTypeId() const { return TypeId; }

    bool IsDumpNeeded() const { return (SerializeMethod & TYPE_DUMP) == TYPE_DUMP;  }
    bool IsLoadNeeded() const { return (SerializeMethod & TYPE_LOAD) == TYPE_LOAD;  }
    bool IsDumpPointerNeeded() const { return (SerializeMethod & TYPE_DUMPPOINTER) == TYPE_DUMPPOINTER; }
    bool IsLoadPointerNeeded() const { return (SerializeMethod & TYPE_LOADPOINTER) == TYPE_LOADPOINTER; }
    bool IsBuildPointerNeeded() const { return (SerializeMethod & TYPE_BUILDPOINTER) == TYPE_BUILDPOINTER; }
    bool IsPointerSerializable() const { return (SerializeMethod & TYPE_SERIALIZE) != 0; }
    bool IsBuildForSerializerWithLoader() const { return (SerializeMethod & TYPE_BUILDPOINTER_W_LOADER) != 0; }

    typedef std::function<void(const TClass&)> TConstClassHandler;
    typedef std::function<void(TClass&)> TNonConstClassHandler;

    void ForEachBase(TNonConstClassHandler handler)
      {
      for (auto base : Bases)
        handler(*base);
      }

    void ForEachBase(TConstClassHandler handler) const
      {
      for (auto base : Bases)
        handler(*base);
      }

    void ForEachDerived(TNonConstClassHandler handler)
      {
      for (auto derived : Derived)
        handler(*derived);
      }

    void ForEachDerived(TConstClassHandler handler) const
      {
      for (auto derived : Derived)
        handler(*derived);
      }

    typedef std::function<void(const TClassMember&)> TMemberHandler;
    void ForEachMember(TMemberHandler handler) const
      {
      for (auto member : Members)
        handler(*member);
      }

  private:
    std::vector<TClass*>        Bases;
    std::vector<TClass*>        Derived;
    std::vector<TClassMember*>  Members;
    TSerializeMethod            SerializeMethod = TYPE_NOT_MARKED;
    mutable int                 TypeId = -1;
    bool                        Abstract = false;
    bool                        VirtuallyDerived = false;
    mutable bool                Ordered = false;
  };

class TXmlElementsFactory final
  {
  public:
    static TXmlElementsFactory* GetInstance();

    TNamespace* CreateNamespace(const std::string& name)
      {
      TNamespace* result = new TNamespace(name);
      Elements.push_front(result);
      return result;
      }

    TClass* CreateClass(const std::string& name, TTagType typeKind, bool publicAccess, int bases,
                        int members)
      {
      TClass* result = new TClass(name, typeKind, publicAccess, bases, members);
      Elements.push_front(result);
      return result;
      }

    TType* CreateType(const std::string& name, TTagType typeKind, bool publicAccess)
      {
      TType* result = new TType(name, typeKind, publicAccess);
      Elements.push_front(result);
      return result;
      }

    TEnum* CreateEnum(const std::string& name, TTagType typeKind, bool publicAccess, size_t _sizeof)
      {
      TEnum* result = new TEnum(name, typeKind, publicAccess, _sizeof);
      Elements.push_front(result);
      return result;
      }

    TArrayType* CreateArrayType(const std::string& name, TTagType typeKind, bool publicAccess, int size)
      {
      TArrayType* result = new TArrayType(name, typeKind, publicAccess, size);
      Elements.push_front(result);
      return result;
      }

    TClassMember* CreateClassMember(const std::string& name, bool publicAccess, int bitfield)
      {
      TClassMember* result = new TClassMember(name, publicAccess, bitfield);
      Elements.push_front(result);
      return result;
      }

    void DeleteElements()
      {
      for (; Elements.empty() == false; Elements.pop_front())
        {
        auto element = Elements.front();
        delete element;
        }
      }

  private:
    TXmlElementsFactory() = default;

  private:
    std::forward_list<AXmlElement*>   Elements;
  };
