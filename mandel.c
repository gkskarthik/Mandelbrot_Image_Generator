/*
Karthik Kumarasubramanian
1001549999
*/

#define _GNU_SOURCE

#include "bitmap.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <pthread.h>

#define MAX_NUM_THREADS 50	//The maximum number of threads to be created 


/*Structure to pass to the compute_image function as a Pthread argument */

struct thr_arg{
	double xmin;
	double xmax;
	double ymin; 
	double ymax; 
	int max;
	struct bitmap *bm;
};

//Convert a iteration number to an RGBA color.
int iteration_to_color( int i, int max );

// Compute the iterations at that point.
int iterations_at_point( double x, double y, int max );

//Function to compute the image for the Mandelbrot Set
void *compute_image(void *ptr);

//Function to print help on passing -h argument
void show_help()
{
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates. (default=4)\n");
	printf("-W <pixels> Width of the image in pixels. (default=500)\n");
	printf("-H <pixels> Height of the image in pixels. (default=500)\n");
	printf("-o <file>   Set output file. (default=mandel.bmp)\n");
	printf("-n <threads>   Set number of threads. (default=1)\n");
	printf("-h          Show this help text.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}

int main( int argc, char *argv[] )
{
	char c;

	// These are the default configuration values used.
	// if no command line arguments are given.

	//Creating an object for the structure.
	struct thr_arg thi;
	
	//Initializing variables with default value to be passed on to the structure for computation.
	const char *outfile = "mandel.bmp";
	double xcenter = 0;
	double ycenter = 0;
	double scale = 4;
	int    image_width = 500;
	int    image_height = 500;
	int    max = 1000;

	//Initializing variables creating threads.
	int i = 0;

	//Initializing variable to check Pthread for NULL values.
	int rc;

	//Number of threads to create to compute on image (Default number of threads is 1).
	int n = 1;

	//Initializing Pthread variable as an array for each thread to work on set of pixels.
	pthread_t my_thread[MAX_NUM_THREADS];

	// For each command line argument given,
	// override the appropriate configuration value.
	while((c = getopt(argc,argv,"x:y:s:W:H:m:o:n:h"))!=-1) {
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
			case 'o':
				outfile = optarg;
				break;
			case 'n':
				n = atoi(optarg);
				break;
			case 'h':
				show_help();
				exit(1);
				break;
		}
	}

	// Display the configuration of the image.
	printf("mandel: x=%lf y=%lf scale=%lf max=%d outfile=%s\n",xcenter,ycenter,scale,max,outfile);

	// Fill it with a dark blue, for debugging
	//bitmap_reset(bm,MAKE_RGBA(0,0,255,0));
	
	//Creating Pthreads and passing compute_image and structure as arguments.
	for( i = 0; i < n; i++){

		//Changing image_width according to thread  
		
		int width = i*(image_width/n);
		//printf("%d\n",width);		// For debugging

		//Changing image_height according to thread

		int height = i*(image_height/n);
		//printf("%d\n",height); 	// For debugging
		
		//Create a bitmap of the appropriate size.
		thi.bm = bitmap_create(width,height);
	
		// Setting the value for the structure variables. 	
		thi.xmin = (xcenter-scale);
		thi.xmax = (xcenter+scale);
		thi.ymin = (ycenter-scale);
		thi.ymax = (ycenter+scale);
		thi.max = max;
		
		rc = pthread_create(&my_thread[i], NULL,compute_image, &thi);

		//Checking for NULL value.
		if(rc){
			printf("ERROR in creating thread\n");
			exit(-1);
		}
	}	
	
	//Joining the threads to set a complete image.
	for( i = 0; i < n; i++){
		pthread_join(my_thread[i], NULL);

	}

	// Save the image in the stated file.
	if(!bitmap_save(thi.bm,outfile)) {
		fprintf(stderr,"mandel: couldn't write to %s: %s\n",outfile,strerror(errno));
		return 1;
	}
	
	return 0;
}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/

void *compute_image(void *ptr)
{
	int i,j;
	
	//Passing the object to the structure object inside the function 
	struct thr_arg *thi = ptr;

	int width = bitmap_width(thi -> bm);

	int height = bitmap_height(thi -> bm);	

	// For every pixel in the image...

	for(j = 0; j < height; j++) {

		for(i = 0; i < width; i++) {
			
			// Determine the point in x,y space for that pixel.
			double x = (thi -> xmin) + i*((thi -> xmax)-(thi -> xmin))/width;

			double y = (thi -> ymin) + j*((thi -> ymax)-(thi -> ymin))/height;

			// Compute the iterations at that point.
			int iters = iterations_at_point(x,y,thi -> max);

			// Set the pixel in the bitmap.
			bitmap_set(thi -> bm,i,j,iters);
		}
	}
	return 0;
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

