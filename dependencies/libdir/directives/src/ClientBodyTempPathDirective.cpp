#include "ClientBodyTempPathDirective.hpp"
#include "HttpDirective.hpp"
#include "ServerDirective.hpp"
#include "LocationDirective.hpp"
#include "DirectiveFactory.hpp"
#include "Exceptions.hpp"
#include <cstdlib>
#include <cerrno>
#include <sstream>

namespace Config
{
	// Register directive: client_body_temp_path allowed in HttpDirective, ServerDirective, LocationDirective
	DEFINE_DIRECTIVE(ClientBodyTempPathDirective, "client_body_temp_path", HttpDirective, ServerDirective, LocationDirective)

	ClientBodyTempPathDirective* ClientBodyTempPathDirective::initClientBodyTempPathDirective(const std::vector<std::string>& args)
	{
		// client_body_temp_path requires at least 1 argument (path)
		// and optionally up to 3 more (level1, level2, level3)
		// Syntax: client_body_temp_path path [level1 [level2 [level3]]]
		if (args.size() < 2 || args.size() > 5)
		{
			throw ArgumentException("client_body_temp_path", "takes 1 to 4 arguments (path [level1 [level2 [level3]]])");
		}

		std::string path = args[1];
		int level1 = 0;
		int level2 = 0;
		int level3 = 0;

		// Parse optional level arguments
		if (args.size() >= 3)
		{
			char* endptr;
			errno = 0;
			long val = std::strtol(args[2].c_str(), &endptr, 10);
			if (errno != 0 || *endptr != '\0' || val < 0 || val > 10)
			{
				throw ArgumentException("client_body_temp_path", "level1 must be a number between 0 and 10");
			}
			level1 = static_cast<int>(val);
		}

		if (args.size() >= 4)
		{
			char* endptr;
			errno = 0;
			long val = std::strtol(args[3].c_str(), &endptr, 10);
			if (errno != 0 || *endptr != '\0' || val < 0 || val > 10)
			{
				throw ArgumentException("client_body_temp_path", "level2 must be a number between 0 and 10");
			}
			level2 = static_cast<int>(val);
		}

		if (args.size() >= 5)
		{
			char* endptr;
			errno = 0;
			long val = std::strtol(args[4].c_str(), &endptr, 10);
			if (errno != 0 || *endptr != '\0' || val < 0 || val > 10)
			{
				throw ArgumentException("client_body_temp_path", "level3 must be a number between 0 and 10");
			}
			level3 = static_cast<int>(val);
		}

		return new ClientBodyTempPathDirective(path, level1, level2, level3);
	}

	ClientBodyTempPathDirective::ClientBodyTempPathDirective()
		: IDirective(), path("client_body_temp"), level1(0), level2(0), level3(0)
	{
	}

	ClientBodyTempPathDirective::ClientBodyTempPathDirective(const std::string& path, int level1, int level2, int level3)
		: IDirective(), path(path), level1(level1), level2(level2), level3(level3)
	{
	}

	ClientBodyTempPathDirective::ClientBodyTempPathDirective(const ClientBodyTempPathDirective& other)
		: IDirective(other), path(other.path), level1(other.level1), level2(other.level2), level3(other.level3)
	{
	}

	ClientBodyTempPathDirective& ClientBodyTempPathDirective::operator=(const ClientBodyTempPathDirective& other)
	{
		if (this != &other)
		{
			IDirective::operator=(other);
			// const members cannot be reassigned
		}
		return *this;
	}

	ClientBodyTempPathDirective::~ClientBodyTempPathDirective()
	{
	}
}
