#pragma once

#include <string>
#include <fstream>
#include "Request.hpp"


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
		ERROR_UNSUPPORTED_TRANSFER_ENCODING
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
		std::string _tempPath;
		
		// Temp file handling
		bool _usesTempFile;
		std::string _tempFilePath;
		std::ofstream _tempFile;
		
		void createTempFile();
		void writeTempFile(const char* data, size_t size);
		void closeTempFile();
	};

}

std::ostream& operator<<(std::ostream& os, libhttp::State s);
