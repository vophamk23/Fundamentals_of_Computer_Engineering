CC = gcc
CFLAGS = -pthread -Wall -Wextra -D_GNU_SOURCE
LDFLAGS = -pthread

# Fork-Join targets
FJ_SOURCES = bkfj.c test_forkjoin.c
FJ_OBJECTS = $(FJ_SOURCES:.c=.o)
FJ_TARGET = test_fj

all: $(FJ_TARGET)

$(FJ_TARGET): $(FJ_OBJECTS)
	$(CC) $(LDFLAGS) -o $(FJ_TARGET) $(FJ_OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(FJ_TARGET)
	./$(FJ_TARGET)

clean:
	rm -f $(FJ_TARGET) $(FJ_OBJECTS)

.PHONY: all run clean