#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>

class CommandParser
{
public:
    CommandParser();

    // Parse a command line string into command and arguments
    // Returns false if parsing failed
    bool parse(const std::string &input, std::vector<std::string> &args);

private:
};

#endif
