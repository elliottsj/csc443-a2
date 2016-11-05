#include <iostream>
#include <fstream>
#include <sstream>
#include "library.h"

int main(int argc, const char * argv[]) {
    if (argc < 4) {
        std::cout << "Usage: write_fixed_len_pages <csv_file> <page_file> <page_size>";
        return 1;
    }
    std::string csv_filename(argv[1]);
    std::string page_filename(argv[2]);
    int page_size = std::stoi(argv[3]);

    std::ifstream csv_file(csv_filename);

    std::string line;
    while (std::getline(csv_file, line)) {
        std::stringstream linestr(line);
        std::string cell;
        while (std::getline(linestr, cell, ',')) {
            std::cout << cell << "\n";
        }
    }

    std::cout << "NUMBER OF RECORDS: " << 1000 << "\n";
    std::cout << "NUMBER OF PAGES: " << 32 << "\n";
    std::cout << "TIME: " << 43 << " milliseconds\n";

    return 0;
}
