#pragma once
#include "cl.h"

class EzConfig;
class VCXProj;

class CL_VCX : public CL_Tool
{
public:
	CL_VCX() : CL_Tool("MSBuild", "vcxproj") { }

	int Run(const char* File, const CL_Args& Args, EzConfig* out_Cfg) override;
	bool ParseStd(VCXProj* Proj, EzConfig* out_Cfg);
};
