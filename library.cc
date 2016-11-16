#include <iostream>
#include <numeric>
#include <vector>
#include <algorithm>
#include <stdint.h>
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
    uint8_t *first_byte = ((uint8_t *) page->data) + page->page_size - page->size_of_directory;
    // Number of bits to set is number of directory bits minus number of data slots
    // e.g. 16 - 11 == 5
    int num_bits_to_set = (8 * page->size_of_directory) - fixed_len_page_capacity(page);
    // Set first byte by shifting (8 - num_bits_to_set) zeros to the left
    *first_byte = 0xFF << (8 - num_bits_to_set);

    // Set all subsequent bytes to 0
    for (int i = 1; i < page->size_of_directory; i++) {
        uint8_t *byte = ((uint8_t *) page->data) + page->page_size - i;
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
 * Return the position of the first free slot in the given page.
 * Return -1 if none exist.
 */
int fixed_len_page_find_freeslot(Page *page) {
    uint8_t* directory = ((uint8_t *) page->data) + page->page_size - page->size_of_directory;
    for (int i = 0; i < page->size_of_directory; i++) {
        for (int j = 0; j < 8; j++) {
            if (((directory[page->size_of_directory - i - 1] >> j) & 1) == 0) {
                return (8 * i) + j;
            }
        }
    }
    return -1;
}

/**
 * Calculate the free space (number of free slots) in the page.data
 * Returns:
 *  Number of free slots.
 */
int fixed_len_page_freeslots(Page *page) {
    int freeslots = 0;
    // get the directory
    uint8_t* directory = ((uint8_t *) page->data) + page->page_size - page->size_of_directory;
    // traverse the directory and calculate the number of free slots
    for (int i = 0; i < page->size_of_directory;i++){
        for (int j = 0; j < 8; j++) {
            freeslots += ((directory[i] >> j) & 1) == 0;
        }
    }
    return freeslots;
};

/**
 * Write a record into a given slot.
 */
void write_fixed_len_page(Page *page, int slot, Record *r){
    // get the slot
    uint8_t* slot_in_page = ((uint8_t *) page->data) + page->slot_size * slot;
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
    uint8_t* directory = ((uint8_t *) page->data) + page->page_size;

    // return early if no free pages
    if (fixed_len_page_freeslots(page) == 0){
        return -1;
    }

    // Find the first free slot and set its bit in the directory
    int first_free_slot = fixed_len_page_find_freeslot(page);
    uint8_t *directory_entry = ((uint8_t *) page->data) + page->page_size - 1 - first_free_slot / 8;
    *directory_entry |= (first_free_slot % 8);

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
 * Append an empty directory to the given heapfile.
 */
void append_empty_directory(Heapfile *heapfile) {
    Page directory_page;
    init_fixed_len_page(&directory_page, page_size, /* slot_size: */ 1000);
    if (fwrite((const char *) directory_page.data, page_size, 1, file) < 1) {
        error("append_empty_directory fwrite");
    }
}

/**
 * Initialize a heapfile to use the file and page size given.
 */
void init_heapfile(Heapfile *heapfile, int page_size, FILE *file){
    heapfile->file_ptr = file;
    heapfile->page_size = page_size;

    append_empty_directory(heapfile);
}

/**
 * Allocate another page in the heapfile. This grows the file by a page.
 */
PageID alloc_page(Heapfile *heapfile) {
    PageID page_id = 0;
    int directory_free_slot = -1;

    // Iterate until we find a directory with free space
    Page directory_page;
    while (feof(heapfile->file_ptr) != 0 && directory_free_slot == -1) {
        // While we have not reached the end of the heap file and have not yet found a directory with free space
        // Read a directory from the heap file
        init_fixed_len_page(&directory_page, heapfile->page_size, /* slot_size: */ 1000);
        if (fread(directory_page.data, heapfile->page_size, 1, heapfile->file_ptr) < 1 && ferror(heapfile->file_ptr) != 0) {
            error("fread");
        }
        directory_free_slot = fixed_len_page_find_freeslot(&directory_page);
        int page_capacity = fixed_len_page_capacity(&directory_page);
        if (directory_free_slot == -1) {
            // Current directory has no free space available
            page_id += page_capacity;
            // Advance file position to the next directory
            if (fseek(heapfile->file_ptr, heapfile->page_size * page_capacity, SEEK_CUR) != 0) {
                error("next directory fseek");
            }
        }
    }

    if (directory_free_slot == -1) {
        // All directories are full; add a new directory
        append_empty_directory(heapfile);
    }

    // We now have a directory with free space

    // Create a new data page
    Page data_page;
    init_fixed_len_page(&data_page, heapfile->page_size, /* slot_size: */ 1000);

    // Create a new record to be put into directory_page
    Record record;

    // {freespace, page_offset} is used so we can re-use Record struct!

    // freespace is just capacity as nothing is inserted into this new data_page
    int freespace = fixed_len_page_capacity(&directory_page);
    std::string freespace_string = std::to_string(freespace);
    record.push_back(freespace_string.c_str());

    // Calculate offset in bytes
    int offset = heapfile->page_size * (directory_free_slot + 1);
    std::string offset_str = std::to_string(offset);
    record.push_back(offset_str.c_str());

    // Add record to directory_page for the new data_page we are allocating
    int slot_written_to = add_fixed_len_page(&directory_page, &record);
    assert(directory_free_slot == slot_written_to);

    // Write new data page to correct spot in heapfile
    if (fseek(heapfile->file_ptr, offset, SEEK_CUR) != 0) {
        error("data page fseek");
    }
    if (fwrite((const char *) data_page.data, heapfile->page_size, 1, heapfile->file_ptr) < 1) {
        error("data page fwrite");
    }

    // Free slot we put data_page into will be the index
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
