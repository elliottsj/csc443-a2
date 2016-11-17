#!/usr/bin/env python3

import csv
import os
import shutil
import subprocess
import sys


def read_fixed_len_pages(page_file, page_size):
    """
    Execute ./read_fixed_len_pages with the given arguments and return the number of milliseconds
    it took to read the file.
    """
    result = subprocess.run(
        [
            os.path.join(os.path.dirname(os.path.realpath(__file__)), './read_fixed_len_pages'),
            page_file,
            str(page_size),
        ],
        stdout=subprocess.PIPE,
    )
    # parse the result to get the desired time
    return int(result.stdout.split()[-2])


def main():
    """
    Call ./read_fixed_len_pages to write to example_page_file with ten different block sizes,
    each in a range of 10 to 100MB.
    NOTE: piazza says use 500mb???
    https://piazza.com/class/isubcms1kpl3xq?cid=92
    I would assume we can only do this if we make tuples.csv larger.

    """
    if len(sys.argv) < 3:
        print('Usage: experiment_read_fixed_len_page <page_file> <page_size>')
        sys.exit(1)
    page_file = sys.argv[1]
    page_size = sys.argv[2]

    page_sizes = [
        128,           # 128 B
        512,           # 512 B
        1 * 2 ** 10,   # 1 KiB
        4 * 2 ** 10,   # 4 KiB
        8 * 2 ** 10,   # 8 KiB
        64 * 2 ** 10,  # 64 KiB
        # 128 * 2 ** 10,  # 128 KiB
        # 512 * 2 ** 10,  # 512 KiB
        # 1 * 2 ** 20,   # 1 MiB
        # 2 * 2 ** 20,   # 2 MiB
    ]

    if os.path.exists('./out'):
        shutil.rmtree('./out')
    os.mkdir('./out')
    csvwriter = csv.DictWriter(
        sys.stdout,
        fieldnames=('page_size', 'milliseconds_elapsed'),
        dialect='unix'
    )
    csvwriter.writeheader()
    for page_size in page_sizes:
        for i in range(50):
            ms_elapsed = read_fixed_len_pages(
                page_file,
                page_size,
            )
            csvwriter.writerow({
                'page_size': page_size,
                'milliseconds_elapsed': ms_elapsed,
            })

if __name__ == '__main__':
    main()
