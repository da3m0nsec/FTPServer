IDIR =../include
CC=g++
CFLAGS=-I$(IDIR) -std=c++17 -lstdc++fs -O3 -lpthread

ODIR=../src
LDIR =../lib

LIBS=-lm

_OBJ = ClientConnection.o FTPServer.o ftp_server.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all: ftp_server

$(ODIR)/ClientConnection.o: $(ODIR)/ClientConnection.cpp $(IDIR)/ClientConnection.h
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/FTPServer.o: $(ODIR)/FTPServer.cpp $(IDIR)/FTPServer.h $(IDIR)/ClientConnection.h
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/ftp_server.o: $(ODIR)/ftp_server.cpp $(IDIR)/FTPServer.h
	$(CC) -c -o $@ $< $(CFLAGS)

ftp_server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ $(INCDIR)/*~
