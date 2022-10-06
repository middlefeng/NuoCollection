
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
	friend class NuoObject;

	void CollectGarbage();

private:

	PNuoStackControlBlock CreateStackControlBlock();

	void CreateMetaTable();
	void DestroyMetaTable();
	void PushMetaTable();

};



class NuoMemberPtr;
typedef std::shared_ptr<NuoMemberPtr> PNuoMemberPtr;



class NuoObject
{

	/**
	 *   handle to the proxy lua object
	 */
	long long _serial;
	std::string _serialString;

	NuoCollection* _manager;
	NuoStackControlBlock* _block;

	std::unordered_set<NuoObject*> _containers;

public:

	virtual ~NuoObject();

	NuoStackPtrImpl StackPointer();

	friend class NuoStackControlBlock;
	friend class NuoStackPtrImpl;

private:

	bool PushProxy();

};





class NuoMemberPtr
{

	std::shared_ptr<NuoObject> _object;

public:

	NuoMemberPtr();

};




class NuoStackPtrImpl
{

	PNuoStackControlBlock _block;

public:

	NuoStackPtrImpl(NuoObject* o, NuoCollection* manager);
	~NuoStackPtrImpl();

	friend class NuoObject;

	void Reset();

private:

	NuoStackPtrImpl();

};






#endif
