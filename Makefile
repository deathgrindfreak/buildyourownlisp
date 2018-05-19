CC = cc
CFLAGS = -std=c99 -Wall
LFLAGS = -ledit -lm

SRC = \
      parsing.c \
      mpc.c

OUTPUT = parsing

build:
	$(CC) $(CFLAGS) $(SRC) $(LFLAGS) -o $(OUTPUT)

clean:
	rm -f parsing *.o
