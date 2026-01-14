#pragma once

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
		// returns: 0=complete, 1=incomplete, -1=error
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
		size_t _expectedBodyLength; // from Content-Length header
        size_t _receivedBodyLength;
		
        Request _request;
        
		int processRequestLine();
        int processHeaders();
        int processBody();
	};

}
