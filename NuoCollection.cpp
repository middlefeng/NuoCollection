

#include "NuoCollection.h"


extern "C"
{

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

}


#include <string>


struct NuoCollectionImpl
{
	lua_State* _luaState;
};


class NuoStackControlBlock
{

	NuoCollection* _manager;

	/**
	 *   handle to the global lua object
	 */
	long long _serial;
	std::string _serialString;

	long long _memberSerialCounter;

	std::shared_ptr<NuoObject> _object;

public:

	~NuoStackControlBlock();

	friend class NuoCollection;
	friend class NuoStackPtr;

private:

	NuoStackControlBlock(NuoCollection* collection, long long serial);

	void CreateProxy();
	void InitObject(NuoObject* o);

};


NuoStackControlBlock::NuoStackControlBlock(NuoCollection* collection, long long serial)
	: _manager(collection),
	  _serial(serial),
	  _memberSerialCounter(0)
{
}


NuoStackControlBlock::~NuoStackControlBlock()
{
	lua_State* luaState = _manager->_impl->_luaState;

	lua_pushnil(luaState);
	lua_setglobal(luaState, _serialString.c_str());

	_object->_blocks.erase(this);
}


void NuoStackControlBlock::CreateProxy()
{
	char serialString[30];
	snprintf(serialString, 30, "nuoProxy%lld", _serial);

	_serialString = serialString;

	lua_State* luaState = _manager->_impl->_luaState;
	lua_newtable(luaState);
	lua_setglobal(luaState, _serialString.c_str());
}


void NuoStackControlBlock::InitObject(NuoObject* o)
{
	_object.reset(o);
	o->_blocks.insert(this);
}



NuoCollection::NuoCollection()
	: _serialCounter(0)
{
	_impl = new NuoCollectionImpl();
	_impl->_luaState = luaL_newstate();
}


NuoCollection::~NuoCollection()
{
	lua_close(_impl->_luaState);
	delete _impl;
}


PNuoStackControlBlock NuoCollection::CreateStackControlBlock()
{
	_serialCounter += 1;

	PNuoStackControlBlock block(new NuoStackControlBlock(this, _serialCounter));

	return block;
}



struct NuoStackPtrControlBlockImpl
{

};



NuoStackPtr::NuoStackPtr(NuoObject* o, NuoCollection* manager)
{
	_block = manager->CreateStackControlBlock();
	_block->InitObject(o);
}
