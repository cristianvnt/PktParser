#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include <string>

class ParseException : public std::runtime_error
{
public:
	explicit ParseException(std::string const& message) : std::runtime_error{ message } { }
};

#endif // !EXCEPTIONS_H