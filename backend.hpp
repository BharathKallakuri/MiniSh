#include "external/CLI11.hpp"
#include <bits/stdc++.h>
#include <filesystem>
#include <wordexp.h>

using namespace std;
namespace fs = std::filesystem;


void backend(string &input, vector<string> &tout)
{
    CLI::App app{"A custom minishell", "shell"};
    app.require_subcommand(1);
    fs::path cwd = fs::current_path();
    // ---------------------------------------------------------------------------------------------------------
    CLI::App *cd = app.add_subcommand("cd", "Change the current working directory");
    string cd_path = "~";
    cd->add_option("directory_path", cd_path, "Change directory to provided path, if not mentioned will change to home directory")->expected(0, 1);
    cd->callback([&]()
                 {
        if(fs::exists(cd_path) && fs::is_directory(cd_path)){
            cwd = fs::path(cd_path);
        }else{
            // tout<<"Give path is incorrect/ Path does not exist";
            tout.push_back("Give path is incorrect/ Path does not exist");
        } });
    // ----------------------------------------------------------------------------------------------------------
    CLI::App *ls = app.add_subcommand("ls", "List the files in the given directory");
    string ls_path = ".";
    ls->add_option("ls_path", ls_path, "List the files in give directory, default mentions the files in current working directory")->expected(0, 1);
    bool ls_lflag = false, ls_rflag = false;
    ls->add_flag("-l", ls_lflag, "List in a formatted way");
    ls->add_flag("-r", ls_rflag, "List files recursively");
    int ls_depth = 0;
    ls->add_option("-d,--depth", ls_depth, "Set depth limit,maximum 1000")->default_val(1000);
    function<void(string, int, int)> print_lsr = [&](string currpath, int depth, int lim)
    {
        if (depth == lim)
        {
            return;
        }
        auto dir_path = fs::directory_iterator(currpath);
        for (auto &entry : dir_path)
        {
            string disp ;
            for (int i = 0; i < depth; ++i)
            {
                disp += "    ";
            }
            disp += "├─";
            string ret;
            disp += entry.path().filename().string();
            tout.push_back(disp);
            if (entry.is_directory())
            {
                print_lsr(entry.path(), depth + 1, lim);
            }
        }
    };
    ls->callback([&]()
                 {
        if(fs::exists(ls_path)){
            if(fs::is_directory(ls_path)){
                auto dir_path = fs::directory_iterator(ls_path);
                int currd = ls_depth;
                if(ls_depth<1000 || ls_rflag){
                        print_lsr(ls_path,0,ls_depth);
                }else{
                        print_lsr(ls_path,0,1);
                
                }
                if(ls_depth<1000){
                }else if(ls_rflag){
                    print_lsr(ls_path,0,ls_depth); 
                }
            }else{
                    fs::directory_entry entry(ls_path);
                    tout.push_back( entry.path().filename().string());
                }
        }else{
            // tout<<"File name is incorrect/ File does not exist";
            tout.push_back("File name is incorrect/ File does not exist");
        } });
    // ----------------------------------------------------------------------------------------------------------
    CLI::App *echo = app.add_subcommand("echo", "Echoes");
    vector<string> echo_str;
    echo->add_option("input", echo_str, "Strings to be echoed");
    echo->callback([&]()
                   {
                    string ret = "";
            for(auto &entry: echo_str){
                
                ret += entry;
                ret += " ";
            }
            tout.push_back(ret); });
    // ----------------------------------------------------------------------------------------------------------
    CLI::App *pwd = app.add_subcommand("pwd", "Current Working Directory");
    pwd->callback([&]()
                  {
            fs::directory_entry entry(cwd);
            // tout<<entry.path().string(); 
                    tout.push_back(entry.path().string()); });
    // ----------------------------------------------------------------------------------------------------------
    CLI:: App *clear = app.add_subcommand("clear","Clears the screen");
    clear->callback([&tout](){
        tout.clear();
    });
    // ----------------------------------------------------------------------------------------------------------

    try
    {
        wordexp_t p;
        wordexp(input.data(), &p, 0);
        app.parse(p.we_wordc, p.we_wordv);
        wordfree(&p);
    }
    catch (CLI::RequiredError &e)
    {
        tout.push_back("Command not found");
    }
    catch (const CLI::ParseError &e)
    {
        tout.push_back("Parsing error");
        (app).exit(e);
        return;
    }
}
