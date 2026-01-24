#pragma once

#include <string>
#include <map>
#include <vector>
#include "Response.hpp"
#include "Constants.hpp"

namespace libhttp
{

    class DefaultResponse : public Response
    {
    private:
        std::string data;
        size_t sent_bytes;

    public:
        DefaultResponse(const std::string &data);
        ~DefaultResponse();

        size_t genChunk(char *buffer, size_t max_size);
        bool hasChunk() const;
        bool isComplete() const;
    };
};
