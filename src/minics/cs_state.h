#pragma once
#include "cs_object.h"
#include "cs_string.h"
#include "cs_bool.h"
#include "cs_int.h"
#include <vector>
#include <stack>
#include <assert.h>

class CS_Object;

#ifdef _DEBUG
	#define CS_DBG_STATE(state, newHandleName) size_t newHandleName = state.Size()
	#define CS_DBG_STATE_END(state, handleName) \
		assert(state.Size() == handleName && "Uh oh! Looks like your stack... is out of wack!");
#else
	#define CS_DBG_STATE(state, newHandleName)
	#define CS_DBG_STATE_END(state, newHandleName)
#endif

class CS_State
{
public:

	bool GetArg(size_t Pos, CS_Ref<CS_Object>* out_Obj);
	bool GetArg(size_t Pos, bool* out_Num) { return GetArg_Num<CS_Bool>(Pos, out_Num); }
	bool GetArg(size_t Pos, int* out_Num) { return GetArg_Num<CS_Int32>(Pos, out_Num); }
	bool GetArg(size_t Pos, double* out_Num) { return GetArg_Num<CS_Double>(Pos, out_Num); }
	bool GetArg(size_t Pos, std::string* out_Str);

	bool GetArgs() { return true; }

	template <class T, class ...TArgs>
	bool GetArgs(T First, TArgs ...Moar)
	{
		size_t n = _GetArgs(First, Moar...);
		if (n == (size_t)-1)
			return false;

		Pop(n);
		return true;
	}

	template <class T, class ...TArgs>
	bool GetArgs_Strict(T First, TArgs ...Moar)
	{
		size_t n = _GetArgs(First, Moar...);
		if (n == (size_t)-1)
		{
			CS_MISMATCH();
			return false;
		}

		Pop(n);
		return true;
	}

	template <class ...TMany>
	void Push(bool Bool, const TMany& ...Many) { Push_Prim<CS_Bool>(Bool, Many...); }
	template <class ...TMany>
	void Push(int32_t Int32, const TMany& ...Many) { Push_Prim<CS_Int32>(Int32, Many...); }
	template <class ...TMany>
	void Push(const std::string& Str, const TMany& ...Many) { Push_Prim<CS_String>(Str, Many...); }
	template <class ...TMany>
	void Push(const CS_Ref<CS_Object>& o, const TMany& ...Many)
	{
		m_stack.push_back(o);
		Push(Many...);
	}

	CS_Ref<CS_Object> Top() { return m_stack.back(); }
	void Pop(size_t N = 1)
	{
		assert(N <= m_stack.size() && "Tried to pop more items than exists on stack");
		m_stack.resize(m_stack.size() - N);
	}

	size_t Size() const { return m_stack.size(); }

private:
	void Push() { }

	template <class TCSPrim, class TPrim, class ...TMany>
	void Push_Prim(const TPrim& prim, const TMany& ...Many)
	{
		m_stack.emplace_back(CS_MakeObj<TCSPrim>(prim));
		Push(Many...);
	}

	template <class TClass, class TVal>
	bool GetArg_Num(size_t Pos, TVal out_Val)
	{
		CS_Ref<CS_Object> obj;
		GetArg(Pos, &obj);
		if (!TClass::_GetType()->Equals(obj))
			return false;

		*out_Val = ((TClass*)obj.get())->_Value();
		return true;
	}

	size_t _GetArgs() { return 0; }

	template <class ...TArgs>
	size_t _GetArgs(std::string* Next, TArgs ...Args)
	{
		CS_Ref<CS_Object> o;
		size_t pos = _GetArgs(Args...);
		if (pos == (size_t)-1)
			return (size_t)-1;

		if (!GetArg(pos, Next))
		{
			if (!GetArg(pos, &o))
				return (size_t)-1;
			*Next = o->ToString();
		}
		return pos + 1;
	}

	template <class T, class ...TArgs>
	size_t _GetArgs(T Next, TArgs ...Args)
	{
		size_t pos = _GetArgs(Args...);
		if (pos == (size_t)-1 || !GetArg(pos, Next))
			return (size_t)-1;
		return pos + 1;
	}

	std::vector<CS_Ref<CS_Object>> m_stack;
};