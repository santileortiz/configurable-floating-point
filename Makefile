all: main

build:
	mkdir build

FILES=integer_arithmetic.c \
	  floating_point.c

main: build
	gcc -g -o build/main main.c $(FILES)
	build/main

