#include <filesystem>
#include <fstream>
#include <iostream>
#include <stack>
#include <unordered_set>
#include <utility>
#include <vector>

namespace bfi {

    // BrainFuck Interpreter
    // Run from the command line
    // arg1 - path to BF source code
    // arg2 - path to BF input file

    std::string tape_memory;
    int data_pointer=0;
    std::vector<std::pair<char, int>> cmds;


    std::pair<std::string, uintmax_t>
    get_basic_cmds(const char* src) {

        /*
        * Parse source code and return string
        * containing unoptimized code
        * Also return safe tape size for init
        */

        uintmax_t safe_size = std::max(
            std::filesystem::file_size(src)+5,
            (uintmax_t) 30005
        );

        std::ifstream f(src);
        std::unordered_set<char> chk{
            '>','<','+','-','.',',','[',']'
        };
        
        char c=0;
        std::stringstream s;

        while(f >> c)
            if (chk.count(c))
                s << c;

        return {s.str(), safe_size};
    }


    void env_init(const char* src) {

        auto tmp = get_basic_cmds(src);
        tape_memory.resize(tmp.second);
        cmds.reserve(tmp.second);

        // Open Brackets Stack
        std::stack<int> OBS;

        /*
        * Optimize BF Source Code
        * For eg: replace ++++++ with {+,6}
        * Store matching pairs of brackets
        */

        for(size_t i=0; i < tmp.first.size(); i++) {
            char c = tmp.first[i];

            if (c == '[') {
                cmds.push_back({'[', 0});
                int open = cmds.size() - 1;
                OBS.push(open);
            }
            else if (c==']') {
                cmds.push_back({']', 0});
                int open = OBS.top(); OBS.pop();
                int close = cmds.size() - 1;
                cmds[open].second = close;
                cmds[close].second = open;
            }
            else {
                if (cmds.size() && cmds.back().first==c) {
                    cmds.back().second++;
                } else {
                    cmds.push_back({c, 1});
                }
            }
        }

        if (! OBS.empty()) {
            std::cout<<"Brackets Unmatched!\n";
            cmds.clear();
        }
    }


    void parse_cmds(const char* input="") {

        if (cmds.empty()) {
            // Either mismatched brackets
            // Or no commands
            std::cout<<"Invalid Source File!\n\n";
            return;
        }

        // Reading the input for BF
        char c;
        int  a;
        std::ifstream bfin(input);

        for (int i=0; i<cmds.size(); i++) {
            
            // We need index i, don't use foreach
            c = cmds[i].first;
            a = cmds[i].second;

            switch(c) {
                case '>': {
                    data_pointer += a;
                    break;
                }
                case '<': {
                    data_pointer -= a;
                    break;
                }
                case '+': {
                    tape_memory[data_pointer]+=a;
                    break;
                }
                case '-': {
                    tape_memory[data_pointer]-=a;
                    break;
                }
                case '.': {
                    std::cout<<tape_memory[data_pointer];
                    break;
                }
                case ',': {
                    if (input=="") {
                        std::cout<<"Invalid/Missing Input File!\n\n";
                        return;
                    }
                    bfin>>tape_memory[data_pointer];
                    break;
                }
                default: {
                    bool cond1 = ((c=='[') && (tape_memory[data_pointer]==0));
                    bool cond2 = ((c==']') && (tape_memory[data_pointer]!=0));
                    if (cond1 || cond2) {
                        i = a;
                    }
                }
            }
        }
    }

}


int main(int argc, char const *argv[])    {
    
    if (argc == 3) {
        bfi::env_init(argv[1]);
        bfi::parse_cmds(argv[2]);
    }
    else if (argc == 2) {
        bfi::env_init(argv[1]);
        bfi::parse_cmds();
    }
    else {
        std::cout<<"Invalid Number of arguments!\n";
        std::cout<<"Usage: bfi source [input]\n";
        std::cout<<std::endl;
    }
    return 0;
}
