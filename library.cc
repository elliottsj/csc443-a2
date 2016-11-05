#include <numeric>
#include <vector>
#include <stdlib.h>
#include <cmath>

typedef const char* V;
typedef std::vector<V> Record;


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
    for (int j = 0; j < record->size(); j++){
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
    for (int i = 0; i < record->size(); i++) {
        std::memcpy((void *) record->at(i), ((char *) buf) + i * value_length, value_length);
    }
}


typedef struct {
    void *data; // Points to a byte array (Record).
    int page_size;
    int slot_size; // How big a Record is.
    int directory_offset; // How many slots away from the beginning is our initial Record.
} Page;


/**
 * Calculates the maximal number of records that fit in a page
 */
int fixed_len_page_capacity(Page *page){
    int num_slots = page->page_size/page->slot_size;

    // in bytes
    int size_of_directory = num_slots/8;

    // find out how much slots our directory is going to need, round up
    int number_of_directory_slots = ceil(size_of_directory/8 / page->slot_size);

    return num_slots - number_of_directory_slots;
};

/**
 * Initializes a page using the given slot size.
 * Calculates size of directory and initilizes inside the data. A directory is held
 * at the end of the data in the Page.

 * Accepts only clean divisions of page_size/slot_size.
 */
void init_fixed_len_page(Page *page, int page_size, int slot_size){
    int num_slots = page_size/slot_size;

    // in bytes
    int size_of_directory = num_slots/8;

    page->page_size = page_size;
    page->slot_size = slot_size;

    int total_page_slots = fixed_len_page_capacity(page);

    void *new_data = malloc(page_size);
    page->data = new_data;

    // insert directory at the end of the page.

    // Initialize the empty directory.
    // A directory which holds a 0 if a slot is empty or a 1 if the slot has data.
    for (int i = 0; i < size_of_directory;i++) {
        char* b = ((char *) page->data) + page_size - size_of_directory + i;
        *b = 0;
    }
};


/**
 * Calculate the free space (number of free slots) in the page
 */
int fixed_len_page_freeslots(Page *page);

/**
 * Add a record to the page
 * Returns:
 *   record slot offset if successful,
 *   -1 if unsuccessful (page full)
 */
int add_fixed_len_page(Page *page, Record *r);

/**
 * Write a record into a given slot.
 */
void write_fixed_len_page(Page *page, int slot, Record *r);

/**
 * Read a record from the page from a given slot.
 */
void read_fixed_len_page(Page *page, int slot, Record *r);
