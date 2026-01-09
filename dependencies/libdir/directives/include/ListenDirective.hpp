#ifndef LISTENDIRECTIVE_HPP
#define LISTENDIRECTIVE_HPP

#include "IDirective.hpp"
#include <string>

namespace Config
{
	class ListenDirective : public IDirective
	{
	public:
		const int port;
		const std::string host;

	public:
		ListenDirective();
		ListenDirective(int port, const std::string& host);
		ListenDirective(const ListenDirective& other);
		ListenDirective& operator=(const ListenDirective& other);
		~ListenDirective();

		static ListenDirective* initListenDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
