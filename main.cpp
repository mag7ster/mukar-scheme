#include <iostream>
#include <string>
#include "src/error.h"
#include "src/scheme.h"

int main() {
    std::string s;
    Interpreter inter;
    while (std::getline(std::cin, s)) {
        try {
            std::cout << inter.Run(s) << std::endl;
        } catch (SyntaxError& e) {
            std::cout << "Syntax error: " << e.what() << std::endl;
        } catch (NameError& e) {
            std::cout << "Name error: " << e.what() << std::endl;
        } catch (RuntimeError& e) {
            std::cout << "Runtime error: " << e.what() << std::endl;
        }
    }
    return 0;
}
