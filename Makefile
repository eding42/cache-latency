CC      = gcc
CFLAGS  = -O2 -Wall
TARGET  = hw1
SRCS    = main.c cache_model.c
LDLIBS  = -lm

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $^ $(LDLIBS) -o $@

run: $(TARGET)
	./$(TARGET) $(ARGS)

clean:
	rm -f $(TARGET)
