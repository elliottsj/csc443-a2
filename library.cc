#include <iostream>
#include <numeric>
#include <vector>
#include <stdlib.h>
#include <cmath>
#include <cstring>
#include "library.h"

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
    // iterate through the record
    for (unsigned int j = 0; j < record->size(); j++){
        // copy the string into the buf
        int string_length = std::strlen(record->at(j));
        int position = (j * string_length);
        std::memcpy((char*) buf + position, record->at(j), string_length);
    }
};

/**
 * Deserializes `size` bytes from the buffer, `buf`, and
 * stores the record in `record`.
 */
void fixed_len_read(void *buf, int size, Record *record) {
    int value_length = size / record->size();
    for (unsigned int i = 0; i < record->size(); i++) {
        std::memcpy((void *) record->at(i), ((char *) buf) + i * value_length, value_length);
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
    for (int i = 0; i < page->size_of_directory;i++) {
        char* b = ((char *) page->data) + page->page_size - page->size_of_directory + i;
        *b = 0;
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
int add_fixed_len_page(Page *page, Record *r){
    // get free slot
    unsigned char* directory = ((unsigned char *) page->data) + page->page_size;
    int first_free_slot = -1;

    for (int i = page->size_of_directory - 1; i >= 0;i--){
        for (int j = 7; j >= 0; j--){
            char shifted = directory[i - 1] >> j;
            if ((shifted & 1) == 0){
                first_free_slot = j;
                break;
            }
        }
        if (first_free_slot != -1){
            break;
        }
    }

    // write the record to the slot
    write_fixed_len_page(page, first_free_slot, r);

    return first_free_slot;
}

/**
 * Read a record from the page from a given slot.
 */
void read_fixed_len_page(Page *page, int slot, Record *r){
    // get the data slot in the page
    char* data_slot = ((char *) page->data) + page->slot_size * slot;
    // serialize the data at the dataslot and store in r
    fixed_len_read(data_slot, page->slot_size, r);
}
