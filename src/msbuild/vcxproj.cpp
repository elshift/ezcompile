#include "vcxproj.h"
#include <assert.h>

void VCXProj::Configure_Hack(const char* CondString)
{
	for (VCXItemDefGroup& group : m_defgroups)
	{
		if (group.cond == CondString)
		{
			m_curCfg = &group;
			return;
		}
	}
}

size_t VCXProj::GetLines(const std::string& Str, std::list<std::string>* out_Lines)
{
	if (Str.empty())
		return 0;

	size_t count = 0;
	size_t start = Str.find_first_not_of(';'), end = Str.find(';', start);

	// For each line (divided by semicolons)
	while (start != std::string::npos)
	{
		++count;
		out_Lines->emplace_back(Str, start, end - start);

		start = end;
		if (end != std::string::npos)
		{
			start = Str.find_first_not_of(';', end);
			end = Str.find(';', start);
		}
	}

	return count;
}

const VCXItemDefGroup* VCXProj::SelectedCfg() const
{
	// Temporary hack. Picks first configuration.
	if (!m_curCfg && !m_defgroups.empty())
		return &m_defgroups.front();
	return m_curCfg;
}
