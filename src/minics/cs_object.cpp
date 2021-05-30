#include "cs_object.h"
#include "cs_string.h"

const CS_Member* CS_Object::GetMember(const std::string& Name) {
	return GetType()->GetMember(Name);
}
