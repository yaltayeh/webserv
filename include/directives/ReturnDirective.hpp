#ifndef RETURNDIRECTIVE_HPP
#define RETURNDIRECTIVE_HPP

#include "IDirective.hpp"
#include <string>

namespace Config
{
	class ReturnDirective : public IDirective
	{
	public:
		const int code;
		const std::string text;

	public:
		ReturnDirective();
		ReturnDirective(int code, const std::string& text);
		ReturnDirective(const ReturnDirective& other);
		ReturnDirective& operator=(const ReturnDirective& other);
		~ReturnDirective();

		static ReturnDirective* initReturnDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
