
#ifndef __NUO_COLLECTION__
#define __NUO_COLLECTION__


#include <memory>
#include <unordered_set>




class NuoStackControlBlock;
typedef std::shared_ptr<NuoStackControlBlock> PNuoStackControlBlock;

class NuoStackPtr;




struct NuoCollectionImpl;

class NuoCollection
{

	NuoCollectionImpl* _impl;

	long long _serialCounter;

public:

	NuoCollection();
	~NuoCollection();

	friend class NuoStackControlBlock;
	friend class NuoStackPtr;

private:

	PNuoStackControlBlock CreateStackControlBlock();

};



class NuoMemberPtr;
typedef std::shared_ptr<NuoMemberPtr> PNuoMemberPtr;



class NuoObject
{

	std::unordered_set<NuoStackControlBlock*> _blocks;

public:

	NuoStackPtr StackPointer();

	friend class NuoStackControlBlock;

};





class NuoMemberPtr
{

	std::shared_ptr<NuoObject> _object;

public:

	NuoMemberPtr();

};





class NuoStackPtr
{

	PNuoStackControlBlock _block;

public:

	NuoStackPtr(NuoObject* o, NuoCollection* manager);

};



#endif
