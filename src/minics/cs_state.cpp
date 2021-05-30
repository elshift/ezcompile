#include "cs_state.h"
#include "cs_type.h"
#include "cs_int.h"
#include "cs_string.h"

bool CS_State::GetArg(size_t Pos, CS_Ref<CS_Object>* out_Obj)
{
	assert(Pos < m_stack.size() && "Tried to access outside bounds of stack");
	*out_Obj = m_stack[m_stack.size() - 1 - Pos];
	return true;
}

bool CS_State::GetArg(size_t Pos, std::string* out_Str)
{
	CS_Ref<CS_Object> obj;
	GetArg(Pos, &obj);
	if (!CS_String::_GetType()->Equals(obj))
		return false;

	*out_Str = ((CS_String*)obj.get())->_Value();
	return true;
}
