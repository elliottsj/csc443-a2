#include <iostream>

int main(int argc, const char * argv[]) {
    if (argc < 4) {
        std::cout << "Usage: scan <heapfile> <page_size>";
        return 1;
    }
    std::string heapfile_filename(argv[1]);
    int page_size = std::stoi(argv[2]);


    return 0;
}
