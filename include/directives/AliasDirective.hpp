#ifndef ALIASDIRECTIVE_HPP
#define ALIASDIRECTIVE_HPP

#include "IDirective.hpp"
#include <string>

namespace Config
{
	class AliasDirective : public IDirective
	{
	public:
		const std::string path;

	public:
		AliasDirective();
		AliasDirective(const std::string& path);
		AliasDirective(const AliasDirective& other);
		AliasDirective& operator=(const AliasDirective& other);
		~AliasDirective();

		static AliasDirective* initAliasDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
