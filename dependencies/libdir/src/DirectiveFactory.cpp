/**
 * @file DirectiveFactory.cpp
 * @brief Implementation of DirectiveFactory class
 */

#include "DirectiveFactory.hpp"
#include "IDirective.hpp"
#include <iostream>

namespace Config
{

// Static member initialization
std::map<std::string, std::pair<DirectiveFactory::CreatorFunc, const std::type_info *> >
	DirectiveFactory::registry_name;

std::map<const std::type_info *, std::pair<DirectiveFactory::CreatorFunc, std::string> >
	DirectiveFactory::registry_id;

/**
 * @brief Registers a directive in the factory
 */
void DirectiveFactory::registerDirective(const std::string &name, 
                                         const std::type_info *id, 
                                         CreatorFunc creator)
{
	registry_name[name] = std::make_pair(creator, id);
	registry_id[id] = std::make_pair(creator, name);
}

/**
 * @brief Gets creator function by name
 */
DirectiveFactory::CreatorFunc DirectiveFactory::getCreator(const std::string &name)
{
	std::pair<CreatorFunc, const std::type_info *> data = registry_name.at(name);
	return data.first;
}

/**
 * @brief Gets creator function by type info
 */
DirectiveFactory::CreatorFunc DirectiveFactory::getCreator(const std::type_info *id)
{
	std::pair<CreatorFunc, std::string> data = registry_id.at(id);
	return data.first;
}

/**
 * @brief Gets directive name from type info
 */
std::string DirectiveFactory::getName(const std::type_info *id)
{
	std::pair<CreatorFunc, std::string> data = registry_id.at(id);
	return data.second;
}

/**
 * @brief Gets type info from directive name
 */
const std::type_info *DirectiveFactory::getTypeInfo(const std::string &name)
{
	std::pair<CreatorFunc, const std::type_info *> data = registry_name.at(name);
	return data.second;
}

} // namespace Config
