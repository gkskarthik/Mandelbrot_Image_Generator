# Mandelbrot_Image_Generator

all: mandelseries mandel

mandel: mandel.o bitmap.o
	gcc mandel.o bitmap.o -o mandel -lpthread

mandelseries: mandelseries.o bitmap.o
	gcc mandelseries.o bitmap.o -o mandelseries -lpthread

mandel.o: mandel.c
	gcc -Wall -g -c mandel.c -o mandel.o

mandelseries.o: mandelseries.c
	gcc -Wall -g -c mandelseries.c -o mandelseries.o

bitmap.o: bitmap.c
	gcc -Wall -g -c bitmap.c -o bitmap.o

clean:
	rm -f mandelseries.o mandel.o bitmap.o mandelseries mandel
