CC=g++
CXXFLAGS=-std=c++11 -Wall -g

all: experiment_fixed_len_sizeof write_fixed_len_pages csv2heapfile scan insert update delete select csv2colstore select2 select3 read_fixed_len_pages

library.o: library.cc library.h
	$(CC) $(CXXFLAGS) -o $@ -c $<

experiment_fixed_len_sizeof: experiment_fixed_len_sizeof.cc library.o
	$(CC) $(CXXFLAGS) -o $@ $< library.o

write_fixed_len_pages: write_fixed_len_pages.cc library.o
	$(CC) $(CXXFLAGS) -o $@ $< library.o

read_fixed_len_pages: read_fixed_len_page.cc library.o
	$(CC) $(CXXFLAGS) -o $@ $< library.o

csv2heapfile: csv2heapfile.cc library.o
	$(CC) $(CXXFLAGS) -o $@ $< library.o

scan: scan.cc library.o
	$(CC) $(CXXFLAGS) -o $@ $< library.o

insert: insert.cc library.o
	$(CC) $(CXXFLAGS) -o $@ $< library.o

update: update.cc library.o
	$(CC) $(CXXFLAGS) -o $@ $< library.o

delete: delete.cc library.o
	$(CC) $(CXXFLAGS) -o $@ $< library.o

select: select.cc library.o
	$(CC) $(CXXFLAGS) -o $@ $< library.o

csv2colstore: csv2colstore.cc library.o
	$(CC) $(CXXFLAGS) -o $@ $< library.o

select2: select2.cc library.o
	$(CC) $(CXXFLAGS) -o $@ $< library.o

select3: select3.cc library.o
	$(CC) $(CXXFLAGS) -o $@ $< library.o

sync-ellio128:
	fswatch -e "\.git" -e "venv" . | while read line; do rsync -avz --delete . ellio128@teach.cs.toronto.edu:csc443/csc443-a2; done
