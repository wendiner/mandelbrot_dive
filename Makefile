mandelbrot:	mandelbrot.c
	gcc mandelbrot.c -o mandelbrot $$(pkg-config allegro-5 allegro_image-5 --libs --cflags) -l:libm.so
