#include <iostream>
#include "image.h"
#include "luaenv.h"
#include "model.h"

int main(int argc, char **argv)
{
    Model model;
    if (!model.load("suzanne.obj"))
    {
        std::cerr << model.get_load_errors() << std::endl << model.get_load_warnings() << std::endl;
    }

    std::string filename;
    std::string prev;
    std::shared_ptr<Lua> lua = Lua::inst();

    std::cout << "Type in the file you want to execute (or just enter to reuse the last one):" << std::endl;
    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, filename);
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
    }

    return 0;
}
