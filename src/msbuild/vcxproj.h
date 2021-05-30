#pragma once
#include <string>
#include <list>
#include <map>

typedef std::map<std::string, std::string> VCXItemDefs, VCXEnv;

struct ProjCfg {
	std::string cfg, plat;
};

// Has a condition and holds pages (ClCompile, Link, ...) of settings (e.g. { "WarningLevel" : "3", ... } )
class VCXItemDefGroup : public std::map<std::string, VCXItemDefs>
{
public:
	std::string cond;
};

class VCXProj
{
public:
	VCXProj() { }

	void Configure(const char* Plat, const char* Cfg);
	void Configure_Hack(const char* CondString);

	static size_t GetLines(const std::string& Str, std::list<std::string>* out_Lines);

	const VCXItemDefGroup* SelectedCfg() const;
	VCXItemDefGroup* SelectedCfg() {
		return (VCXItemDefGroup*)((const VCXProj*)this)->SelectedCfg();
	}

	std::list<ProjCfg>& Configs() { return m_cfgs; }
	auto& Compile() { return m_cmpl; }
	auto& Compile() const { return m_cmpl; }
	auto& Include() { return m_incl; }
	auto& Include() const { return m_incl; }
	std::list<VCXItemDefGroup>& ItemDefGroups() { return m_defgroups; }

private:
	std::list<ProjCfg> m_cfgs;
	std::list<std::string> m_cmpl;
	std::list<std::string> m_incl;
	std::list<VCXItemDefGroup> m_defgroups;

	VCXItemDefGroup* m_curCfg = 0;
};