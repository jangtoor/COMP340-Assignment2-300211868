CC = gcc
CFLAGS = -Wall

all: oss user

oss: oss.c
	$(CC) $(CFLAGS) -o oss oss.c

user: user.c
	$(CC) $(CFLAGS) -o user user.c

clean:
	rm -f oss user
