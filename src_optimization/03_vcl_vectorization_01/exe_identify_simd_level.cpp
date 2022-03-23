

#include <iostream>

#include "vcl/vectorclass.h"

int main()
{
    int l_level = instrset_detect();
    Vec4q a;
    std::cout << "SIMD Level:" << l_level                           << std::endl;
    std::cout << "supported instruction sets:"                      << std::endl;
    switch ( l_level )
    {
        case 10:
            std::cout << "  " << "AVX512VL, AVX512BW, and AVX512DQ" << std::endl;
        case 9:
            std::cout << "  " << "AVX512F"                          << std::endl;
        case 8:
            std::cout << "  " << "AVX2"                             << std::endl;
        case 7:
            std::cout << "  " << "AVX supported by CPU and O.S."    << std::endl;
        case 6:
            std::cout << "  " << "SSE4.2"                           << std::endl;
        case 5:
            std::cout << "  " << "SSE4.1"                           << std::endl;
        case 4:
            std::cout << "  " << "Supplementary SSE3 (SSSE3)"       << std::endl;
        case 3:
            std::cout << "  " << "SSE3"                             << std::endl;
        case 2:
            std::cout << "  " << "SSE2"                             << std::endl;
        case 1:
            std::cout << "  " << "SSE supported by CPU"             << std::endl;
        case 0:
            std::cout << "  " << "80386 instruction set"            << std::endl;
            break;
        default:
            std::cout << "ERROR!"                                   << std::endl;
    }
    return 0;
}
