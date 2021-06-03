#include "cs_object.h"
#include "cs_string.h"

const CS_Member* CS_Object::GetMember(const std::string& Name) {
	return GetType()->GetMember(Name);
}

void CS_Object::_Bind(const char* Name, CS_StaticMethod_t Val, int Flags) {
	GetType()->_Bind(Name, Val, Flags);
}
void CS_Object::_Bind(const char* Name, CS_BoundMethod_t Val, int Flags) {
	GetType()->_Bind(Name, Val, Flags);
}
void CS_Object::_Bind(const char* Name, CS_StaticAccess_t Val, int Flags) {
	GetType()->_Bind(Name, Val, Flags);
}
void CS_Object::_Bind(const char* Name, CS_BoundAccess_t Val, int Flags) {
	GetType()->_Bind(Name, Val, Flags);
}
