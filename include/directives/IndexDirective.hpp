#ifndef INDEXDIRECTIVE_HPP
#define INDEXDIRECTIVE_HPP

#include "IDirective.hpp"
#include <vector>
#include <string>

namespace Directive
{
	class IndexDirective : public IDirective
	{
	public:
		const std::vector<std::string> files;

	public:
		IndexDirective();
		IndexDirective(const std::vector<std::string>& files);
		IndexDirective(const IndexDirective& other);
		IndexDirective& operator=(const IndexDirective& other);
		~IndexDirective();

		static IndexDirective* initIndexDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
