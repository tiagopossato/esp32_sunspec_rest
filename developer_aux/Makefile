CC=gcc
CFLAGS=-I.
DEPS = sunspec_models.h
OBJ = main.o sunspec_models.o 

.PHONY: clean

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -rf main.o sunspec_models.o main
