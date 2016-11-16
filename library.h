#include <vector>


// PAGE
typedef const char* V;
typedef std::vector<V> Record;

typedef struct {
    void *data; // Points to a byte array (Record).
    int page_size; // The total page_size, implicitly includes size_of_directory
    int slot_size; // How big a Record is.
    int size_of_directory; // The size of our directory in bytes.
    bool next_directory;
} Page;


// HEAPFILE

typedef struct {
    FILE *file_ptr;
    int page_size;
} Heapfile;

// unique across the entire heapfile
typedef int PageID;

typedef struct {
    int page_id;
    int slot;
} RecordID;

class RecordIterator {
    public:
    RecordIterator(Heapfile *heapfile);
    Record next();
    bool hasNext();
    Page get_next_directory_page();
    Page get_next_data_page();
    Heapfile *current_heapfile;
    Page current_directory;
    int current_data_slot; // what data_page slot we are at in this directory
    Page current_data;
    int current_record_slot; // what record slot we are at in this data_page
};

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

// heapfiles
void init_heapfile(Heapfile *heapfile, int page_size, FILE *file);
PageID alloc_page(Heapfile *heapfile);
