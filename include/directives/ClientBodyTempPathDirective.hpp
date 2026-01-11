#ifndef CLIENTBODYTEMPLATHDIRECTIVE_HPP
#define CLIENTBODYTEMPLATHDIRECTIVE_HPP

#include "IDirective.hpp"
#include <string>

namespace Directive
{
	class ClientBodyTempPathDirective : public IDirective
	{
	public:
		const std::string path;
		const int level1;  // 0 means not specified
		const int level2;  // 0 means not specified
		const int level3;  // 0 means not specified

	public:
		ClientBodyTempPathDirective();
		ClientBodyTempPathDirective(const std::string& path, int level1 = 0, int level2 = 0, int level3 = 0);
		ClientBodyTempPathDirective(const ClientBodyTempPathDirective& other);
		ClientBodyTempPathDirective& operator=(const ClientBodyTempPathDirective& other);
		~ClientBodyTempPathDirective();

		static ClientBodyTempPathDirective* initClientBodyTempPathDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
