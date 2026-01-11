#include "IndexDirective.hpp"
#include "HttpDirective.hpp"
#include "ServerDirective.hpp"
#include "LocationDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"

namespace Directive
{
	// Register directive: index allowed in HttpDirective, ServerDirective, and LocationDirective
	DEFINE_DIRECTIVE(IndexDirective, "index", HttpDirective, ServerDirective, LocationDirective)

	IndexDirective* IndexDirective::initIndexDirective(const std::vector<std::string>& args)
	{
		// index requires at least one file
		if (args.size() < 2)
		{
			throw ArgumentException("index", "requires at least one filename");
		}

		// Collect all index files
		std::vector<std::string> files;
		for (size_t i = 1; i < args.size(); i++)
		{
			if (args[i].empty())
			{
				throw ArgumentException("index", "filename cannot be empty");
			}
			files.push_back(args[i]);
		}

		return new IndexDirective(files);
	}

	// Default constructor
	IndexDirective::IndexDirective()
		: IDirective(), files()
	{
	}

	// Parameterized constructor
	IndexDirective::IndexDirective(const std::vector<std::string>& files)
		: IDirective(), files(files)
	{
	}

	// Copy constructor
	IndexDirective::IndexDirective(const IndexDirective& other)
		: IDirective(other), files(other.files)
	{
	}

	// Assignment operator
	IndexDirective& IndexDirective::operator=(const IndexDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
			// files is const, cannot reassign
		}
		return *this;
	}

	// Destructor
	IndexDirective::~IndexDirective()
	{
	}
}
