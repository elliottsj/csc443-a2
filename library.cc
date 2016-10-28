#include <numeric>
#include <vector>


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
