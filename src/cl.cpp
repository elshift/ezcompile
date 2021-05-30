#include "cl.h"
#include <string.h>

std::vector<CL_Tool*> CL_Tool::m_tools;

int CL_Args::FindFlag(const char* Flag, int Following) const
{
	for (int i = 0; i < m_argc - Following; ++i)
	{
		if (!strcmp(Flag, m_argv[i]))
		{
			int c = 0;
			for (; c < Following; c++)
			{
				if (m_argv[i + c + 1][0] == '-')
					break; // Beginning of new flag
			}
			if (c < Following)
				continue;

			return i + 1;
		}
	}
	return 0;
}
