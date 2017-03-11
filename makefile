SRC = alg.c alg.h io.c io.h aux.c aux.h
CFLAGS = -Wall


all: my_route_lookup

my_route_lookup: $(SRC)
	gcc $(CFLAGS) $(SRC) -o my_route_lookup -lm

.PHONY: clean
clean:
	rm -f my_route_lookup
