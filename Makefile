src = $(wildcard src/*.c)
obj = $(src:.c=.o)

LDFLAGS =
CFLAGS = -Wall -Wextra -Wpedantic

.PHONY: all debug clean

all: a.out
all: CFLAGS += -s
all: LDFLAGS += -s

debug: CFLAGS += -DDEBUG -g
debug: a.out

clean:
	rm -f $(obj) a.out

a.out: $(obj)
	$(CC) -o $@ $^ $(LDFLAGS)
