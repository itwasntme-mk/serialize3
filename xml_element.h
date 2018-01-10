///\file xml_element.h - contains definitions of objects created while xml analysis
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
      TypeElaborated,
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

    const std::string& GetId() const { return Id; }

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
    AXmlElement(const std::string& name, const std::string& id, TTagType tagKind);
    ~AXmlElement() {}

  protected:
    std::string         Name;
    const std::string&  Id;
    const AXmlElement*  Parent = nullptr;
    TElemKind           ElemKind = TypeUnknown;
    unsigned char       Const = false;
    unsigned char       PublicAccess = true;

    friend class TXmlElementsFactory;
  };

class TNamespace final : public AXmlElement
  {
  public:
    TNamespace(const std::string& name, const std::string& id) : AXmlElement(name, id, TAG_NAMESPACE) {}
  };

class TType : public AXmlElement
  {
  public:
    typedef AXmlElement::TElemKind TTypeKind;

  public:
    TType(const std::string& name, const std::string& id, TTagType typeKind, bool publicAccess, int _sizeof)
      : AXmlElement(name, id, typeKind), Sizeof(_sizeof)
      {
      PublicAccess = publicAccess;
      }

    TTypeKind GetTypeKind() const { return ElemKind; }

    void SetPointedType(const TType* _type) { Type = _type; }

    const TType* GetPointedType() const { return Type; }

    void SetConst() { Const = true; }

    bool IsConst() const { return Const != 0; }

    int GetSizeof() const { return Sizeof; }

    bool IsTypedef() const { return ElemKind == Typedef || ElemKind == TypeElaborated; }

  protected:
    const TType*  Type = nullptr; // could be null if fundamental type
    int           Sizeof = 0;
  };

class TEnum final : public TType
  {
  public:
    TEnum(const std::string& name, const std::string& id, TTagType typeKind, bool publicAccess, int _sizeof)
      : TType(name, id, typeKind, publicAccess, _sizeof) {}
  };

class TArrayType final : public TType
  {
  public:
    TArrayType(const std::string& name, const std::string& id, TTagType typeKind, bool publicAccess, int size)
      : TType(name, id, typeKind, publicAccess, size) {}

    int GetSize() const { return GetSizeof(); }

    const TType* GetElemType() const { return GetPointedType(); }
  };

class TClassMember final : public AXmlElement
  {
  public:
    explicit TClassMember(const std::string& name, const std::string& id, bool publicAccess, int bitfield)
      : AXmlElement(name, id, TAG_FIELD), Bitfield(bitfield)
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
    TClass(const std::string& name, const std::string& id, TTagType typeKind, bool publicAccess, int bases, int members)
      : TType(name, id, typeKind, publicAccess, 0)
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
      { return SerializeMethod; }

    bool NeedGenerateSerializeCode() const
      { return SerializeMethod != TYPE_DO_NOT_SERIALIZE && SerializeMethod != TYPE_NOT_MARKED; }

    bool IsTemplate() const;

    void SetAbstract() { Abstract = true; }
    bool IsAbstract() const { return Abstract; }

    void SetVirtuallyDerived() { VirtuallyDerived = true; }
    bool IsVirtuallyDerived() const { return VirtuallyDerived; }

    void SetOrdered() const { Ordered = true; }
    bool IsOrdered() const { return Ordered; }

    void SetTypeId(int typeId) const { TypeId = typeId; }
    int GetTypeId() const { return TypeId; }

    void ChooseBiggestMember() const;
    const TClassMember* GetBiggestUnionMember() const
      { return BiggestMember; }
    int GetUnionSizeof() const
      {
      const TType* type = nullptr;
      if (BiggestMember == nullptr || (type = BiggestMember->GetType()) == nullptr)
        return 0;
      return type->GetSizeof();
      }

    bool IsDumpNeeded() const { return (SerializeMethod & TYPE_DUMP) == TYPE_DUMP;  }
    bool IsLoadNeeded() const { return (SerializeMethod & TYPE_LOAD) == TYPE_LOAD;  }
    bool IsDumpPointerNeeded() const { return (SerializeMethod & TYPE_DUMPPOINTER) == TYPE_DUMPPOINTER; }
    bool IsLoadPointerNeeded() const { return (SerializeMethod & TYPE_LOADPOINTER) == TYPE_LOADPOINTER; }
    bool IsBuildPointerNeeded() const { return (SerializeMethod & TYPE_BUILDPOINTER) == TYPE_BUILDPOINTER; }
    bool IsPointerSerializable() const { return (SerializeMethod & TYPE_SERIALIZE) != 0; }
    bool IsBuildForSerializerWithLoader() const { return (SerializeMethod & TYPE_BUILDPOINTER_W_LOADER) != 0; }
    bool IsObjectSerializable() const { return (SerializeMethod & TYPE_OBJECT_SERIALIZE) != 0; }

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
    /// Valid only for Union. Member with biggest sizeof.
    mutable const TClassMember* BiggestMember = nullptr;
    /// Set and used with TSerializableMap.
    mutable int                 TypeId = -1;
    bool                        Abstract = false;
    bool                        VirtuallyDerived = false;
    mutable bool                Ordered = false;
  };

class TXmlElementsFactory final
  {
  public:
    static TXmlElementsFactory* GetInstance();

    TNamespace* CreateNamespace(const std::string& name, const std::string& id)
      {
      TNamespace* result = new TNamespace(name, id);
      Elements.push_front(result);
      return result;
      }

    TClass* CreateClass(const std::string& name, const std::string& id, TTagType typeKind, bool publicAccess, int bases,
                        int members)
      {
      TClass* result = new TClass(name, id, typeKind, publicAccess, bases, members);
      Elements.push_front(result);
      return result;
      }

    TType* CreateType(const std::string& name, const std::string& id, TTagType typeKind, bool publicAccess, int _sizeof)
      {
      TType* result = new TType(name, id, typeKind, publicAccess, _sizeof);
      Elements.push_front(result);
      return result;
      }

    TEnum* CreateEnum(const std::string& name, const std::string& id, TTagType typeKind, bool publicAccess, int _sizeof)
      {
      TEnum* result = new TEnum(name, id, typeKind, publicAccess, _sizeof);
      Elements.push_front(result);
      return result;
      }

    TArrayType* CreateArrayType(const std::string& name, const std::string& id, TTagType typeKind, bool publicAccess, int size)
      {
      TArrayType* result = new TArrayType(name, id, typeKind, publicAccess, size);
      Elements.push_front(result);
      return result;
      }

    TClassMember* CreateClassMember(const std::string& name, const std::string& id, bool publicAccess, int bitfield)
      {
      TClassMember* result = new TClassMember(name, id, publicAccess, bitfield);
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
