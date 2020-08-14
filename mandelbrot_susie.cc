/**
 *  \file mandelbrot_serial.cc
 *  \brief HW 2: Mandelbrot set serial code
 */


#include <iostream>
#include <cstdlib>
#include <mpi.h>
#include "render.hh"
#include <stdlib.h>
#include <vector>
using namespace std;

#define WIDTH 1000
#define HEIGHT 1000


int mandelbrot(double x, double y) {
	int maxit = 511;
	double cx = x;
	double cy = y;
	double newx, newy;

	int it = 0;
	for (it = 0; it < maxit && (x*x + y*y) < 4; ++it) {
		newx = x*x - y*y + cx;
		newy = 2*x*y + cy;
		x = newx;
		y = newy;
	}
	return it;
}

int
main(int argc, char* argv[]) {
	double minX = -2.1;
	double maxX = 0.7;
	double minY = -1.25;
	double maxY = 1.25;

	int height, width;
	if (argc == 3) {
		height = atoi (argv[1]);
		width = atoi (argv[2]);
		assert (height > 0 && width > 0);
	} else {
		fprintf (stderr, "usage: %s <height> <width>\n", argv[0]);
		fprintf (stderr, "where <height> and <width> are the dimensions of the image.\n");
		return -1;
	}
	int size, rank;
	int root = 0;

	double it = (maxY - minY)/height;
	double jt = (maxX - minX)/width;
	double x, y;
	double t1, t2;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//	printf("%d\n", rank);
//	printf("%d\n", size);
	int arraysize = (height*width)/size;
	vector<float> sendarray(arraysize);
	y = minY + rank*it; //starting y
	//printf("rank is %d and starting y is %f\n", rank, y);

	//x = minX;

	for (int i = 0; i < (height/size); i++){ //iterate every p columns and height
		x = minX;
		for (int j = 0; j < width; j++){ //rows and width
			//printf("%d", width*i + j);		
			sendarray[width*i + j] = mandelbrot(x, y);
			x += jt;
		}
		y += it*size;
	}
	vector<float> receivearray;
	if (rank == root){
		t1 = MPI_Wtime();
		receivearray.resize(width*height);
		MPI_Gather(&sendarray.front(), (height*width)/size, MPI_INT, &receivearray.front(), (height*width)/size, MPI_INT, root, MPI_COMM_WORLD);
	
	              t2 = MPI_Wtime();
                printf("(MS) Time taken: %f  with %d processes \n", (t2-t1),size);
	}
	MPI_Finalize();

	gil::rgb8_image_t img(height, width);
	auto img_view = gil::view(img);

//	for (int i = 0; i < height; i++){
//		printf("\n");
//		for(int j = 0; j < width; j++){
//			printf("%f, ", receivearray[width*i+j]/512.0);
//		}
//	}

	vector<float> arr(width*height);
	int idx = 0;
	for (int i = 0; i < size; i++)
		for (int j = i; j < height; j+=size)
			for(int k = 0; k < width; k++){
				arr[idx] = receivearray[width*j+k];
				//printf("%d: %f\n",idx, arr[idx]);
				idx++;
			}



	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; ++j) {
			img_view(j, i) = render(arr[width*i+j]/512.0);
		} 
	} 
	gil::png_write_view("mandelbrot.png", const_view(img));




}

/* eof */
