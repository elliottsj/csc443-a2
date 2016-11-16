#include <iostream>
#include <numeric>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <cmath>
#include <cstring>
#include "library.h"

/**
 * Print the current errno and exit with status 1.
 */
void error(const char *message) {
    perror(message);
    exit(1);
}

/**
  * Returns result plus length of current.
  */
int sum_strlen(int result, const char* current) {
    // Apparently, strlen(current) will always be the same size
    // https://goo.gl/KRneIj
    return result + std::strlen(current);
}

/**
 * Compute the number of bytes required to serialize record.
 */
int fixed_len_sizeof(Record *record) {
    return std::accumulate(record->begin(), record->end(), 0, sum_strlen);
};

/**
 * Serialize the record to a byte array to be stored in buf.
 */
void fixed_len_write(Record *record, void *buf) {
    for (unsigned int j = 0; j < record->size(); j++){
        // copy the string into the buf
        int string_length = std::strlen(record->at(j));
        int position = (j * string_length);
        std::memcpy((char*) buf + position, record->at(j), string_length);
    }
};

/**
 * Deserializes `size` bytes from the buffer, `buf`, and
 * stores the data in `record`.
 */
void fixed_len_read(void *buf, int size, Record *record) {
    record->reserve(100);
    for (int i = 0; i < 100; i++) {
        // char * buf_value = NULL;
        // memcpy(buf_value, (char*)buf + i, 1);
        record->push_back((char*)buf + i);
    }
}


/**
 * Calculates the maximal number of records that fit in a page
 */
int fixed_len_page_capacity(Page *page){
    int num_slots = page->page_size/page->slot_size;

    // find out how many slots our directory is going to need
    int number_of_directory_slots = std::ceil(page->size_of_directory / page->slot_size);

    return num_slots - number_of_directory_slots;
};


/** Initialize the empty directory.

 *  A directory which holds a 0 if a slot is empty or a 1 if the slot has data.
 */
void init_directory(Page *page){
    // First byte in directory: set leading bits to match slot count
    // e.g. if there are 11 data slots, directory should look like:
    // 1111 1000 0000 0000
    unsigned char *first_byte = ((unsigned char *) page->data) + page->page_size - page->size_of_directory;
    // Number of bits to set is number of directory bits minus number of data slots
    // e.g. 16 - 11 == 5
    int num_bits_to_set = (8 * page->size_of_directory) - fixed_len_page_capacity(page);
    // Set first byte by shifting (8 - num_bits_to_set) zeros to the left
    *first_byte = 0xFF << (8 - num_bits_to_set);

    // Set all subsequent bytes to 0
    for (int i = 1; i < page->size_of_directory; i++) {
        unsigned char *byte = ((unsigned char *) page->data) + page->page_size - i;
        *byte = 0;
    }
}

/**
 * Initializes a page using the given slot size.
 * Calculates size of directory and puts it in inside the page data. A directory is held
 * at the end of the data in the Page.

 * A directory is a bitmap.
 * Directory contents are traversed backwards.

 * Say we want to insert a record, we look into the directory to figure out
 * which slot is free. Traversing backwards inside the directory allows us to
 * not keep track of directory offset.

 * Accepts only clean divisions of page_size/slot_size.
 */
void init_fixed_len_page(Page *page, int page_size, int slot_size){
    int num_slots = page_size/slot_size;

    page->page_size = page_size;
    page->slot_size = slot_size;
    // size_of_directory is in bytes
    page->size_of_directory = std::ceil(num_slots/(double)8);

    page->data = malloc(page_size);

    // Insert directory at the end of the page.
    init_directory(page);
};


/**
 * Calculate the free space (number of free slots) in the page.data

 * Returns:
 *  Number of free slots.
 */
int fixed_len_page_freeslots(Page *page){
    int freeslots = 0;
    // get the directory
    unsigned char* directory = ((unsigned char *) page->data) + page->page_size - page->size_of_directory;
    // traverse the directory and calculate the number of free slots
    for (int i = 0; i < page->size_of_directory;i++){
        for (int j = 0;j < 8;j++) {
            char shifted = directory[i] >> j;
            freeslots += (shifted & 1) == 0;
        }
    }
    return freeslots;
};

/**
 * Write a record into a given slot.
 */
void write_fixed_len_page(Page *page, int slot, Record *r){
    // get the slot
    unsigned char* slot_in_page = ((unsigned char *) page->data) + page->slot_size * slot;
    // serialize the data in r and write to slot_in_page
    fixed_len_write(r, (void *) slot_in_page);
}

/**
 * Add a record to the page.
 * Returns:
 *   record slot offset if successful,
 *   -1 if unsuccessful (page full)
 */
