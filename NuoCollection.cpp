

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

	NuoObjectImpl* _object;

public:

	~NuoStackControlBlock();

	friend class NuoCollection;
	friend class NuoStackPtrImpl;
	friend class NuoObjectImpl;

	long long Serial() const;
	std::string SerialString() const;

private:

	NuoStackControlBlock(NuoCollection* collection, long long serial);

	void CreateProxy();
	void PushProxy();

	void InitObject(NuoObjectImpl* o);

};


NuoStackControlBlock::NuoStackControlBlock(NuoCollection* collection, long long serial)
	: _manager(collection),
	  _serial(serial),
	  _memberSerialCounter(0),
	  _object(nullptr)
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


void NuoStackControlBlock::InitObject(NuoObjectImpl* o)
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
		NuoObjectImpl* o = (NuoObjectImpl*)lua_touserdata(L, -1);
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


void NuoCollection::CollectGarbage()
{
	lua_gc(_impl->_luaState, LUA_GCCOLLECT);
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




NuoMemberPtrImpl::NuoMemberPtrImpl(NuoObjectImpl* o)
	: _thisObject(o),
	  _memberObject(nullptr)
{
}


NuoMemberPtrImpl::~NuoMemberPtrImpl()
{
	SetMember(nullptr);
}


void NuoMemberPtrImpl::SetMember(NuoObjectImpl* o)
{
	if (!_thisObject->PushProxy())
		return;

	lua_State* luaState = _thisObject->_manager->_impl->_luaState;

	// dissociate the current member
	//
	if (_memberObject)
	{
		lua_pushnil(luaState);
		lua_setfield(luaState, -2, _memberObject->_serialString.c_str());
		lua_pop(luaState, 1);
	}

	_memberObject = o;

	if (!o || !o->PushProxy())
		return;
	
	lua_setfield(luaState, -2, o->_serialString.c_str());
	lua_pop(luaState, 1);
}




NuoStackPtrImpl::NuoStackPtrImpl(NuoObjectImpl* o, NuoCollection* manager)
{
	_block = manager->CreateStackControlBlock();
	_block->CreateProxy();
	_block->InitObject(o);

	o->_manager = manager;
}


NuoStackPtrImpl::~NuoStackPtrImpl()
{
	_block.reset();
}


NuoObjectImpl* NuoStackPtrImpl::ObjectImpl()
{
	return _block->_object;
}


void NuoStackPtrImpl::Reset()
{
	NuoStackPtrImpl::~NuoStackPtrImpl();
}


NuoStackPtrImpl::NuoStackPtrImpl()
{
}


NuoObjectImpl::~NuoObjectImpl()
{
}


NuoStackPtrImpl NuoObjectImpl::StackPointerImpl()
{
	NuoStackPtrImpl stackPtr;
	stackPtr._block = _block->shared_from_this();

	return stackPtr;
}



bool NuoObjectImpl::PushProxy()
{
	if (_block)
	{
		_block->PushProxy();
		return true;
	}
	else
	{
		for (NuoObjectImpl* o : _containers)
		{
			if (o->PushProxy())
			{
				// now a container's proxy is on stack, retrieve the proxy of
				// this object from its field

				lua_State* state = _manager->_impl->_luaState;
				lua_getfield(state, -1, _serialString.c_str());
				lua_remove(state, -1);

				return true;
			}
		}

		return false;
	}
}
