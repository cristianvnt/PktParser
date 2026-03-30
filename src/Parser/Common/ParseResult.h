#pragma once

#include "Misc/Define.h"
#include "ISearchFields.h"

#include <string>

namespace PktParser::Common
{
    struct ParseResult
    {
        std::string json;
        ISearchFields* searchFields = nullptr;

        ParseResult(std::string json, ISearchFields* fields) : json{ std::move(json) }, searchFields{ fields } {}

        ParseResult() = default;
        ~ParseResult() { delete searchFields; }
        ParseResult(ParseResult&& other) noexcept
        {
            json = std::move(other.json);
            searchFields = other.searchFields;
            other.searchFields = nullptr;
        }

        ParseResult& operator=(ParseResult&& other) noexcept
        {
            if (this != &other)
            {
                delete searchFields;
                json = std::move(other.json);
                searchFields = other.searchFields;
                other.searchFields = nullptr;
            }
            return *this;
        }

        ParseResult(ParseResult const&) = delete;
        ParseResult& operator=(ParseResult const&) = delete;
    };
}
