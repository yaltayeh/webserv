#ifndef UPLOADSTOREDIRECTIVE_HPP
#define UPLOADSTOREDIRECTIVE_HPP

#include "IDirective.hpp"
#include <string>

namespace Directive
{
	class UploadStoreDirective : public IDirective
	{
	public:
		const std::string path;

	public:
		UploadStoreDirective();
		UploadStoreDirective(const std::string& path);
		UploadStoreDirective(const UploadStoreDirective& other);
		UploadStoreDirective& operator=(const UploadStoreDirective& other);
		~UploadStoreDirective();

		static UploadStoreDirective* initUploadStoreDirective(const std::vector<std::string>& args);
		DEFINE_ALLOWED_INSIDE();
	};
}

#endif
