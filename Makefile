CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
TARGET = editor

all: $(TARGET)

$(TARGET): editor.c
	$(CC) $(CFLAGS) -o $(TARGET) editor.c

clean:
	rm -f $(TARGET) $(TARGET).exe *.o
