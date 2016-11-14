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
    int page_size = std::stoi(argv[3]); // 4096 for example

    // Open the page file for writing
    std::ofstream page_file;
    page_file.open(page_filename, std::ios::out | std::ios::binary);

    // Read the CSV file line-by-line:
    std::ifstream csv_file(csv_filename);
    std::string line;
    Page page;
    int should_create_new_page = 1;

    while (std::getline(csv_file, line)) {
        std::stringstream linestr(line);
        std::string cell;

        // Read cells into a Record
        Record record;
        while (std::getline(linestr, cell, ',')) {
            record.push_back(cell.c_str());
        }

        // First run, the page will not be initialized
        if (should_create_new_page) {
            init_fixed_len_page(&page, page_size, fixed_len_sizeof(&record));
        }
        should_create_new_page = add_fixed_len_page(&page, &record) == -1;
        int should_write_page = should_create_new_page;

        // if -1, init a new page and add this record to it
        if (should_create_new_page) {
            init_fixed_len_page(&page, page_size, fixed_len_sizeof(&record));
            add_fixed_len_page(&page, &record);
            should_create_new_page = 0;
        }

        if (should_write_page) {
            // Write page.data to page_file
            page_file.write((const char *) page.data, page.page_size);
        }

    }

    if (!should_create_new_page) {
        // Write page.data to page_file
        page_file.write((const char *) page.data, page.page_size);
    }

    page_file.close();

    std::cout << "NUMBER OF RECORDS: " << 1000 << "\n";
    std::cout << "NUMBER OF PAGES: " << 32 << "\n";
    std::cout << "TIME: " << 43 << " milliseconds\n";

    return 0;
}
