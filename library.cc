#include <numeric>
#include <vector>


typedef const char* V;
typedef std::vector<V> Record;


/**
  * Returns result plus length of current.
  */
int sum_strlen(int result, char* current){
    // Apparently, strlen(current) will always be the same size
    // https://goo.gl/KRneIj
    return result + strlen(current);
}

/**
 * Compute the number of bytes required to serialize record.
 */
int fixed_len_sizeof(Record *record){
    return std::accumulate (record.begin(), record.end(), 0, sum_strlen);
};

/**
 * Serialize the record to a byte array to be stored in buf.
 */
void fixed_len_write(Record *record, void *buf){
    // iterate through the record
    for (int j = 0; j < record.size(); j++){
        // copy the string into the buf
        int string_length = strlen(record[j]);
        int position = (j * string_length);
        memcpy((char*) buf + position, record[j], string_length);
    }
};
