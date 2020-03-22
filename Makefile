src = $(wildcard src/*.c)
obj = $(src:.c=.o)

LDFLAGS = 

a.out: $(obj)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) a.out
