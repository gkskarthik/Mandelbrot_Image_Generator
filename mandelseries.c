#define _GNU_SOURCE

#include "bitmap.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_NUM_PROCESS 50 	//Maximum number of process to be run 

int iteration_to_color( int i, int max );
int iterations_at_point( double x, double y, int max );
void compute_image( struct bitmap *bm, double xmin, double xmax, double ymin, double ymax, int max );

void show_help()
{
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0.2863)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0.0142)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates. (default=2)\n");
	printf("-W <pixels> Width of the image in pixels. (default=500)\n");
	printf("-H <pixels> Height of the image in pixels. (default=500)\n");
	printf("-o <file>   Set output file. (default=mandel.bmp)\n");
	printf("-h          Show this help text.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}


int main( int argc, char *argv[] )
{
	// These are the default configuration values used
	// if no command line arguments are given.

	char   c;
	char   outfile[MAX_NUM_PROCESS];
	double xcenter = 0.2863;
	double ycenter = 0.0142;
	double scale;
	int    image_width = 500;
	int    image_height = 500;
	int    max = 1000;
	
	int    i = 0; 
	int    j = 0;	
	int    n = 1;
	int    status;

	double init_value = 2.260129;

	pid_t pid[MAX_NUM_PROCESS];
	
	if(!argc){
	n = atoi(argv[1]);
	}
	
	// For each command line argument given,
	// override the appropriate configuration value.

	while((c = getopt(argc,argv,"x:y:s:W:H:m:o:h"))!=-1) {
		switch(c) {
			case 'x':
				xcenter = atof(optarg);
				break;
			case 'y':
				ycenter = atof(optarg);
				break;
			case 's':
				scale = atof(optarg);
				break;
			case 'W':
				image_width = atoi(optarg);
				break;
			case 'H':
				image_height = atoi(optarg);
				break;
			case 'm':
				max = atoi(optarg);
				break;
			case 'h':
				show_help();
				exit(1);
				break;
		}
	}

	for ( i = 0; i	< MAX_NUM_PROCESS; i++){

		struct bitmap *bm;

		// Reducing the value of scale to desired value 
		double temp = (init_value) * 0.884905;
		scale = temp;
		init_value = temp;

		// Creating files for saving the image.
		sprintf(outfile, "mandel%d.bmp", i+1);
		
		// Save the image in the stated file.
		if(!bitmap_save(bm,outfile)){
			//fprintf(stderr,"mandel: couldn't write to %s: %s\n",outfile,strerro(errno));
			return 1;
		}

		// Display the configuration of the image.
		printf("mandel: x=%lf y=%lf scale=%lf max=%d outfile=%s\n",xcenter,ycenter,scale,max,outfile);

		for ( j = 0; j < n; j+=n){
						
			//Creating a new process
			pid[n] = fork();
			
			//Checking for error while forking
			if ( pid[n] == -1){
				perror("fork");
				abort();
			}

			//Child Process
			else if (pid[n] == 0){

				// Create a bitmap of the appropriate size.
				bm = bitmap_create(image_width,image_height);

				// Compute the Mandelbrot image
				compute_image(bm,xcenter-scale,xcenter+scale,ycenter-scale,ycenter+scale,max);
			}
		}
		
		for ( j = 0; j < n; j+=n){
	
			//Wait for Child Process to finish 
			if(pid[n] > 0){
				waitpid(pid[n], &status, 0);
				return 1;
  			}
		}		
	}	
	exit(0);

	return 0;
}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/

void compute_image( struct bitmap *bm, double xmin, double xmax, double ymin, double ymax, int max )
{
	int i,j;

	int width = bitmap_width(bm);
	int height = bitmap_height(bm);

	// For every pixel in the image...

	for(j=0;j<height;j++) {

		for(i=0;i<width;i++) {

			// Determine the point in x,y space for that pixel.
			double x = xmin + i*(xmax-xmin)/width;
			double y = ymin + j*(ymax-ymin)/height;

			// Compute the iterations at that point.
			int iters = iterations_at_point(x,y,max);

			// Set the pixel in the bitmap.
			bitmap_set(bm,i,j,iters);
		}
	}
}

/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point( double x, double y, int max )
{
	double x0 = x;
	double y0 = y;

	int iter = 0;

	while( (x*x + y*y <= 4) && iter < max ) {

		double xt = x*x - y*y + x0;
		double yt = 2*x*y + y0;

		x = xt;
		y = yt;

		iter++;
	}

	return iteration_to_color(iter,max);
}

/*
Convert a iteration number to an RGBA color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/

int iteration_to_color( int i, int max )
{
	int gray = 255*i/max;
	return MAKE_RGBA(gray,gray,gray,0);
}
