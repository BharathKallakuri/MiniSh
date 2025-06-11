#ifndef EXECUTE_HPP
#define EXECUTE_HPP

#include <vector>
#include <string>

class PosixProcess {
public:
    PosixProcess();

    // Execute a command with arguments, using fork + execvp
    // Waits for completion and returns exit code
    int run_command(const std::vector<std::string>& args);

    // Optionally add piping and redirection support later
};

#endif // EXECUTE_HPP