int add_fixed_len_page(Page *page, Record *record){
    // get free slot
    unsigned char* directory = ((unsigned char *) page->data) + page->page_size;
    int first_free_slot = -1;

    // return early if no free pages
    if (fixed_len_page_freeslots(page) == 0){
        return first_free_slot;
    }

    for (int i = page->size_of_directory - 1; i >= 0;i--){
        for (int j = 7; j >= 0; j--){
            char shifted = directory[i - 1] >> j;
            if ((shifted & 1) == 0){
                first_free_slot = j;
                // turn the bit at j to 1 from 0
                unsigned char bit_to_change = 0x1 << j;
                directory[i - 1] = directory[i - 1] | bit_to_change;
                break;
            }
        }
        if (first_free_slot != -1){
            break;
        }
    }

    // write the record to the slot
    write_fixed_len_page(page, first_free_slot, record);

    return first_free_slot;
}

/**
 * Read a record from the page from a given slot.
 */
void read_fixed_len_page(Page *page, int slot, Record *r){
    // get the data slot in the page
    char* data_slot = ((char *) page->data) + (page->slot_size * slot);
    // serialize the data at the dataslot and store in r
    fixed_len_read(data_slot, page->slot_size, r);
}


// Heap file functions, maybe put this in a new file?

/**
 * Initialize a heapfile to use the file and page size given.
 */
void init_heapfile(Heapfile *heapfile, int page_size, FILE *file){
    heapfile->file_ptr = file;
    heapfile->page_size = page_size;

    // init directory
    Page directory_page;
    init_fixed_len_page(&directory_page, page_size, 1000);
    directory_page.next_directory = false;

    // write directory to file
    fwrite((const char *) directory_page.data, page_size, 1, file);
}

/**
 * Allocate another page in the heapfile. This grows the file by a page.
 */
PageID alloc_page(Heapfile *heapfile){
    PageID page_id = 0;

    int is_free_space = 0;

    // get start of file
    int curr_heapfile_ptr = 0;

    // iterate until we find a directory with free space
    // TODO: what if we have no free space?
    Page directory_page;
    while(!is_free_space){
        // get directory
        init_fixed_len_page(&directory_page, heapfile->page_size, 1000);
        // TODO: rewrite this to use fseek
        fread(directory_page.data, heapfile->page_size, 1, heapfile->file_ptr + curr_heapfile_ptr);
        is_free_space = fixed_len_page_freeslots(&directory_page) != 0;
        // if we are going to go to the next directory slot, add to page_id
        int total_slots = fixed_len_page_capacity(&directory_page);

        if (!is_free_space){
            page_id += total_slots;
            // move pointer to where the next directory would be
            curr_heapfile_ptr += heapfile->page_size * (total_slots + 1);
        }
    }

    // we now have a directory with free space! yupii!

    // create a new record to be put into directory_page
    Record record;

    // {freespace, page_offset} is used so we can re-use Record struct!

    // create new data page
    Page data_page;
    init_fixed_len_page(&data_page, heapfile->page_size, 1000);

    // freespace is just capacity as nothing is inserted into this new data_page
    int freespace = fixed_len_page_capacity(&data_page);
    std::string freespace_string = std::to_string(freespace);
    record.push_back(freespace_string.c_str());

    // add record to directory_page for the new data_page we are allocating
    int slot_written_to = add_fixed_len_page(&directory_page, &record);

    // calculate offset
    int offset = heapfile->page_size * (slot_written_to + 1);
    std::string page_size_string = std::to_string(offset);
    record.push_back(page_size_string.c_str());

    // write new data page to correct spot in heapfile
    curr_heapfile_ptr += offset;
    fwrite(data_page.data, heapfile->page_size, 1, heapfile->file_ptr);

    // free_slot we put data_page into will be the index
    page_id += slot_written_to;

    return page_id;
}

/**
 * Read a page into memory.
 * Takes the page from the heapfile and puts it into page.

 * > The page ID of p can be the index of its entry in the directory.
 * Because entries in the directory are in the same order as the entries in an
 * heapfile actual file, we can just skip ahead and not need to read each directory page
 * as we can do math to find where page is

 * General strategy:
 * - pid is an index of where the page is in all of the data page_sizes
 * - Can figure out how many data_pages are in each set between directory pages
 * - Use this number to figure out how far in the heap_file to go
 */
void read_page(Heapfile *heapfile, PageID pid, Page *page){
    // get directory page
    Page directory_page;
    init_fixed_len_page(&directory_page, heapfile->page_size, 1000);
    fread(directory_page.data, heapfile->page_size, 1, heapfile->file_ptr);

    // find out how many data_pages per directory page
    int number_of_data_pages = fixed_len_page_capacity(&directory_page);

    //how many directory pages are we away from the beginning?
    // TODO: Can we assume this? Are we suppose to look into directory records
    // to find offsets?
    int directory_offset = 0;

    while (pid > number_of_data_pages){
        number_of_data_pages = pid - number_of_data_pages;
        directory_offset += 1;
    }
    init_fixed_len_page(page, heapfile->page_size, 1000);
    fread(
        directory_page.data,
        heapfile->page_size,
        1,
        heapfile->file_ptr + (heapfile->page_size * (directory_offset * number_of_data_pages)) // calculation of where the data_page will be
    );
}


