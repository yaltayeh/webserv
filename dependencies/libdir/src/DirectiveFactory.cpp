/**
 * @file DirectiveFactory.cpp
 * @brief Implementation of DirectiveFactory class
 */

#include "DirectiveFactory.hpp"
#include "IDirective.hpp"
#include <iostream>

namespace Config
{

/**
 * @brief Gets registry by name (initialized on first use)
 */
std::map<std::string, std::pair<DirectiveFactory::CreatorFunc, const std::type_info *> >&
DirectiveFactory::getRegistryName()
{
	static std::map<std::string, std::pair<CreatorFunc, const std::type_info *> > registry;
	return registry;
}

/**
 * @brief Gets registry by type info (initialized on first use)
 */
std::map<const std::type_info *, std::pair<DirectiveFactory::CreatorFunc, std::string> >&
DirectiveFactory::getRegistryId()
{
	static std::map<const std::type_info *, std::pair<CreatorFunc, std::string> > registry;
	return registry;
}

/**
 * @brief Registers a directive in the factory
 */
void DirectiveFactory::registerDirective(const std::string &name, 
                                         const std::type_info *id, 
                                         CreatorFunc creator)
{
	getRegistryName()[name] = std::make_pair(creator, id);
	getRegistryId()[id] = std::make_pair(creator, name);
}

/**
 * @brief Gets creator function by name
 */
DirectiveFactory::CreatorFunc DirectiveFactory::getCreator(const std::string &name)
{
	std::pair<CreatorFunc, const std::type_info *> data = getRegistryName().at(name);
	return data.first;
}

/**
 * @brief Gets creator function by type info
 */
DirectiveFactory::CreatorFunc DirectiveFactory::getCreator(const std::type_info *id)
{
	std::pair<CreatorFunc, std::string> data = getRegistryId().at(id);
	return data.first;
}

/**
 * @brief Gets directive name from type info
 */
std::string DirectiveFactory::getName(const std::type_info *id)
{
	std::pair<CreatorFunc, std::string> data = getRegistryId().at(id);
	return data.second;
}

/**
 * @brief Gets type info from directive name
 */
const std::type_info *DirectiveFactory::getTypeInfo(const std::string &name)
{
	std::pair<CreatorFunc, const std::type_info *> data = getRegistryName().at(name);
	return data.second;
}

} // namespace Config
