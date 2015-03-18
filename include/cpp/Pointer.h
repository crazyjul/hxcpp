#ifndef CPP_POINTER_H
#define CPP_POINTER_H

namespace cpp
{

struct AutoCast
{
   void *value;

   explicit inline AutoCast(void *inValue) : value(inValue) { }
};

Dynamic CreateDynamicPointer(void *inValue);
typedef String (*DynamicToStringFunc)(const void *);
Dynamic CreateDynamicStruct(const void *inValue, int inSize, const char *inClassName, DynamicToStringFunc inFunc);

template<typename T> class Reference;


template<typename T>
class Pointer
{
public:
   T *ptr;

   inline Pointer( ) : ptr(0) { }
   inline Pointer( const Pointer &inRHS ) : ptr(inRHS.ptr) {  }
   inline Pointer( const Dynamic &inRHS) { ptr = inRHS==null()?0: (T*)inRHS->__GetHandle(); }
   inline Pointer( const null &inRHS ) : ptr(0) { }
   inline Pointer( const T *inValue ) : ptr( (T*) inValue) { }
   //inline Pointer( T *inValue ) : ptr(inValue) { }
   inline Pointer( AutoCast inValue ) : ptr( (T*)inValue.value) { }
   inline Pointer operator=( const Pointer &inRHS ) { return ptr = inRHS.ptr; }
   inline Dynamic operator=( Dynamic &inValue )
   {
      ptr = inValue==null() ? 0 : (T*) inValue->__GetHandle();
      return inValue;
   }
   inline Dynamic operator=( null &inValue ) { ptr=0; return inValue; }
   inline AutoCast reinterpret() { return AutoCast(ptr); }

   inline bool operator==( const null &inValue ) const { return ptr==0; }
   inline bool operator!=( const null &inValue ) const { return ptr!=0; }

   // Allow '->' syntax
   inline Pointer *operator->() { return this; }
 	inline Pointer inc() { return ++ptr; }
	inline Pointer dec() { return --ptr; }
	inline Pointer add(int inInt) { return ptr+inInt; }
 	inline Pointer incBy(int inDiff) { ptr+=inDiff; return ptr; }
 	inline T &postIncRef() { return *ptr++; }
 	inline T &postIncVal() { return *ptr++; }

   inline T &at(int inIndex) { return ptr[inIndex]; }

   inline T &__get(int inIndex) { return ptr[inIndex]; }
   inline T &__set(int inIndex, const T &inValue) { T *p = ptr+inIndex; *p = inValue; return *p; }

   inline T &get_value() { return *ptr; }
   inline T &get_ref() { return *ptr; }
   inline T &set_ref(const T &inValue) { return *ptr = inValue;  }

   operator Dynamic () { return CreateDynamicPointer((void *)ptr); }
   operator T * () { return ptr; }
   T * get_raw() { return ptr; }

   inline void destroy() { delete ptr; }
   inline void destroyArray() { delete [] ptr; }

   inline bool lt(Pointer inOther) { return ptr < inOther.ptr; }
   inline bool gt(Pointer inOther) { return ptr > inOther.ptr; }
   inline bool leq(Pointer inOther) { return ptr <= inOther.ptr; }
   inline bool geq(Pointer inOther) { return ptr >= inOther.ptr; }

};

template<typename T>
inline bool operator == (const null &, Pointer<T> inPtr) { return inPtr.ptr==0; }
template<typename T>
inline bool operator != (const null &, Pointer<T> inPtr) { return inPtr.ptr!=0; }



template<typename T>
class Reference : public Pointer<T>
{
   using Pointer<T>::ptr;

public:

   inline Reference( const T &inRHS ) : Pointer<T>(&inRHS) {  }
   inline Reference( T &inRHS ) : Pointer<T>(&inRHS) {  }

   inline Reference( ) : Pointer<T>(0) { }
   inline Reference( const Reference &inRHS ) : Pointer<T>(inRHS.ptr) {  }
   inline Reference( const Dynamic &inRHS) { ptr = inRHS==null()?0: (T*)inRHS->__GetHandle(); }
   inline Reference( const null &inRHS ) : Pointer<T>(0) { }
   inline Reference( const T *inValue ) : Pointer<T>( (T*) inValue) { }
   //inline Reference( T *inValue ) : Pointer(inValue) { }
   inline Reference( AutoCast inValue ) : Pointer<T>( (T*)inValue.value) { }
   inline Reference operator=( const Reference &inRHS ) { return ptr = inRHS.ptr; }


   inline T *operator->() { return ptr; }

};

class DefaultStructHandler
{
   public:
      static inline const char *getName() { return "unknown"; }
      static inline String toString( const void *inData ) { return HX_CSTRING("Struct"); }
};

class Int64Handler : public DefaultStructHandler
{
   public:
      static inline const char *getName() { return "cpp.Int64"; }
      static inline String toString( const void *inData ) { return String( *(Int64 *)inData ); }
};


template<typename T, typename HANDLER = DefaultStructHandler >
class Struct
{
public:
   T value;
   // This allows 'StaticCast' to be used from arrays
   typedef Dynamic Ptr;