/**
 * Write a page from memory to disk
 * TODO: do we use freespace in the data entries for anything?
 */
void write_page(Page *page, Heapfile *heapfile, PageID pid){
    // get directory page
    Page directory_page;
    init_fixed_len_page(&directory_page, heapfile->page_size, 1000);
    fread(directory_page.data, heapfile->page_size, 1, heapfile->file_ptr);

    int number_of_data_pages = fixed_len_page_capacity(&directory_page);

    int directory_offset = 0;

    while (pid > number_of_data_pages){
        number_of_data_pages = pid - number_of_data_pages;
        directory_offset += 1;
    }

    fwrite(
        page->data,
        heapfile->page_size,
        1,
        heapfile->file_ptr + (heapfile->page_size * (directory_offset * number_of_data_pages))
    );
}

/**
 * Constructor for RecordIterator.
 * RecordIterator should be able to iterate through records.

 * Sets up methods next() to return the first record in the heapfile.
 */
RecordIterator::RecordIterator(Heapfile *heapfile){
    heapfile = heapfile;

    // get directory page
    Page directory_page;
    init_fixed_len_page(&directory_page, heapfile->page_size, 1000);
    fread(directory_page.data, heapfile->page_size, 1, heapfile->file_ptr);

    current_directory = directory_page;

    current_data_slot = 0;

    // get first record in this directory
    Record record;
    read_fixed_len_page(&directory_page, current_data_slot, &record);

    current_data_slot += 1;

    // get data_page at offset
    int offset = atoi(record.at(1));

    // init data_page
    Page data_page;
    init_fixed_len_page(&data_page, heapfile->page_size, 1000);
    fread(data_page.data, heapfile->page_size, 1, heapfile->file_ptr + offset);

    // set current_page
    current_data = data_page;

    current_record_slot = 0;
}


Page RecordIterator::get_next_directory_page(){
    Page new_directory_page;
    init_fixed_len_page(&new_directory_page, heapfile->page_size, 1000);

    fread(
        new_directory_page.data,
        heapfile->page_size,
        1,
        // where the new directory will be
        heapfile->file_ptr + (heapfile->page_size * fixed_len_page_capacity(&current_directory)) + 1
    );

    return new_directory_page;
}

Page RecordIterator::get_next_data_page(){
    Record new_record;
    read_fixed_len_page(&current_directory, current_data_slot, &new_record);

    Page new_data_page;
    init_fixed_len_page(&new_data_page, heapfile->page_size, 1000);
    int offset = atoi(new_record.at(1));
    fread(new_data_page.data, heapfile->page_size, 1, heapfile->file_ptr + offset);

    return new_data_page;
}

/**
 * Returns the next Record.
 * TODO: dont assume we have records in each data_page slot
 * TODO: what if we are at the last data_page in the current_directory?
 */
Record RecordIterator::next(){
    // check if we are out of records for this current_data page
    if (current_record_slot > fixed_len_page_capacity(&current_data)) {
        // check if we are out of data_pages for this current_directory page
        if (current_data_slot > fixed_len_page_capacity(&current_directory)) {
            // calcuate offset to next directory_page
            // TODO: what if there is no next directory_page???
            Page new_directory_page;
            current_directory = get_next_directory_page();
            current_data_slot = 0;
        }
        // get next data_page
        Page new_data_page;
        current_data = get_next_data_page();
        // move data_slot up one
        current_data_slot += 1;
        // reset record slot as we are beginning from the start again
        current_record_slot = 0;
    }

    // return record at current slot
    Record record;
    read_fixed_len_page(&current_data, current_data_slot, &record);

    current_data_slot += 1;

    return record;
}


/**
 * Returns True if the RecordIterator has a value for next().
 */
bool RecordIterator::hasNext(){
    Page temp_current_data;
    temp_current_data = current_data;

    int temp_current_record_slot = current_record_slot;

    int capacity_of_data_page = fixed_len_page_capacity(&current_data);
    // if we are currently outside the current_data pages capacity
    // need to keep check if the next data_page has a record in its first slot
    if (current_record_slot > capacity_of_data_page) {
        // check if we are out of data_pages for this current_directory page
        if (current_data_slot > fixed_len_page_capacity(&current_directory)) {
            // calcuate offset to next directory_page
            // TODO: what if there is no next directory_page???
            get_next_directory_page();
        }

        // get next data_page
        temp_current_data = get_next_data_page();

        temp_current_record_slot = 0;
    }

    int freespace_in_data_page = fixed_len_page_freeslots(&temp_current_data);
    capacity_of_data_page = fixed_len_page_capacity(&temp_current_data);
    if (capacity_of_data_page - temp_current_record_slot <= freespace_in_data_page) {
        return false;
    }

    return true;
}
