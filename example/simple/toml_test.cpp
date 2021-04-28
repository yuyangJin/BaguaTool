#include <toml.hpp> // that's all! now you can use it.
#include <iostream>
#include <string>

int main()
{
    const auto data  = toml::parse("example.toml");
    const auto title = toml::find<std::string>(data, "title");
    std::cout << "the title is " << title << std::endl;
    return 0;
}