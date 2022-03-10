BIN_FILES  = broker publisher suscriptor.class serverAlmacenamiento

CC = gcc
JCC = javac

CCFLAGS =	-Wall  -g

LDFLAGS = -L$(INSTALL_PATH)/lib/
LDLIBS = -lpthread

JFLAGS = -g


all: CFLAGS=$(CCFLAGS)
all: $(BIN_FILES)
.PHONY : all

broker: broker.o read_line.o database.o almacenamientoTemas_clnt.o almacenamientoTemas_xdr.c
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

publisher: publisher.o read_line.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

suscriptor.class: suscriptor.java
	$(JCC) $(JFLAGS) suscriptor.java

serverAlmacenamiento:
	$(CC) -c serverAlmacenamiento.c
	$(CC) -o serverAlmacenamiento serverAlmacenamiento.o almacenamientoTemas_svc.o almacenamientoTemas_xdr.c

%.o: %.c
	$(CC) $(CCFLAGS) $(CFLAGS) -c $<

clean:
	rm -f $(BIN_FILES) *.o *.class *~
	rm -f almacenamientoTemas.h almacenamientoTemas_client.c almacenamientoTemas_clnt.c almacenamientoTemas_svc.c almacenamientoTemas_xdr.c almacenamientoTemas_client almacenamientoTemas_server almacenamientoTemas_server.c Makefile.almacenamientoTemas

.SUFFIXES:
.PHONY : clean
