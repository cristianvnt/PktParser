#pragma once

#include "JsonWriter.h"

namespace PktParser::Common
{
    struct ISearchFields
    {
        virtual ~ISearchFields() = default;
        virtual void WriteTo(JsonWriter& doc) const = 0;
    };
}