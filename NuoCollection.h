
#ifndef __NUO_COLLECTION__
#define __NUO_COLLECTION__


#include <memory>
#include <unordered_set>




class NuoStackControlBlock;
typedef std::shared_ptr<NuoStackControlBlock> PNuoStackControlBlock;

class NuoStackPtrImpl;
class NuoMemberPtrImpl;




struct NuoCollectionImpl;

class NuoCollection
{

	NuoCollectionImpl* _impl;

	long long _serialCounter;

public:

	NuoCollection();
	~NuoCollection();

	friend class NuoStackControlBlock;
	friend class NuoStackPtrImpl;
	friend class NuoMemberPtrImpl;
	friend class NuoObjectImpl;

	void CollectGarbage();

private:

	PNuoStackControlBlock CreateStackControlBlock();

	void CreateMetaTable();
	void DestroyMetaTable();
	void PushMetaTable();

};


/***********************************************************************
 *
 *  Root colllectable object implementation
 *
 ***********************************************************************/


class NuoObjectImpl
{

	/**
	 *   handle to the proxy lua object
	 */
	long long _serial;
	std::string _serialString;

	NuoCollection* _manager;
	NuoStackControlBlock* _block;

	std::unordered_set<NuoObjectImpl*> _containers;

public:

	~NuoObjectImpl();

	friend class NuoStackControlBlock;
	friend class NuoStackPtrImpl;
	friend class NuoMemberPtrImpl;

private:

	bool PushProxy();

protected:

	NuoStackPtrImpl StackPointerImpl();

};



template <class T> class NuoStackPtr;


/***********************************************************************
 *
 *  Member smart pointer implementation
 *
 ***********************************************************************/

class NuoMemberPtrImpl
{

protected:

	NuoObjectImpl* _thisObject;
	NuoObjectImpl* _memberObject;

public:

	NuoMemberPtrImpl(NuoObjectImpl* o);
	~NuoMemberPtrImpl();

protected:

	void SetMember(NuoObjectImpl* o);
	void SetMember(NuoStackPtrImpl& stackPtr);

};



template <class Base, class T>
class NuoMemberPtr : NuoMemberPtrImpl
{

public: 

	NuoMemberPtr(Base* b);

	NuoMemberPtr<Base, T>& operator = (NuoStackPtr<T>& o);
	NuoMemberPtr<Base, T>& operator = (NuoMemberPtr <Base, T>& o);

	T* operator ->();

};


template <class Base, class T>
NuoMemberPtr<Base, T>::NuoMemberPtr(Base* b)
	: NuoMemberPtrImpl(b)
{
}


template <class Base, class T>
NuoMemberPtr<Base, T>& NuoMemberPtr<Base, T>::operator = (NuoStackPtr<T>& o)
{
	SetMember(o);
	return *this;
}


template <class Base, class T>
NuoMemberPtr<Base, T>& NuoMemberPtr<Base, T>::operator = (NuoMemberPtr <Base, T>& o)
{
	SetMember(o._thisObject);
	return *this;
}


template <class Base, class T>
T* NuoMemberPtr<Base, T>::operator ->()
{
	return _memberObject;
}



/***********************************************************************
 *
 *  Stack smart pointer implementation
 *
 ***********************************************************************/

class NuoStackPtrImpl
{

protected:

	PNuoStackControlBlock _block;

	NuoStackPtrImpl(NuoObjectImpl* o, NuoCollection* manager);
	virtual ~NuoStackPtrImpl();

	NuoObjectImpl* ObjectImpl();

public:

	friend class NuoObjectImpl;
	friend class NuoMemberPtrImpl;

	void Reset();

private:

	NuoStackPtrImpl();

};



template <class T>
class NuoObject;


/***********************************************************************
 *
 *  Stack smart pointer
 *
 ***********************************************************************/


template <class T>
class NuoStackPtr : public NuoStackPtrImpl
{

public:

	NuoStackPtr(T* o, NuoCollection* manager);
	NuoStackPtr(const NuoStackPtrImpl& impl);

	T* operator ->();

	NuoStackPtr<T>& operator = (NuoStackPtr<T>& o);

	template <class Base>
	NuoStackPtr<T>& operator = (NuoMemberPtr<Base, T>& o);
};



template <class T>
NuoStackPtr<T>::NuoStackPtr(T* o, NuoCollection* manager)
	: NuoStackPtrImpl(o, manager)
{
}


template <class T>
NuoStackPtr<T>::NuoStackPtr(const NuoStackPtrImpl& impl)
{
	_block = impl._block;
}


template <class T>
T* NuoStackPtr<T>::operator ->()
{
	return (T*)ObjectImpl();
}


template <class T>
NuoStackPtr<T>& NuoStackPtr<T>::operator = (NuoStackPtr<T>& o)
{
	_block = o._block;
}


template <class T>
template <class Base>
NuoStackPtr<T>& NuoStackPtr<T>::operator = (NuoMemberPtr<Base, T>& o)
{
	NuoStackPtrImpl stackPtrImpl = o._memberObject->StackPointerImpl();
	_block = stackPtrImpl._block;
}



/***********************************************************************
 *
 *  NuoObject template base
 * 
 ***********************************************************************/

template <class T>
class NuoObject : public NuoObjectImpl
{

public:

	NuoObject();

	NuoStackPtr<T> StackPointer();

};


template <class T>
NuoObject<T>::NuoObject()
	: NuoObjectImpl()
{
}


template <class T>
NuoStackPtr<T> NuoObject<T>::StackPointer()
{
	NuoStackPtr<T> stackPtr = StackPointerImpl();
	return stackPtr;
}



#endif
