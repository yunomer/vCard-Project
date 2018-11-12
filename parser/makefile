CC=gcc
CFLAGS= -Wall -pedantic -std=c11 -g

SRCDIR = ./src/
BINDIR = ./bin/
ROOT = ./../

#A3 Makefile Code
sharedLib: LinkedListSER.o VCardParserSER.o VCardHelperSER.o
	$(CC) -Iinclude -shared -o $(ROOT)sharedLib.so $(BINDIR)VCardParserSER.o $(BINDIR)VCardHelperSER.o $(BINDIR)LinkedListSER.o

LinkedListSER.o: src/LinkedListAPI.c
	$(CC) $(CFLAGS) -Iinclude -c -fpic $(SRCDIR)LinkedListAPI.c -o $(BINDIR)LinkedListSER.o

VCardParserSER.o: src/VCardParser.c
	$(CC) $(CFLAGS) -Iinclude -c -fpic $(SRCDIR)VCardParser.c -o $(BINDIR)VCardParserSER.o

VCardHelperSER.o: src/VCardHelper.c
	$(CC) $(CFLAGS) -Iinclude -c -fpic $(SRCDIR)VCardHelper.c -o $(BINDIR)VCardHelperSER.o


# A1 and A2
all: list parser

LinkedListAPI.o: $(SRCDIR)LinkedListAPI.c
	$(CC) -Iinclude -c $(SRCDIR)LinkedListAPI.c -o $(BINDIR)LinkedListAPI.o $(CFLAGS)

VCardParser.o: $(SRCDIR)VCardParser.c
	$(CC) -Iinclude -c $(SRCDIR)VCardParser.c -o $(BINDIR)VCardParser.o $(CFLAGS)

VCardHelper.o: $(SRCDIR)VCardHelper.c
	$(CC) -Iinclude -c $(SRCDIR)VCardHelper.c -o $(BINDIR)VCardHelper.o $(CFLAGS)

list: LinkedListAPI.o
	ar cr $(BINDIR)libllist.a $(BINDIR)LinkedListAPI.o

parser: VCardParser.o LinkedListAPI.o VCardHelper.o
	ar cr $(BINDIR)libcparse.a $(BINDIR)VCardParser.o $(BINDIR)LinkedListAPI.o $(BINDIR)VCardHelper.o

#run:
#	$(CC) $(CFLAGS) -Iinclude $(SRCDIR)testMain.c $(SRCDIR)LinkedListAPI.c $(SRCDIR)VCardHelper.c $(SRCDIR)VCardParser.c -o runMe

clean:
	rm runMe -f bin/*.o *.so bin/*.a bin/parser
