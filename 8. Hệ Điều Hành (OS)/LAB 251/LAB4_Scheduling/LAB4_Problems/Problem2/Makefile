CC = gcc
CFLAGS = -pthread -D_GNU_SOURCE -DINFO -w
LDFLAGS = -pthread -lrt

TARGET = mypool
SOURCES = main.c bktpool.c bktask.c bkwrk.c bkshm.c
HEADERS = bktpool.h bkshm.h
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	@./$(TARGET)

# Build fork version
fork: CFLAGS += -UWORK_THREAD
fork: clean $(TARGET)

# Build thread version (default)
thread: CFLAGS += -DWORK_THREAD
thread: clean $(TARGET)

clean:
	rm -f $(TARGET) $(OBJECTS)

.PHONY: all run fork thread clean

