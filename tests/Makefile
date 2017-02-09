TEST_CLIENTS = unit-test

LIBS = ../sandscript-map-extension.so -lpthread

CFLAGS = -g -O0 -D_GNU_SOURCE -I ..

all: $(TEST_CLIENTS)

clean:
	@rm -f $(TEST_CLIENTS)

unit-test: unit-test.c $(TEST_DEPS)
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)
