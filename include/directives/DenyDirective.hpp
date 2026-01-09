#ifndef DENYDIRECTIVE_HPP
#define DENYDIRECTIVE_HPP

#include "IDirective.hpp"
#include <string>

namespace Config
{
	class DenyDirective : public IDirective
	{
	public:
		const std::string address;  // IP address, CIDR, or "all"

	public:
		DenyDirective();
		DenyDirective(const std::string& address);
		DenyDirective(const DenyDirective& other);
		DenyDirective& operator=(const DenyDirective& other);
		~DenyDirective();

		static DenyDirective* initDenyDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
