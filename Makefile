TARGET = pj

CC = gcc
CFLAGS = -I$(INCDIR)

SRCDIR = src
INCDIR = headers

SRCS = $(wildcard $(SRCDIR)/*.c) main.c

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(SRCDIR)/*.o main.o $(TARGET)

.PHONY: all clean