#include <iostream>
#include <fstream>
#include <sstream>
#include <istream>
#include <sys/timeb.h>
#include "library.h"
#include <string>

/*
    page_file is a file containing binary data filled with a binary repr of page data.
    This page data includes records and also directories. For this to work, it must use
    the same page_size as a previous ./write_fixed_len_page call did.
 */
int main(int argc, const char * argv[]) {
    if (argc < 3) {
        std::cout << "Usage: read_fixed_len_page <page_file> <page_size>";
        return 1;
    }

    std::string page_filename(argv[1]);
    int page_size = std::stoi(argv[2]);

    // start timer
    struct timeb t;
    ftime(&t);
    unsigned long start_ms = t.time * 1000 + t.millitm;

    std::ifstream page_file;
    page_file.open(page_filename, std::ios::in | std::ios::binary);

    // switch this to tuples2.csv to see output
    FILE * dev_null = fopen("tuples2.csv", "w");

    // int slot_size = 1000;
    // int num_slots = page_size/slot_size;

    // for output
    int number_of_records = 0;
    int number_of_pages = 0;

    while (!page_file.eof()){
        Page page;
        // Initialize the page
        init_fixed_len_page(&page, page_size, 1000);

        // Read data from page_file and insert into page.data
        page_file.read((char *) page.data, page_size);

        // Read each record from the page.data and serialize into csv form for output!
        for (int i = 0; i < fixed_len_page_capacity(&page); i++) {
            // now that we have the data in the page,
            // deserialize each slot in the page to get records!
            Record record;
            read_fixed_len_page(&page, i, &record);

            // output record data to dev_null
            for (unsigned int j = 0; j < record.size(); j++) {
                std::cout << record.at(j);
                // fputs(record.at(j), dev_null);
                if (j != record.size() - 1){
                    std::cout << ",";
                    // fputs(",", dev_null);
                }
            }
            fputs("\n", dev_null);
            number_of_records += 1;
        }
        number_of_pages += 1;
    }
    fclose(dev_null);
    page_file.close();

    // stop timer
    ftime(&t);
    unsigned long stop_ms = t.time * 1000 + t.millitm;

    std::cout << "NUMBER OF RECORDS: " << number_of_records << "\n";
    std::cout << "NUMBER OF PAGES: " << number_of_pages << "\n";
    std::cout << "TIME: " << stop_ms - start_ms << " milliseconds\n";

    return 0;
}
