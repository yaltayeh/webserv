#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include "Request.hpp"

namespace libhttp {

	class Parser {
	public:
		Parser();
		~Parser();
		
		// simple state checkers
		bool isComplete() const;
		bool isError() const;

		// parsing function
		int parse(const char* data, size_t size);

		// get the parsed request
		Request getRequest() const;

	private:

		enum State {
            REQUEST_LINE,
            HEADERS,
            BODY,
            COMPLETE,
            ERROR
        };

		State _state;
		std::string _buffer;
        Request _request;

        int processRequestLine();
        int processHeaders();
        int processBody();
	};

}

#endif