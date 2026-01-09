CC = gcc
CFLAGS = -Wall -Wextra -std=c17 `sdl2-config --cflags`
LDFLAGS = `sdl2-config --libs` -lm

SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)
TARGET = app

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)
