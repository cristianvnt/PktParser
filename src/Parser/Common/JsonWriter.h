#ifndef JSON_WRITER_H
#define JSON_WRITER_H

#include "Misc/Define.h"

#include <string>
#include <fmt/format.h>

namespace PktParser::Common
{
    class JsonWriter
    {
    private:
        std::string _buffer;
        bool _needsComma;
    
    public:
        explicit JsonWriter(size_t reserveBytes = 1024) : _needsComma{ false }
        {
            _buffer.reserve(reserveBytes);
        }

        void BeginObject()
        {
            Comma();
            _buffer += '{';
            _needsComma = false;
        }

        void EndObject()
        {
            _buffer += '}';
            _needsComma = true;
        }

        void BeginArray()
        {
            Comma();
            _buffer += '[';
            _needsComma = false;
        }

        void EndArray()
        {
            _buffer += ']';
            _needsComma = true;
        }

        void Key(char const* key)
        {
            Comma();
            _buffer += '"';
            _buffer += key;
            _buffer += "\":";
            _needsComma = false;
        }

        void String(std::string const& val)
        {
            Comma();
            _buffer += '"';
            EscapeString(val);
            _buffer += '"';
            _needsComma = true;
        }

        void String(char const* val)
        {
            Comma();
            _buffer += '"';
            EscapeString(val);
            _buffer += '"';
            _needsComma = true;
        }

        void Int(int64 val)
        {
            Comma();
            fmt::format_to(std::back_inserter(_buffer), "{}", val);
            _needsComma = true;
        }

        void UInt(uint64 val)
        {
            Comma();
            fmt::format_to(std::back_inserter(_buffer), "{}", val);
            _needsComma = true;
        }

        void Double(double val)
        {
            Comma();
            fmt::format_to(std::back_inserter(_buffer), "{}", val);
            _needsComma = true;
        }

        void Bool(bool val)
        {
            Comma();
            _buffer += val ? "true" : "false";
            _needsComma = true;
        }

        void Null()
        {
            Comma();
            _buffer += "null";
            _needsComma = true;
        }

        void WriteString(char const* key, std::string const& val)
        {
            Key(key);
            String(val);
        }

        void WriteString(char const* key, char const* val)
        {
            Key(key);
            String(val);
        }

        void WriteInt(char const* key, int64 val)
        {
            Key(key);
            Int(val);
        }

        void WriteUInt(char const* key, uint64 val)
        {
            Key(key);
            UInt(val);
        }

        void WriteDouble(char const* key, double val)
        {
            Key(key);
            Double(val);
        }

        void WriteBool(char const* key, bool val)
        {
            Key(key);
            Bool(val);
        }

        std::string TakeString() { return std::move(_buffer); }
        std::string const& GetString() const { return _buffer; }
        char const* Data() const { return _buffer.data(); }
        size_t Size() const { return _buffer.size(); }
    
    private:
        void Comma()
        {
            if (_needsComma)
                _buffer += ',';
        }

        void EscapeString(std::string const& s)
        {
            for (char c : s)
            {
                switch (c)
                {
                case '"':
                    _buffer += "\\\"";
                    break;
                case '\\':
                    _buffer += "\\\\";
                    break;
                case '\n':
                    _buffer += "\\n";
                    break;
                case '\r':
                    _buffer += "\\r";
                    break;
                case '\t':
                    _buffer += "\\t";
                    break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20)
                        fmt::format_to(std::back_inserter(_buffer), "\\u{:04x}", static_cast<unsigned char>(c));
                    else
                        _buffer += c;
                    break;
                }
            }
        }

        void EscapeString(char const* s)
        {
            while (*s)
            {
                char c = *s++;
                switch (c)
                {
                case '"':
                    _buffer += "\\\"";
                    break;
                case '\\':
                    _buffer += "\\\\";
                    break;
                case '\n':
                    _buffer += "\\n";
                    break;
                case '\r':
                    _buffer += "\\r";
                    break;
                case '\t':
                    _buffer += "\\t";
                    break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20)
                        fmt::format_to(std::back_inserter(_buffer), "\\u{:04x}", static_cast<unsigned char>(c));
                    else
                        _buffer += c;
                    break;
                }
            }
        }
    };
}

#endif