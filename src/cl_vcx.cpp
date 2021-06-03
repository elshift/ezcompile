#include "cl_vcx.h"
#include "msbuild/msbuild.h"
#include "ezcompiler.h"

#include <stdio.h>

static CL_VCX vcx_handler;

int CL_VCX::Run(const char* File, const CL_Args& Args, EzConfig* out_Cfg)
{
	const char* solution = 0;
	char input[8];

	if (int sln = Args.FindFlag("-sln", 1))
		solution = Args[sln];

	MSBuild build(solution);
	if (!build.LoadProject(File))
		return -1;

	VCXProj* proj = build.Proj();

	puts("Select a configuration");
	int i = 0;
	for (VCXItemDefGroup& group : proj->ItemDefGroups())
		printf("\t%d) %s\n", i++, group.cond.c_str());

	printf("Config #: ");

	fgets(input, sizeof(input), stdin);
	i = atoi(input);
	if (i < 0 || (size_t)i >= proj->ItemDefGroups().size())
		i = 0;

	for (auto it = proj->ItemDefGroups().begin(); it != proj->ItemDefGroups().end(); ++it, --i)
	{
		if (i == 0)
		{
			proj->Configure_Hack(it->cond.c_str());
			break;
		}
	}

	if (!ParseStd(proj, out_Cfg))
		printf("[MSBuild] Failed to get C++ standard of project");

	build.GetIncludeDirs(&out_Cfg->inclDirs);

	for (const std::string& cxx : build.CxxFiles())
		out_Cfg->cxxFiles.push_back(build.Env()["ProjectDir"] + cxx);

	return 0;
}

bool CL_VCX::ParseStd(VCXProj* Proj, EzConfig* out_Cfg)
{
	if (out_Cfg->Lang().val == Lang_Cpp)
	{
		std::string std = (*Proj->SelectedCfg())["ClCompile"]["LanguageStandard"];
		ECppStd cpp = CppStd_Default;

		if (std.empty() || std == "stdcpp14")
			cpp = CppStd_14;
		else if (std == "stdcpp17")
			cpp = CppStd_17;
		else if (std == "stdcpplatest")
		{
			printf(
				"[MSBuild] Warning: \"stdcpplatest\" is relative to time. "
				"As of Saturday, May 29, 2021, the latest version in ezcompile is the C++20 draft (C++2a)");

			cpp = CppStd_2a;
		}
		else
			printf("[MSBuild] Warning: unknown LanguageStandard \"%s\". Using compiler default\n",
				std.c_str());

		out_Cfg->LangStd().val.cpp = cpp;
	}
	return true;
}
