#include <vector>

typedef const char* V;
typedef std::vector<V> Record;

typedef struct {
    void *data; // Points to a byte array (Record).
    int page_size; // The total page_size, implicitly includes size_of_directory
    int slot_size; // How big a Record is.
    int size_of_directory; // The size of our directory in bytes.
} Page;

/**
 * Compute the number of bytes required to serialize record
 */
int fixed_len_sizeof(Record *record);

/**
 * Serialize the record to a byte array to be stored in buf.
 */
void fixed_len_write(Record *record, void *buf);

/**
 * Deserializes `size` bytes from the buffer, `buf`, and
 * stores the record in `record`.
 */
void fixed_len_read(void *buf, int size, Record *record);

int fixed_len_page_capacity(Page *page);
void init_directory(Page *page);
void init_fixed_len_page(Page *page, int page_size, int slot_size);
int fixed_len_page_freeslots(Page *page);
void write_fixed_len_page(Page *page, int slot, Record *r);
int add_fixed_len_page(Page *page, Record *r);
void read_fixed_len_page(Page *page, int slot, Record *r);
