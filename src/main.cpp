#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include "cl.h"
#include "ezcompiler.h"
#include "ezpath.h"

/*
* ezcompile commandline:
* ezcompile [ezcompile flags] <file> [args for build system] [-passthrough args for compiler...]
*/

static int quit(const char* Fmt, ...);

int main(int argc, const char** argv)
{	
	CL_Args args;
	int file = 0;
	const char* ext;
	bool isCpp;
	bool makeScript;
	CL_Tool* tool = nullptr;

	setvbuf(stdout, NULL, _IONBF, 0); // Compiling with Clang causes output to not flush :/

	// TODO: implement -passthrough lol

	// TODO: Handlers should use wildcards, as certain systems' files rely on naming and not extensions.
	// (ex: 'makefile') ((Not that I ever intend to emulate makefile))

	// Iterate past ezcompile flags to find input file
	if (argc > 1)
	{
		for (int i = 1; i < argc; ++i)
		{
			if (argv[i][0] != '-')
			{
				file = i;
				break;
			}
		}
	}

	if (!file)
		return quit("ezcompile was not given a file\n");

	ext = strrchr(argv[file], '.');
	if (!ext)
		return quit("ezcompile expected a file with an extension\n");

	 // Find a tool that matches the file type
	++ext;
	for (CL_Tool* t : CL_Tool::AllTools())
	{
		for (const char* match : t->FileExts())
		{
			if (!strcmp(match, ext))
			{
				tool = t;
				break;
			}
		}
		if (tool)
			break;
	}

	if (!tool)
		return quit("ezcompile does not have a tool for \".%s\" file type", ext);

	args = CL_Args(file - 1, argv + 1);
	isCpp = args.FindFlag("-cpp");
	makeScript = args.FindFlag("-script");

	EzClang clang = EzClang(nullptr, true);

	// Run the tool with args proceeding 'file'
	if (int err = tool->Run(argv[file], CL_Args(argc - (file + 1), argv + file + 1), &clang))
		return quit("ezcompile tool \"%s\" exited with error %d\n", tool->Name(), err);

	if (makeScript)
	{
		// TODO: Output cmd to appropriate <compiler>.sh file when more compilers are added
		std::ofstream f = std::ofstream("clang.sh");
		if (!f)
			return quit("ezcompile failed to write script file");

		std::string cmd = (clang.GenerateCmd() + " \"" + clang.CxxFileName() + '"');

		f.write(cmd.c_str(), cmd.size());
		f.close();

		printf("Wrote cmd to clang.sh:\n%s\n", cmd.c_str());
		return clang.GenerateCxxFile() ? 0 : -1;
	}
	else
	{
		std::stringstream cmd;
		cmd << clang.GenerateCmd() + ' ';

		EzPath out = clang.OutDir();
		for (auto it = clang.cxxFiles.begin(); it != clang.cxxFiles.end();)
		{
			cmd << '"' << out.Relative(*it).String() << '"';
			if (++it == clang.cxxFiles.end())
				break;
			cmd << ' ';
		}

		printf("Running cmd: %s\n", cmd.str().c_str());
		system(cmd.str().c_str());
	}

	return 0;
}

int quit(const char* Fmt, ...)
{
	va_list va;
	va_start(va, Fmt);

	vprintf(Fmt, va);

	va_end(va);
	return -1;
}
