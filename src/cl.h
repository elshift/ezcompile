#pragma once
#include <vector>
#include <assert.h>

/*
* Commandline interface
*/

class EzConfig;

class CL_Args
{
public:
	CL_Args() : m_argc(0), m_argv(nullptr) { }
	CL_Args(int Argc, const char** Argv) : m_argc(Argc), m_argv(Argv) { }

	// - Returns index after flag, or 0 on failure
	int FindFlag(const char* Flag, int FollowingArgs = 0) const;
	int Argc() const { return m_argc; }
	inline const char* operator[](int i) const {
		assert(i >= 0 && i < m_argc && "Commandline arg index out of bounds");
		return m_argv[i];
	}


private:
	int m_argc;
	const char** m_argv;
};

// Choose who manages the commandline base on input file type
class CL_Tool
{
public:
	virtual int Run(const char* File, const CL_Args& Args, EzConfig* out_Cfg) = 0;

	const char* Name() const { return m_name; }
	const std::vector<const char*>& FileExts() const { return m_ext; }

	static const std::vector<CL_Tool*>& AllTools() { return m_tools; }

protected:
	template <class ...T>
	CL_Tool(const char* Name, const char* FileExt, T ...Moar)
		: m_name(Name), m_ext({ FileExt, Moar... }) {
		m_tools.push_back(this);
	}

private:

	const char* const m_name;
	std::vector<const char*> m_ext;
	static std::vector<CL_Tool*> m_tools;
};
