
#ifndef __NUO_COLLECTION__
#define __NUO_COLLECTION__


#include <memory>




class NuoStackControlBlock;
typedef std::shared_ptr<NuoStackControlBlock> PNuoStackControlBlock;

struct NuoCollectionImpl;

class NuoCollection
{

	NuoCollectionImpl* _impl;

	long long _serialCounter;

public:

	NuoCollection();
	~NuoCollection();

	friend class NuoStackControlBlock;

private:

	PNuoStackControlBlock CreateStackControlBlock();

};



class NuoMemberPtr;
typedef std::shared_ptr<NuoMemberPtr> PNuoMemberPtr;



class NuoObject
{

	// std::vector<PNuoMemberPtr> _members;

public:

};





class NuoMemberPtr
{

	std::shared_ptr<NuoObject> _object;

public:

	NuoMemberPtr();

};





class NuoStackPtr
{

public:

	NuoStackPtr(NuoObject* o);

};



#endif
