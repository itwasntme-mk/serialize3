#pragma once

/** This file contains declarations of TSerializePtrType class (to be used in client code).
    Other classes are they helpers and should not be used directly - they must be placed here 
    only becasuse of C++ language needs.

    \warning Before including this file, OWNER_API macro must be defined. Preferred way is to create
             local module header in using module defining such macro and including it anywhere
             it is needed.
*/

#include <serialize3/h/client_code/serialize_macros.h>
#include <serialize3/h/client_code/serialize_utils.h>

#include <cassert>
#include <cstddef>
#include <vector>

#ifndef OWNER_API
#define OWNER_API /* Only to make VC preparser's life better */
#error Wrong usage: OWNER_API must point to API macro of module owning serialize registries - f.e. VPI_API
#endif

//#define SERIALIZED_PTR_DEBUG

#ifdef _DEBUG
  #define SERIALIZED_PTR_DEBUG
#endif


///convert "object" or "pointer to object" to "pointer to object" and throw away const
template <class T>
class OWNER_API_UNIXONLY TPtrHelper
  {
  public:
    typedef T* TPtrType;

    TPtrType operator()(const T& t) const
      {
      return const_cast<T*>(&t);
      }
  };

template <class T>
class OWNER_API_UNIXONLY TPtrHelper<TSingleRefPtr<T> >
  {
  public:
    typedef T* TPtrType;

    TPtrType operator()(const TSingleRefPtr<T>& singleRefPtr) const
      {
      return const_cast< TSingleRefPtr<T>& >(singleRefPtr);
      }
  };


/** To simplify serialization pointers to some object will be stored and
    registered here. Referencing members will contain THandle instead which is
    a primitive type.
    DLN Note: Objects as well as object pointers can be stored in the
    registry's vector. The type stored is generally dictated by TSerializePtrWrapper::TStoredType
    trait. TPtrHelper is used to convert any internally stored objects to pointers to those objects.
*/
template <class TStored>
class OWNER_API TSerializedObjectRegistry
  {
  public:
    typedef TStored                            TStoredType; //trait for outside use    
    typedef size_t                             THandle;     //index into registry
    typedef TSerializedObjectRegistry<TStored> TRegistry;   //short alias for this class
    typedef std::vector<TStored>               TStorage;    //storage for registered objects

    typedef typename TPtrHelper<TStored>::TPtrType TPtrType; //pointer to "stored" object

    static TRegistry* GetInstance()
      {
      TRegistry** instancePlace = GetInstancePlace();
      return *instancePlace;
      }

    static TRegistry* SwitchInstance(TRegistry* newRegistry)
      {
      TRegistry** instancePlace = GetInstancePlace();
      TRegistry* oldInstance = *instancePlace;
      
      *instancePlace = newRegistry;
      
      return oldInstance;
      }

    TSerializedObjectRegistry() :
      Storage(1), HandleOffset(0)
      {
      TRegistry** instancePlace = GetInstancePlace();
      *instancePlace = this;
      }

    /// Destructor must be public because of serialize purposes.
    virtual ~TSerializedObjectRegistry()
      {
      TRegistry** instancePlace = GetInstancePlace();
      assert(*instancePlace == NULL || *instancePlace == this);
      *instancePlace = NULL;
      }

    /// Returns a handle being associated with next registered object.
    THandle GetNextHandle() const
      {
      if(FreeHandles.empty())
        return Storage.size();
      else
        return FreeHandles.back();
      }

    void    ReleaseHandle(THandle h)
      {
      Storage[h] = TStored();
      FreeHandles.push_back(h);
      }

    THandle AddToRegistry(const TStored& object)
      {
      THandle handle = 0;

      if(FreeHandles.empty())
        {
        handle = Storage.size();
        Storage.push_back(object);
        }
      else
        {
        handle = FreeHandles.back();
        Storage[handle] = object;
        FreeHandles.pop_back();
        }

      return handle;
      }

    TPtrType GetRegisteredObject(THandle handle) const
      {
      assert(handle < Storage.size());
      TPtrHelper<TStored> ptrHelper;
      return ptrHelper(Storage[handle]);
      }

    /// Clears all registered objects (but don't frees memory of the objects !!!).
    void Clear()
      {
      Storage.clear();
      Storage = TStorage(1);
      FreeHandles.clear();
      }

    THandle GetLoadOffset() const
      {
      return HandleOffset;
      }

    void SetLoadOffset(THandle offset)
      {
      HandleOffset = offset;
      }

    TStorage& GetStorage()
      {
      return Storage;
      }

    const TStorage& GetStorage() const
      {
      return Storage;
      }

  protected:
    static TRegistry** GetInstancePlace()
      {
#if defined(__GNUC__)
# pragma GCC visibility push(default)
#endif
      static TRegistry* Instance = NULL;
#if defined(__GNUC__)      
#pragma GCC visibility pop
#endif
      return &Instance;
      }

  /// Class attributes:
  private:
    typedef std::vector<THandle> THandleContainer;

    TStorage         Storage;
    THandleContainer FreeHandles;
    THandle          HandleOffset;
  }; //TSerializedObjectRegistry

