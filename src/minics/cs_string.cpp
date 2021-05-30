#include "cs_string.h"
#include "cs_state.h"

CS_RuntimeTypeInfo _cs_strinfo = CS_RuntimeTypeInfo::AddType<CS_String>();

CS_Ref<CS_Type> CS_String::_GetType()
{
	static CS_Type tp("System.String");
	tp._Bind("Substring", [](CS_Ref<CS_Object> o, CS_State& s) -> CS_Ref<CS_Object>
		{
			int off, len;
			s.GetArgs(&off, &len);
			return ((CS_String*)o.get())->Substring(off, len);
		});
	tp._Bind("Length", [](CS_Ref<CS_Object> o) -> CS_Ref<CS_Object> {
			return ((CS_String*)o.get())->Length();
		});

	static CS_Ref<CS_Type> ref(&tp);
	return ref;
}

CS_Ref<CS_Int32> CS_String::Length() {
	return CS_MakeObj<CS_Int32>(m_str.length());
}
