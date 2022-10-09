

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



/**
 *   stack references are managed by reference-counting
 */
class NuoStackControlBlock : public std::enable_shared_from_this<NuoStackControlBlock>
{

	NuoCollection* _manager;

	/**
	 *   the global reference to the lua object proxy of "_object"
	 */
	long long _serial;
	std::string _serialString;

	NuoObjectImpl* _object;

public:

	~NuoStackControlBlock();

	friend class NuoCollection;
	friend class NuoStackPtrImpl;
	friend class NuoObjectImpl;

	long long Serial() const;
	std::string SerialString() const;

private:

	// construct a new block with a new serial
	//
	NuoStackControlBlock(NuoCollection* collection, long long serial);

	//  re-construct a block for an object
	//
	NuoStackControlBlock(NuoObjectImpl* o);

	void CreateProxy();
	void PushProxy();

	void InitObject(NuoObjectImpl* o);

};


NuoStackControlBlock::NuoStackControlBlock(NuoCollection* collection, long long serial)
	: _manager(collection),
	  _serial(serial),
	  _object(nullptr)
{
}


NuoStackControlBlock::NuoStackControlBlock(NuoObjectImpl* o)
	: _manager(o->_manager),
	  _serial(o->_serial),
	  _object(o)
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
	o->_serial = _serial;
	o->_serialString = _serialString;

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
	_thisObject->AddMember(this);
}


NuoMemberPtrImpl::~NuoMemberPtrImpl()
{
	SetMember(nullptr);
}


void NuoMemberPtrImpl::SetMember(NuoObjectImpl* o)
{
	// this has to be done regardless the proxy exists or not
	//
	if (_memberObject)
	{
		_memberObject->_containers.erase(_thisObject);
	}

	if (!_thisObject->PushProxy())
	{
		// this has to be done even when the proxy has been destroyed
		//
		_memberObject = o;

		return;
	}

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
	o->_containers.insert(_thisObject);

	if (!o || !o->PushProxy())
		return;
	
	lua_setfield(luaState, -2, o->_serialString.c_str());
	lua_pop(luaState, 1);
}


void NuoMemberPtrImpl::SetMember(NuoStackPtrImpl& stackPtr)
{
	SetMember(stackPtr.ObjectImpl());
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
	bool deletionHappend = false;

	do
	{
		deletionHappend = false;

		for (NuoObjectImpl* object : _containers)
		{
			for (NuoMemberPtrImpl* member : object->_members)
			{
				if (member->_memberObject == this)
				{
					member->SetMember(nullptr);
					deletionHappend = true;
				}
			}

			// if a deletion of a relationship happens, that means "_containers" must
			// have changed, the iteration has to be terminated and started over again
			//
			if (deletionHappend)
				break;
		}
	}
	while (deletionHappend);
}


NuoStackPtrImpl NuoObjectImpl::StackPointerImpl()
{
	NuoStackPtrImpl stackPtr;

	if (_block)
	{
		stackPtr._block = _block->shared_from_this();
	}
	else
	{
		stackPtr._block.reset(new NuoStackControlBlock(this));
		_block = stackPtr._block.get();
	}

	return stackPtr;
}


bool NuoObjectImpl::PushProxy()
{
	return PushProxy(this);
}


bool NuoObjectImpl::PushProxy(NuoObjectImpl* exclude)
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
			if (o == exclude)
				continue;

			if (o->PushProxy(exclude))
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


void NuoObjectImpl::AddMember(NuoMemberPtrImpl* member)
{
	_members.insert(member);
}
