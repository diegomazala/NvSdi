#ifndef __EVENT_HANDLER_H__
#define __EVENT_HANDLER_H__



//////////////////////////////////////////////////////////////////////////
#include <typeinfo.h>
class TypeInfo
{
public:
	explicit TypeInfo(const type_info& info) : mTypeInfo(info) {};
	bool operator < (const TypeInfo& rhs) const
	{
		return mTypeInfo.before(rhs.mTypeInfo) != 0;
	}
private:
	const type_info& mTypeInfo;
};
//////////////////////////////////////////////////////////////////////////


#include <map>
#include "WinCore.h"
class IEvent;

class HandlerFunctionBase
{
public:
	virtual ~HandlerFunctionBase() {};
	void exec(const IEvent* event) {call(event);}

private:
	virtual void call(const IEvent*) = 0;
};


template <class T, class EventT>
class MemberFunctionHandler : public HandlerFunctionBase
{
public:
	typedef void (T::*MemberFunc)(EventT*);
	MemberFunctionHandler(T* instance, MemberFunc memFn) : _instance(instance), _function(memFn) {};

	void call(const IEvent* event)
	{
		(_instance->*_function)(static_cast<EventT*>(event));
	}

private:
	T* _instance;
	MemberFunc _function;
};


class WINAPP_API EventHandler
{
protected:
	~EventHandler();
	void handleEvent(const IEvent*);

	template <class T, class EventT>
	void registerEventFunc(T*, void (T::*memFn)(EventT*));

private:
	typedef std::map<TypeInfo, HandlerFunctionBase*> Handlers;
	Handlers _handlers;
};


template <class T, class EventT>
void EventHandler::registerEventFunc(T* obj, void (T::*memFn)(EventT*))
{
	_handlers[TypeInfo(typeid(EventT))]= new MemberFunctionHandler<T, EventT>(obj, memFn);
}

#endif	// __EVENT_HANDLER_H__