#pragma once

#include <string>
#include <fstream>
#include "Request.hpp"

namespace libhttp {

	class Parser {
	public:
		Parser();
		Parser(size_t maxBodySize, const std::string& tempPath);
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
		// Non-copyable due to ofstream member
		Parser(const Parser&);
		Parser& operator=(const Parser&);

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
