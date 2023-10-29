#include <iostream>
#include <string>

int main() {
    for (std::string line; std::getline(std::cin, line);) {
        std::cout << line << "foo" << std::endl;
    }
    return 0;
}

