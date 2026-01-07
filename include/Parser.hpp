#include <string>

#pragma once

namespace Config
{
	class Parser
	{
	private:
		const std::string &_configString;
		const char *_start;
		const char *_end;
		const char *_p;
		const char *_directiveStart;
		const char *_directiveEnd;

	public:
		Parser(const std::string &configString);
		Parser(const std::string &configString, const char *start, const char *end);
		~Parser();
		bool getNextDirectives();
		std::string copyDirective() const;

		const char *getDirectiveStart() const;
		const char *getDirectiveEnd() const;

		static const char *skipSpace(const char *p, const char *end);
		static const char *skipComment(const char *p, const char *end);
		static const char *nextQout(const char *p, const char *end, char qout);
		static const char *getName(const char *p, const char *end, std::string &name);
	};

}
