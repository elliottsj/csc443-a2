#!/usr/bin/env python3

import csv
import os
import shutil
import subprocess
import sys


def write_fixed_len_pages(csv_file, output_file, page_size):
    """
    Execute ./write_fixed_len_pages with the given arguments and return the number of milliseconds
    it took to create the file.
    """
    result = subprocess.run(
        [
            os.path.join(os.path.dirname(os.path.realpath(__file__)), './write_fixed_len_pages'),
            csv_file,
            output_file,
            str(block_size),
        ],
        stdout=subprocess.PIPE,
    )
    # parse the result to get the desired time
    return int(result.stdout.split()[-2])


def main():
    """
    Call ./write_fixed_len_pages to write to example_page_file with ten different block sizes,
    each in a range of 10 to 100MB.
    NOTE: piazza says use 500mb???
    https://piazza.com/class/isubcms1kpl3xq?cid=92
    I would assume we can only do this if we make tuples.csv larger.

    """
    if len(sys.argv) < 4:
        print('Usage: experiment_get_histogram <csv_file> <output_file>, <page_size>')
        sys.exit(1)
    csv_file = sys.argv[1]
    output_file = sys.argv[2]
    page_size = sys.argv[3]

    block_sizes = [
        128,           # 128 B
        512,           # 512 B
        1 * 2 ** 10,   # 1 KiB
        4 * 2 ** 10,   # 4 KiB
        8 * 2 ** 10,   # 8 KiB
        64 * 2 ** 10,  # 64 KiB
        128 * 2 ** 10,  # 128 KiB
        512 * 2 ** 10,  # 512 KiB
        1 * 2 ** 20,   # 1 MiB
        2 * 2 ** 20,   # 2 MiB
    ]
    total_size = 100 * 2 ** 20  # 100 MiB

    if os.path.exists('./out'):
        shutil.rmtree('./out')
    os.mkdir('./out')
    csvwriter = csv.DictWriter(
        sys.stdout,
        fieldnames=('block_size', 'milliseconds_elapsed'),
        dialect='unix'
    )
    csvwriter.writeheader()
    for block_size in block_sizes:
        for i in range(50):
            ms_elapsed = write_fixed_len_pages(
                csv_file,
                output_file,
                page_size,
            )
            csvwriter.writerow({
                'block_size': block_size,
                'milliseconds_elapsed': ms_elapsed,
            })

if __name__ == '__main__':
    main()
