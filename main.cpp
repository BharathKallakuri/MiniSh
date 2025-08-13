#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <unistd.h>
#include <sys/wait.h>
#include <thread>
#include <vector>
#include <string>
#include <atomic>
#include <mutex>
#include <sstream>
#include <signal.h>
#include <algorithm>
#include <iostream>
#include "backend.hpp"

using namespace ftxui;

enum ScreenIndex
{
    MAIN_SCREEN = 0,
    EXEC_SCREEN = 1,
    FUNC_SCREEN = 2
};
class ScrollableTextArea : public ComponentBase
{
public:
    ScrollableTextArea(std::vector<std::string> &lines, int height = 10)
        : lines_(lines), height_(height) {}

private:
    Element OnRender()
    {
        Elements visible_lines;
        int max_visible = std::min<int>(lines_.size() - top_, height_);
        for (int i = 0; i < max_visible; ++i)
        {
            visible_lines.push_back(text(lines_[top_ + i]));
        }
        return vbox(std::move(visible_lines)) | border;
    }

    bool OnEvent(Event event)
    {
        if (event == Event::ArrowUp)
        {
            if (top_ > 0)
                --top_;
            return true;
        }
        if (event == Event::ArrowDown)
        {
            if (top_ + height_ < int(lines_.size()))
                ++top_;
            return true;
        }
        if (event.is_mouse())
        {
            if (event.mouse().button == Mouse::WheelUp)
            {
                if (top_ > 0)
                    --top_;
                return true;
            }
            if (event.mouse().button == Mouse::WheelDown)
            {
                if (top_ + height_ < int(lines_.size()))
                    ++top_;
                return true;
            }
        }

        return false;
    }

    bool Focusable() { return true; }

    int top_ = 0;
    int height_;
    std::vector<std::string> &lines_;
};

Component MakeScrollableTextArea(std::vector<std::string> &lines, int height = 10)
{
    return Make<ScrollableTextArea>(lines, height);
}

struct ProcessIO
{
    int in_fd = -1;
    int out_fd = -1;
    pid_t pid = -1;
    std::atomic<bool> running{false};
    std::mutex output_mutex;
    std::vector<std::string> output_lines;
};

void start_process(const std::string &cmd, ProcessIO &pio)
{
    int in_pipe[2], out_pipe[2];
    pipe(in_pipe);
    pipe(out_pipe);

    pid_t pid = fork();
    if (pid == 0)
    {
        dup2(in_pipe[0], STDIN_FILENO);
        dup2(out_pipe[1], STDOUT_FILENO);
        dup2(out_pipe[1], STDERR_FILENO);
        close(in_pipe[1]);
        close(out_pipe[0]);
        execl("/bin/sh", "sh", "-c", cmd.c_str(), (char *)NULL);
        _exit(1);
    }
    else
    {
        pio.pid = pid;
        pio.in_fd = in_pipe[1];
        pio.out_fd = out_pipe[0];
        close(in_pipe[0]);
        close(out_pipe[1]);
        pio.running = true;

        std::thread([&pio]()
                    {
            char buffer[256];
            while (pio.running) {
                ssize_t n = read(pio.out_fd, buffer, sizeof(buffer) - 1);
                if (n > 0) {
                    buffer[n] = '\0';
                    std::lock_guard<std::mutex> lock(pio.output_mutex);
                    std::stringstream ss(buffer);
                    std::string line;
                    while (std::getline(ss, line))
                        pio.output_lines.push_back(line);
                } else {
                    break;
                }
            }
            pio.running = false;
            close(pio.out_fd);
            close(pio.in_fd);
            pio.in_fd = -1;
            pio.out_fd = -1; })
            .detach();
    }
}

auto f = backend;
int main()
{
    signal(SIGPIPE, SIG_IGN);
    auto screen = ScreenInteractive::Fullscreen();
    int tab_index = MAIN_SCREEN;

    ProcessIO pio;
    std::string command;
    auto command_input = Input(&command, "Enter command");
    auto run_button = Button("Run", [&]
                             {
        if (!command.empty()) {
            std::lock_guard<std::mutex> lock(pio.output_mutex);
            pio.output_lines.clear();
            start_process(command, pio);
            tab_index = EXEC_SCREEN;
        } });
    auto goto_func_screen = Button("Go to custom shell Screen", [&]
                                   { tab_index = FUNC_SCREEN; });
    auto main_container = Container::Vertical({command_input, run_button, goto_func_screen});
    auto main_renderer = Renderer(main_container, [&]
                                  { return vbox({text("Shell Launcher") | bold,
                                                 command_input->Render(),
                                                 run_button->Render(),
                                                 goto_func_screen->Render()}) |
                                           border; });
    std::string exec_input_str;
    auto exec_input = Input(&exec_input_str, "Type input for process");
    auto exec_send_button = Button("Send", [&]
                                   {
        if (!exec_input_str.empty() && pio.running && pio.in_fd != -1) {
            std::string to_send = exec_input_str + "\n";
            write(pio.in_fd, to_send.c_str(), to_send.size());
            exec_input_str.clear();
        } });
    auto back_button_exec = Button("Back", [&]
                                   {
        if (pio.running) {
            pio.running = false;
            kill(pio.pid, SIGTERM);
            waitpid(pio.pid, nullptr, 0);
        }
        tab_index = MAIN_SCREEN; });

    auto exec_area = MakeScrollableTextArea(pio.output_lines, 20);
    auto exec_container = Container::Vertical({exec_area, exec_input, exec_send_button, back_button_exec});
    auto exec_renderer = Renderer(exec_container, [&]
                                  { return vbox({text("Process Output") | bold,
                                                 exec_area->Render(),
                                                 exec_input->Render(),
                                                 exec_send_button->Render(),
                                                 back_button_exec->Render()}) |
                                           border; });

    std::string func_input_str;
    std::vector<std::string> func_output_lines;
    auto func_input = Input(&func_input_str, "Enter function input");
    auto func_run_button = Button("Run Function", [&]
                                  { backend(func_input_str, func_output_lines); });
    auto back_button_func = Button("Back", [&]
                                   { tab_index = MAIN_SCREEN; });

    auto func_area = MakeScrollableTextArea(func_output_lines, 30);
    auto func_container = Container::Vertical({func_area, func_input, func_run_button, back_button_func});
    auto func_renderer = Renderer(func_container, [&]
                                  { return vbox({text("Function Output") | bold,
                                                 func_area->Render(),
                                                 func_input->Render(),
                                                 func_run_button->Render(),
                                                 back_button_func->Render()}) |
                                           border; });
    auto tab_container = Container::Tab(
        Components{main_renderer, exec_renderer, func_renderer},
        &tab_index);
    screen.Loop(tab_container);
}
