///\file serialize_utils.h
#pragma once

#if defined(__GNUC__) && defined(__unix__)
  #define OWNER_API_UNIXONLY __attribute__((__visibility__("default")))
#else
  #define OWNER_API_UNIXONLY
#endif

//------- TSingleRefPtr and TWrapper definitions
template <class T> 
class OWNER_API_UNIXONLY TSingleRefPtr
{
public:
  typedef T* TPtrType;
private:  
  T* Object;
public:
  TSingleRefPtr() : Object(0) {}
  TSingleRefPtr(T* ptr) : Object(ptr) {}

  TSingleRefPtr& operator =(T* ptr) { Object = ptr; return *this; }

      bool operator ==(T* ptr) const { return Object == ptr; }
      bool operator !=(T* ptr) const { return Object != ptr; }
        T& operator*() { return *Object; }
  const T& operator*() const { return *Object; }
  const T* operator ->() const { return Object; }
        T* operator ->() { return Object; }
           operator T*() const { return Object; }
       int IsNotNull() const { return Object != 0; }
  T* GetPointedObject() const { return Object; }
};

/** Class introduced to avoid serialization of data structures needed only
    during elaboration, but embedded into automatically serialized classes.
*/
template <class TWrappedType>
class OWNER_API_UNIXONLY TNoSerializeWrapper
  {
  private:
    TWrappedType Object;

  public:
    TNoSerializeWrapper() {}
    TNoSerializeWrapper(const TWrappedType& wrappedObject) : Object(wrappedObject) {}
    TNoSerializeWrapper& operator=(const TWrappedType& wrappedObject)
      {
      Object = wrappedObject;
      return *this;
      }

    operator TWrappedType& ()
      {
      return Object;
      }

    operator const TWrappedType& () const
      {
      return Object;
      }

    operator TWrappedType* ()
      {
      return &Object;
      }

    operator const TWrappedType* () const
      {
      return &Object;
      }

    TWrappedType* operator -> ()
      {
      return &Object;
      }

    const TWrappedType* operator -> () const
      {
      return &Object;
      }
  };

/** Class introduced to avoid serialization of data structures needed only
    during elaboration, but embedded into automatically serialized classes.
    This version keeps pointer so it can be freed/not created when not needed
    to avoid taking space.
    \warning Pointer is just held here, not managed (both creation and
    destruction of the object is external responsibity)
*/

template <class TWrappedType>
class OWNER_API_UNIXONLY TNoSerializePtrWrapper
  {
  private:
    TWrappedType* Pointer;

  public:
    TNoSerializePtrWrapper() : Pointer(0) {}
    TNoSerializePtrWrapper(TWrappedType* pointer) : Pointer(pointer) {}
    TNoSerializePtrWrapper& operator=(TWrappedType* pointer)
      {
      Pointer = pointer;
      return *this;
      }

    operator TWrappedType* ()
      {
      return Pointer;
      }

    operator const TWrappedType* () const
      {
      return Pointer;
      }

    TWrappedType* operator -> ()
      {
      return Pointer;
      }

    const TWrappedType* operator -> () const
      {
      return Pointer;
      }
  };
