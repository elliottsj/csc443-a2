CC = g++

all: csv2heapfile scan insert update delete select csv2colstore select2 select3

library.o: library.cc library.h
	$(CC) -o $@ -c $<

csv2heapfile: csv2heapfile.cc library.o
	$(CC) -o $@ $< library.o

scan: scan.cc library.o
	$(CC) -o $@ $< library.o

insert: insert.cc library.o
	$(CC) -o $@ $< library.o

update: update.cc library.o
	$(CC) -o $@ $< library.o

delete: delete.cc library.o
	$(CC) -o $@ $< library.o

select: select.cc library.o
	$(CC) -o $@ $< library.o

csv2colstore: csv2colstore.cc library.o
	$(CC) -o $@ $< library.o

select2: select2.cc library.o
	$(CC) -o $@ $< library.o

select3: select3.cc library.o
	$(CC) -o $@ $< library.o

sync-ellio128:
	fswatch -e "\.git" . | while read line; do rsync -avz --delete . ellio128@teach.cs.toronto.edu:csc469/csc469-a1; done
