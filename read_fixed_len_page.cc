#include <iostream>
#include <fstream>
#include <sstream>
#include <istream>
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

    std::ifstream page_file;
    page_file.open(page_filename, std::ios::out | std::ios::binary);

    // switch this to tuples2.csv to see output
    FILE * dev_null = fopen("/dev/null", "w");

    int slot_size = 1000;
    int num_slots = page_size/slot_size;

    while (true){
        Page page;
        // Initialize the page
        init_fixed_len_page(&page, page_size, 1000);

        // Read data from page_file and insert into page.data
        page_file.read((char *) page.data, page_size);

        // Read each record from the page.data and serialize into csv form for output!
        for (int i = 0;i < num_slots;i++) {
            // now that we have the data in the page,
            // deserialize each slot in the page to get records!
            Record record;
            read_fixed_len_page(&page, i, &record);

            // output record data to dev_null
            for (int j = 0;j < record.size() - 1;j++){
                std::string mycppstr(record.at(i));
                std::string to_write = mycppstr + ",";
                fputs(to_write.c_str(), dev_null);
            }
            fputs("\n", dev_null);
        }

        if(page_file.eof()){
            break;
        }
    }
    fclose(dev_null);
    page_file.close();
}
