#pragma once
#include "vcxproj.h"
#include <pugixml.hpp>

const char* Load_VCXProj(VCXProj& Proj, const pugi::xml_document& Doc);