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
    heapfile_pointer = std::fopen(heapfile_filename.c_str(), "w+b");

    // Read the CSV file line-by-line:
    std::ifstream csv_file(csv_filename);
    std::string line;

    Heapfile heapfile;
    init_heapfile(&heapfile, page_size, heapfile_pointer);

    alloc_page(&heapfile);

    // use write_fixed_len_page to write all tuples to data file


    // convert data_file to use new heapfile format

    // for (int i = 0;i < heapfile->page_size * )
    // Page data_page;
    // void * page_data = fread(data_page, heapfile.page_size, 1, data_file);

    // write_page(Page *page, Heapfile *heapfile, PageID pid)

    return 0;
}
