#ifndef CLIENTMAXBODYSIZEDIRECTIVE_HPP
#define CLIENTMAXBODYSIZEDIRECTIVE_HPP

#include "IDirective.hpp"
#include <string>

namespace Config
{
	class ClientMaxBodySizeDirective : public IDirective
	{
	public:
		const std::string size;

	public:
		ClientMaxBodySizeDirective();
		ClientMaxBodySizeDirective(const std::string& size);
		ClientMaxBodySizeDirective(const ClientMaxBodySizeDirective& other);
		ClientMaxBodySizeDirective& operator=(const ClientMaxBodySizeDirective& other);
		~ClientMaxBodySizeDirective();

		static ClientMaxBodySizeDirective* initClientMaxBodySizeDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
