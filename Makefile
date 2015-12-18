all: main

build:
	mkdir build

FILES=integer_arithmetic.c \
	  floating_point.c
	  #modular_arithmetic.c

main: build
	gcc -g -o build/main main.c $(FILES)
	build/main