   inline Struct( ) {  }
   inline Struct( const T &inRHS ) : value(inRHS) {  }
   inline Struct( const null &) { value = T(); }

   inline Struct<T,HANDLER> &operator=( const T &inRHS ) { value = inRHS; return *this; }
   inline Struct<T,HANDLER> &operator=( const null & ) { value = T(); }
   inline Struct<T,HANDLER> &operator=( const Dynamic &inRHS ) { return *this = Struct<T,HANDLER>(inRHS); }

   operator Dynamic() const { return CreateDynamicStruct(&value,sizeof(T),HANDLER::getName(), &HANDLER::toString ); }
   operator String() const { return HANDLER::toString(value); }

   bool operator==(const Struct<T,HANDLER> &inRHS) const { return value==inRHS.value; }

   // Haxe uses -> notation
   inline T *operator->() { return &value; }

   T &get() { return value; }

   static inline bool is( const Dynamic &inRHS)
   {
      hx::Object *ptr = inRHS.mPtr;
      if (!ptr)
         return false;
      if (!ptr->__GetHandle())
         return false;
      if (ptr->__length() != sizeof(T))
         return false;
      return ptr->__CStr() == HANDLER::getName();
   }

   inline Struct( const Dynamic &inRHS)
   {
      hx::Object *ptr = inRHS.mPtr;
      if (!ptr)
      {
         value = T();
         return;
      }
      T *data = (T*)ptr->__GetHandle();
      int len = ptr->__length();
      if (!data || len<sizeof(T))
      {
         hx::NullReference("DynamicData", true);
         return;
      }
      value = *data;
   }


   inline operator T& () { return value; }

};

typedef Struct<Int64,Int64Handler> Int64Struct;



template<typename T>
class Function
{
public:
   T *call;

   inline Function( ) { }
   inline Function( const Function &inRHS ) : call(inRHS.call) {  }
   inline Function( const Dynamic &inRHS) { call = inRHS==null()?0: (T*)inRHS->__GetHandle(); }
   inline Function( const null &inRHS ) { }
   inline Function( T *inValue ) : call((T*)(inValue)) { }
   //inline Function( T *inValue ) : call(inValue) { }
   inline Function( AutoCast inValue ) : call( (T*)inValue.value) { }
   inline Function( const hx::AnyCast &inValue ) : call( (T*)inValue.mPtr) { }

   template<typename FROM>
   inline static Function __new(FROM from)
   {
      return Function(from);
   }

   inline Function operator=( const Function &inRHS ) { return call = inRHS.call; }
   inline Dynamic operator=( Dynamic &inValue )
   {
      call = inValue==null() ? 0 : (T*) inValue->__GetHandle();
   }
   inline Dynamic operator=( null &inValue ) { call=0; return inValue; }
   inline bool operator==( const null &inValue ) const { return call==0; }
   inline bool operator!=( const null &inValue ) const { return call!=0; }


   operator Dynamic () { return CreateDynamicPointer((void *)call); }
   operator T * () { return call; }
   operator void * () { return (void *)call; }

   inline T &get_call() { return *call; }

   inline bool lt(Function inOther) { return call < inOther.call; }
   inline bool gt(Function inOther) { return call > inOther.call; }
   inline bool leq(Function inOther) { return call <= inOther.call; }
   inline bool geq(Function inOther) { return call >= inOther.call; }

};


template<typename T>
inline bool operator == (const null &, Function<T> inPtr) { return inPtr.call==0; }
template<typename T>
inline bool operator != (const null &, Function<T> inPtr) { return inPtr.call!=0; }



class Function_obj
{
public:

	inline static AutoCast getProcAddress(String inLib, String inPrim)
   {
      return AutoCast(__hxcpp_get_proc_address(inLib, inPrim,false));
   }


   template<typename T>
	inline static AutoCast fromStaticFunction(T *inFunction)
   {
      return AutoCast(inFunction);
   }
};


class Pointer_obj
{
public:
   template<typename T>
	inline static AutoCast arrayElem(::Array<T> array, int inIndex)  { return AutoCast(&array[inIndex]); }
	inline static AutoCast arrayElem(Dynamic inVal, int inIndex)
   {
      if (inVal==null() || !inVal->__IsArray())
         return AutoCast(0);
      hx::ArrayBase *base = (hx::ArrayBase *)inVal.GetPtr();
      return AutoCast(base->GetBase());
   }
   template<typename T>
	inline static Pointer<T> addressOf(T &value)  { return Pointer<T>(&value); }

   template<typename T>
	inline static Pointer<T> fromPointer(T *value)  { return Pointer<T>(value); }
   template<typename T>
	inline static Pointer<T> fromPointer(const T *value)  { return Pointer<T>(value); }

   inline static AutoCast fromHandle(Dynamic inValue, String inKind)
   {
      if (inValue==null() || (inKind!=null() && inKind!=__hxcpp_get_kind(inValue)))
         return AutoCast(0);
      return AutoCast(inValue->__GetHandle());
   }
};


class Reference_obj
{
public:

};



} // end namespace cpp


#endif
