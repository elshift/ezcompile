#include "cs_type.h"
#include "cs_string.h"
#include "cs_bool.h"
#include "cs_state.h"

CS_TypeDict* CS_RuntimeTypeInfo::m_types = nullptr;
CS_RuntimeTypeInfo _cs_tpinfo = CS_RuntimeTypeInfo::AddType<CS_Type>();

CS_Type::CS_Type(const char* Name) : m_name(Name)
{
	_Bind("GetType", [](CS_Ref<CS_Object> o, CS_State&) -> CS_Ref<CS_Object> { return o->GetType(); });
	//Bind("GetHashCode", [](CS_Object* o) { return new CS_Int32(o->GetHashCode()); });
	_Bind("Equals", [](CS_Ref<CS_Object> o, CS_State& s) -> CS_Ref<CS_Object> {
		return CS_MakeObj<CS_Bool>(o->Equals(s.Top()));
		});
	_Bind("ToString", [](CS_Ref<CS_Object> o, CS_State&) -> CS_Ref<CS_Object> {
		return CS_MakeObj<CS_String>(o->ToString());
		});
}

const CS_Member* CS_Type::GetMember(const std::string& Name)
{
	const CS_Member* m;;
	auto it = m_members.find(Name);

	if (it == m_members.end())
		return nullptr;

	m = &(*it).second;
	if (!(m->flags & CSFlag_Public))
		return nullptr;
	return m;
}
