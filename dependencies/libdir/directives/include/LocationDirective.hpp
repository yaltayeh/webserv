#ifndef LOCATIONDIRECTIVE_HPP
#define LOCATIONDIRECTIVE_HPP

#include "IDirective.hpp"
#include <string>

namespace Config
{
	class LocationDirective : public IDirective
	{
	public:
		const std::string modifier;  // "=" (exact match), "@" (named location), or empty (prefix match)
		const std::string uri;

	public:
		LocationDirective();
		LocationDirective(const std::string& modifier, const std::string& uri);
		LocationDirective(const LocationDirective& other);
		LocationDirective& operator=(const LocationDirective& other);
		~LocationDirective();

		static LocationDirective* initLocationDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
