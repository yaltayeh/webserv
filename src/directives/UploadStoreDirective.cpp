#include "UploadStoreDirective.hpp"
#include "LocationDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"

namespace Directive
{
	// Register directive: upload_store allowed in LocationDirective
	DEFINE_DIRECTIVE(UploadStoreDirective, "upload_store", LocationDirective)

	UploadStoreDirective* UploadStoreDirective::initUploadStoreDirective(const std::vector<std::string>& args)
	{
		// upload_store requires exactly one argument (path)
		if (args.size() != 2)
		{
			throw ArgumentException("upload_store", "takes exactly one argument (path)");
		}

		std::string path = args[1];
		
		// Validate path is not empty
		if (path.empty())
		{
			throw ArgumentException("upload_store", "path cannot be empty");
		}

		return new UploadStoreDirective(path);
	}

	// Default constructor
	UploadStoreDirective::UploadStoreDirective()
		: IDirective(), path("")
	{
	}

	// Parameterized constructor
	UploadStoreDirective::UploadStoreDirective(const std::string& path)
		: IDirective(), path(path)
	{
	}

	// Copy constructor
	UploadStoreDirective::UploadStoreDirective(const UploadStoreDirective& other)
		: IDirective(other), path(other.path)
	{
	}

	// Assignment operator
	UploadStoreDirective& UploadStoreDirective::operator=(const UploadStoreDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
			// path is const, cannot reassign
		}
		return *this;
	}

	// Destructor
	UploadStoreDirective::~UploadStoreDirective()
	{
	}
}
