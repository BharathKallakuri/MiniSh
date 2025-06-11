#ifndef TUI_HPP
#define TUI_HPP

#include <string>
// #include <ncurses.h>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/dom/text.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
class TUI {
public:
    TUI();
    ~TUI();

    void printPrompt(const std::string& prompt);
    std::string getInput();
    void printOutput(const std::string& output);
    void clear();

private:

};

#endif 