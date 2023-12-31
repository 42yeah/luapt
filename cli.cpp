#include <iostream>
#include "image.h"
#include "luaenv.h"
#include "model.h"

int main(int argc, char **argv)
{
    bool has_file_name = argc > 1;
    std::string filename;
    std::string prev;
    std::shared_ptr<Lua> lua = Lua::inst();

    std::cout << "Type in the file you want to execute (or just enter to reuse the last one):" << std::endl;
    while (true)
    {
        std::cout << "> ";
        if (!has_file_name)
        {
            std::getline(std::cin, filename);
        }
        else
        {
            filename = argv[1];
        }
        if (std::cin.eof())
        {
            break;
        }
        if (filename == "")
        {
            filename = prev;
        }
        lua->execute_file(filename);
        prev = filename;

        if (has_file_name)
        {
            break;
        }
    }

    return 0;
}
