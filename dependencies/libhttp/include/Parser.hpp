#pragma once

#include <string>
#include <fstream>
#include "Request.hpp"
#include "RequestBody.hpp"

namespace libhttp {
	
	enum State {
		REQUEST_LINE,
		HEADERS,
		BODY,
		COMPLETE,

		ERROR,
		CLOSED,
		BODY_EXCEEDED_MAX_SIZE,
		ERROR_URI_TOO_LONG,
		ERROR_HTTP_VERSION_NOT_SUPPORTED,
		ERROR_UNSUPPORTED_TRANSFER_ENCODING,
		ERROR_HEADER_TOO_LARGE,
		ERROR_EMPTY_URI,
		ERROR_INVALID_HEADER,
		ERROR_LENGTH_REQUIRED
	};

	class Parser {
	public:
		Parser(size_t maxBodySize, const std::string& tempPath);
		~Parser();
		
		// simple state checkers
		bool isComplete() const;
		bool isError() const;

		// parsing function
		State parse(const char* data, size_t size);

		// get the parsed request
		const Request &getRequest() const;
		const RequestBody &getBody() const;

	private:
		State _state;
		std::string _buffer;
		size_t _expectedBodyLength; // from Content-Length header
        size_t _receivedBodyLength;
		bool _hasContentLength; // track if Content-Length header was set
		bool _hasTransferEncoding; // track if Transfer-Encoding header was set
		
        Request _request;
        
		int processRequestLine();
        int processHeaders();
        int processBody();

		// Config for body size limit
		size_t _maxBodySize;
		RequestBody _body;
	};

}

std::ostream& operator<<(std::ostream& os, libhttp::State s);
