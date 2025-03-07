CC =gcc
FLAGS=-Wall -Wextra -pedantic -g --std=c17
MAIN=json_parser


.PHONY: all clean recompile

all: $(MAIN)

$(MAIN): main.o uds.o
	gcc $^ -o $(MAIN) $(FLAGS)


main.o: main.c
	gcc -c $< -o $@ $(FLAGS)

uds.o: uds.c
	gcc -c $< -o $@ $(FLAGS)

clean:
	@echo "Removing files"
	rm -rf $(MAIN) *.o *.gch
	@echo "Done!"