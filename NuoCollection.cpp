

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


class NuoStackControlBlock : public std::enable_shared_from_this<NuoStackControlBlock>
{

	NuoCollection* _manager;

	/**
	 *   the global reference to the lua object proxy of "_object"
	 */
	long long _serial;
	std::string _serialString;

	long long _memberSerialCounter;

	NuoObject* _object;

public:

	~NuoStackControlBlock();

	friend class NuoCollection;
	friend class NuoStackPtr;
	friend class NuoObject;

	long long Serial() const;
	std::string SerialString() const;

private:

	NuoStackControlBlock(NuoCollection* collection, long long serial);

	void CreateProxy();
	void PushProxy();

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

	// clean the global reference to the proxy of "_object"
	//
	lua_pushnil(luaState);
	lua_setglobal(luaState, _serialString.c_str());

	_object->_block = nullptr;
}


long long NuoStackControlBlock::Serial() const
{
	return _serial;
}


std::string NuoStackControlBlock::SerialString() const
{
	return _serialString;
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


void NuoStackControlBlock::PushProxy()
{
	lua_State* luaState = _manager->_impl->_luaState;
	lua_getglobal(luaState, _serialString.c_str());
}


void NuoStackControlBlock::InitObject(NuoObject* o)
{
	_object = o;
	o->_block = this;

	PushProxy();
	
	lua_State* luaState = _manager->_impl->_luaState;
	lua_pushlightuserdata(luaState, (void*)o);
	lua_setfield(luaState, -2, "proxy");

	_manager->PushMetaTable();
	lua_setmetatable(luaState, -2);

	lua_pop(luaState, 1);
}



static int NuoObjectReclaim(lua_State* L)
{
	lua_getfield(L, -1, "proxy");
	
	if (lua_islightuserdata(L, -1))
	{
		NuoObject* o = (NuoObject*)lua_touserdata(L, -1);
		delete o;
	}

	return 0;
}


NuoCollection::NuoCollection()
	: _serialCounter(0)
{
	_impl = new NuoCollectionImpl();
	_impl->_luaState = luaL_newstate();

	CreateMetaTable();
}


NuoCollection::~NuoCollection()
{
	lua_close(_impl->_luaState);
	delete _impl;
}


void NuoCollection::CreateMetaTable()
{
	lua_State* luaState = _impl->_luaState;

	lua_newtable(luaState);
	lua_pushcclosure(luaState, NuoObjectReclaim, 0);
	lua_setfield(luaState, -2, "__gc");
	lua_setglobal(luaState, "nuoMetatable");
}


void NuoCollection::DestroyMetaTable()
{
	lua_State* luaState = _impl->_luaState;

	lua_pushnil(luaState);
	lua_setglobal(luaState, "nuoMetatable");
}


void NuoCollection::PushMetaTable()
{
	lua_State* luaState = _impl->_luaState;
	lua_getglobal(luaState, "nuoMetatable");
}


PNuoStackControlBlock NuoCollection::CreateStackControlBlock()
{
	_serialCounter += 1;

	PNuoStackControlBlock block(new NuoStackControlBlock(this, _serialCounter));

	return block;
}



NuoStackPtr::NuoStackPtr(NuoObject* o, NuoCollection* manager)
{
	_block = manager->CreateStackControlBlock();
	_block->CreateProxy();
	_block->InitObject(o);

	o->_manager = manager;
}


NuoStackPtr::NuoStackPtr()
{
}




NuoStackPtr NuoObject::StackPointer()
{
	NuoStackPtr stackPtr;
	stackPtr._block = _block->shared_from_this();

	return stackPtr;
}



bool NuoObject::PushProxy()
{
	if (_block)
	{
		_block->PushProxy();
		return true;
	}
	else
	{
		for (NuoObject* o : _containers)
		{
			if (o->PushProxy())
				return true;
		}

		return false;
	}
}
