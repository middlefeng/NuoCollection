
#ifndef __NUO_COLLECTION__
#define __NUO_COLLECTION__


#include <memory>
#include <unordered_set>




class NuoStackControlBlock;
typedef std::shared_ptr<NuoStackControlBlock> PNuoStackControlBlock;

class NuoStackPtrImpl;




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
	friend class NuoObjectImpl;

	void CollectGarbage();

private:

	PNuoStackControlBlock CreateStackControlBlock();

	void CreateMetaTable();
	void DestroyMetaTable();
	void PushMetaTable();

};



class NuoMemberPtr;
typedef std::shared_ptr<NuoMemberPtr> PNuoMemberPtr;



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

	virtual ~NuoObjectImpl();

	friend class NuoStackControlBlock;
	friend class NuoStackPtrImpl;

private:

	bool PushProxy();

protected:

	NuoStackPtrImpl StackPointerImpl();

};




class NuoMemberPtr
{

	std::shared_ptr<NuoObjectImpl> _object;

public:

	NuoMemberPtr();

};




class NuoStackPtrImpl
{

protected:

	PNuoStackControlBlock _block;

	NuoStackPtrImpl(NuoObjectImpl* o, NuoCollection* manager);
	virtual ~NuoStackPtrImpl();

	NuoObjectImpl* ObjectImpl();

public:

	friend class NuoObjectImpl;

	void Reset();

private:

	NuoStackPtrImpl();

};


template <class T>
class NuoObject;


template <class T>
class NuoStackPtr : public NuoStackPtrImpl
{

public:

	NuoStackPtr(T* o, NuoCollection* manager);
	NuoStackPtr(const NuoStackPtrImpl& impl);

	T* operator ->();
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
