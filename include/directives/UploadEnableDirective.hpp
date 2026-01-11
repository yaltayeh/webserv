#ifndef UPLOADENABLEDIRECTIVE_HPP
#define UPLOADENABLEDIRECTIVE_HPP

#include "IDirective.hpp"

namespace Directive
{
	class UploadEnableDirective : public IDirective
	{
	public:
		const bool enabled;

	public:
		UploadEnableDirective();
		UploadEnableDirective(bool enabled);
		UploadEnableDirective(const UploadEnableDirective& other);
		UploadEnableDirective& operator=(const UploadEnableDirective& other);
		~UploadEnableDirective();

		static UploadEnableDirective* initUploadEnableDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