/// Base class for TSerializePtrWrapper template class.
class OWNER_API APtrWrapper
  {
  SERIALIZABLE_OBJECT
  public:
    typedef size_t THandle;

    APtrWrapper& operator = (THandle h) 
      {
      Handle = h;
      return *this;
      }

    /// Operator needed to properly handle use of TPtrWrappers as map keys.
    bool operator < (const APtrWrapper& rhs) const
      {
      return Handle < rhs.Handle;
      }

  protected:
    APtrWrapper(THandle h = 0) : Handle(h) {}

  protected:
    THandle Handle;
  };

template <class TType, class TStorageType,
          class TRegistryType = TSerializedObjectRegistry<TStorageType> >
class OWNER_API_UNIXONLY TSerializePtrWrapper : public APtrWrapper
  {
  public:
    typedef typename TPtrHelper<TType>::TPtrType   TPtrType;
    typedef TStorageType                           TStoredType;
    typedef TRegistryType                          TRegistry;
    typedef typename TRegistry::THandle            THandle;

    static TSerializePtrWrapper<TType, TStorageType, TRegistryType> WrapNewObject(
      const TStoredType& object)
      {
      TRegistry* registry = GetRegistry();
      THandle handle = registry->AddToRegistry(object);
      TSerializePtrWrapper<TType, TStorageType, TRegistryType> retVal;
      retVal.SetHandle(handle);
      return retVal;
      }

    static const TSerializePtrWrapper<TType, TStorageType, TRegistryType>& Null()
      {
#if defined(__GNUC__)
# pragma GCC visibility push(default)
#endif
      static TSerializePtrWrapper<TType, TStorageType, TRegistryType> null;
#if defined(__GNUC__)      
#pragma GCC visibility pop
#endif
      return null;
      }

    TSerializePtrWrapper() : APtrWrapper(0)
      {
#ifdef SERIALIZED_PTR_DEBUG
      Object = GetPointedObject();
#endif
      }

    void     SetHandle(THandle h)
      {
      this->Handle = h;
#ifdef SERIALIZED_PTR_DEBUG
      Object = GetPointedObject();
#endif
      }

    THandle  GetHandle() const
      {
      return this->Handle;
      }

    const TPtrType operator -> () const
      {
      return GetPointedObject();
      }

    TPtrType operator -> ()
      {
      return GetPointedObject();
      }

    operator TPtrType () 
      {
      return GetPointedObject();
      }

    operator const TPtrType () const
      {
      return GetPointedObject();
      }

    template <class TTarget>
    operator TSerializePtrWrapper<TTarget, TStorageType, TRegistryType>() const
      {
      TSerializePtrWrapper<TTarget, TStorageType, TRegistryType> retVal;
      retVal.SetHandle(GetHandle());

      /// \warning TType must be convertable to TTarget type.
      assert(static_cast<const TTarget*>(retVal.GetPointedObject()) == GetPointedObject());

      return retVal;
      }

    void Load(ASerializeLoader& loader)
      {
      APtrWrapper::Load(loader);
      if(this->Handle != 0)
        this->Handle += GetRegistry()->GetLoadOffset();
      }

  protected:
    TPtrType GetPointedObject() const
      {
      /** Put Object updating also here, to simplify debugging runtime code.
          It helps to solve this problem after loading serialized data (Object
          cannot be updated just after reading a handle from serialization
          database).
      */
      if(this->Handle == 0)
        {
#ifdef SERIALIZED_PTR_DEBUG
        Object = NULL;
#endif
        return NULL;
        }
      else
        {
        const TRegistryType* registry = GetRegistry();
#ifdef SERIALIZED_PTR_DEBUG
        Object = (TPtrType)registry->GetRegisteredObject(this->Handle);
#endif
        return (TPtrType)registry->GetRegisteredObject(this->Handle);
        }
      }

    static TRegistryType* GetRegistry()
      {
      assert(TRegistryType::GetInstance() != NULL);
      return TRegistryType::GetInstance();
      }

  /// Class attributes:
  private:
#ifdef SERIALIZED_PTR_DEBUG
    mutable TPtrType Object;
#endif
  };
