#ifndef ROOTDIRECTIVE_HPP
#define ROOTDIRECTIVE_HPP

#include "IDirective.hpp"
#include <string>

namespace Config
{
	class RootDirective : public IDirective
	{
	public:
		const std::string path;

	public:
		RootDirective();
		RootDirective(const std::string& path);
		RootDirective(const RootDirective& other);
		RootDirective& operator=(const RootDirective& other);
		~RootDirective();

		static RootDirective* initRootDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
