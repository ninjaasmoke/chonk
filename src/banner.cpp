#include <iostream>
#include "banner.h"

void printBanner()
{
    std::cout << "\033[1;36m";
    std::cout << "============================\n";
    std::cout << "     FILE SPLITTER CLI      \n";
    std::cout << "============================\n";
    std::cout << "\033[0m";
}