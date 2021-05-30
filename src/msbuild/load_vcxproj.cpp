#include "load_vcxproj.h"

#include <pugixml.cpp> // Imma just slip this in here

const char* Load_ItemGroup(VCXProj& Proj, const pugi::xml_node& XMLGroup);
const char* Load_ItemDefGroup(VCXProj& Proj, const pugi::xml_node& XMLGroup);
const char* Load_ProjConfig(VCXProj& Proj, const pugi::xml_node& XMLConfig);

const char* Load_VCXProj(VCXProj& Proj, const pugi::xml_document& Doc)
{
	pugi::xml_node x_proj = Doc.child("Project");
	const char* err = 0;
	
	for (auto& child : x_proj)
	{
		if (!strcmp(child.name(), "ItemGroup"))
			err = Load_ItemGroup(Proj, child);
		else if (!strcmp(child.name(), "ItemDefinitionGroup"))
			err = Load_ItemDefGroup(Proj, child);

		if (err)
			return err;
	}

    return nullptr;
}

const char* Load_ItemGroup(VCXProj& Proj, const pugi::xml_node& XMLGroup)
{
	for (auto& child : XMLGroup)
	{
		if (!strcmp(child.name(), "ClCompile"))
			Proj.Compile().push_back(child.attribute("Include").value());
		else if (!strcmp(child.name(), "ClInclude"))
			Proj.Include().push_back(child.attribute("Include").value());
		else if (!strcmp(child.name(), "ProjectConfiguration"))
		{
			if (const char* err = Load_ProjConfig(Proj, child))
				return err;
		}
	}
	return nullptr;
}

const char* Load_ItemDefGroup(VCXProj& Proj, const pugi::xml_node& XMLGroup)
{
	Proj.ItemDefGroups().emplace_back();
	VCXItemDefGroup& group = Proj.ItemDefGroups().back();

	group.cond = XMLGroup.attribute("Condition").as_string();
	for (auto& x_defs : XMLGroup)
	{
		VCXItemDefs& defs = group[x_defs.name()];
		for (auto& x_item : x_defs)
			defs[x_item.name()] = x_item.text().as_string();
	}

	return nullptr;
}

const char* Load_ProjConfig(VCXProj& Proj, const pugi::xml_node& XMLConfig)
{
	const pugi::char_t* cfg, * plat;
	cfg = XMLConfig.child("Configuration").text().as_string();
	plat = XMLConfig.child("Platform").text().as_string();

	Proj.Configs().push_back({ cfg, plat });

	return nullptr;
}
