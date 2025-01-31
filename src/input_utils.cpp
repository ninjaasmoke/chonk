#include "input_utils.h"
#include <iostream>
#include <termios.h>
#include <unistd.h>

std::string getHiddenInput(const std::string &prompt)
{
    std::cout << prompt;
    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    std::string input;
    std::getline(std::cin, input);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << "\n";
    return input;
}