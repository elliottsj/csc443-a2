#include <iostream>
#include <fstream>
#include <sstream>
#include "library.h"

int main(int argc, const char * argv[]) {
    if (argc < 4) {
        std::cout << "Usage: csv2heapfile <csv_file> <heapfile> <page_size>";
        return 1;
    }
    std::string csv_filename(argv[1]);
    std::string heapfile_filename(argv[2]);
    int page_size = std::stoi(argv[3]);

    // Open the page file for writing
    FILE * heapfile_pointer;
    heapfile_pointer = std::fopen(heapfile_filename.c_str(), "wb");

    // Read the CSV file line-by-line:
    std::ifstream csv_file(csv_filename);
    std::string line;

    Heapfile heapfile;
    init_heapfile(&heapfile, page_size, heapfile_pointer);

    alloc_page(&heapfile);

    return 0;
}
