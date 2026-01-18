# ===== COMPILER CONFIGURATION =====
CC = gcc
CFLAGS = -pthread -D_GNU_SOURCE -DINFO -w
LDFLAGS = -pthread

# ===== TARGET CONFIGURATION =====
TARGET = mypool

# ===== SOURCE FILES =====
SOURCES = main.c bktpool.c bktask.c bkwrk.c
HEADERS = bktpool.h
OBJECTS = $(SOURCES:.c=.o)

# ===== DEFAULT TARGET =====
all: $(TARGET)

# ===== BUILD =====
$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# ===== RUN =====
run: $(TARGET)
	@ ./$(TARGET)

# ===== DEBUG BUILD =====
debug: CFLAGS += -g -DDEBUG -O0
debug: clean $(TARGET)

# ===== CLEAN =====
clean:
	rm -f $(TARGET) $(OBJECTS) *.o

# ===== REBUILD =====
rebuild: clean all

# ===== PHONY TARGETS =====
.PHONY: all run debug clean rebuild