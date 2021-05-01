#include <iostream>
#include <string>
#include <toml.hpp>  // that's all! now you can use it.

int main() {
  const auto data = toml::parse("example.toml");
  const auto title = toml::find<std::string>(data, "title");
  std::cout << "the title is " << title << std::endl;
  return 0;
}