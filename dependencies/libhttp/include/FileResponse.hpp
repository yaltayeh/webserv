#pragma once

#include <string>
#include <map>
#include "Response.hpp"
#include "Constants.hpp"

namespace libhttp {

    class FileResponse: public Response {
    public:
        FileResponse(std::string filePath, const std::string &contentType, int statusCode = 200);
        ~FileResponse();

        // Override virtual methods from Response
        size_t genChunk(char *buffer, size_t max_size);
        bool hasChunk() const;
        bool isComplete() const;

        std::string toString() const;
    private:
        int         _fd;
        std::string _headerData;
        size_t      _offset;
        size_t      _readBytes;
        size_t      _fileSize;
        std::string _filePath;
    };

}

// Stream insertion operator
std::ostream &operator<<(std::ostream &os, const libhttp::FileResponse &response);