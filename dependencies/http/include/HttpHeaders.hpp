#include <map>
#include <string>

#pragma once

namespace Http
{
	class HttpHeaders
	{
	private:
		std::multimap<std::string, std::string> headers;

	public:
		HttpHeaders();
		~HttpHeaders();
		void add(const std::string &key, const std::string &value);
		bool get(const std::string &key, std::string &valeu) const;
		bool exist(const std::string &key) const;
		bool remove(const std::string &key);
		const std::multimap<std::string, std::string> &getAll() const;
	};
}
