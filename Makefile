CC = gcc
CFLAGS = -Iinclude -Isrc -O2 -Wall
LDFLAGS = -Llib -lraylib -ltcc -lws2_32 -lgdi32 -lwinmm -lopengl32
TARGET = LeafBlock.exe
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)
