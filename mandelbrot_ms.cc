
/**
 *  \file mandelbrot_ms.cc
 *
 *  \brief Implement your parallel mandelbrot set in this file.
 */

#include <iostream>
#include <cstdlib>
#include "render.hh"
#include <mpi.h>
using namespace std;
#define WIDTH 1000
#define HEIGHT 1000
#include <vector>
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


int main (int argc, char* argv[]){
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

	double it = (maxY - minY)/height;
	double jt = (maxX - minX)/width;
	double x, y;

	int rank, size;
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Status status;
	int rownum;

	if(rank == 0){ //master

		double t1,t2;
		t1 = MPI_Wtime();

		rownum = 0;
		int s = size-1;    
		int row[width];
		int imagearr[height][width]; 
		while( s > 0 ){
			MPI_Send( &rownum, 1, MPI_INT, s, 1, MPI_COMM_WORLD); //send row to each process
			rownum++;
			s++;
		}    
		while( rownum < height ){
			MPI_Recv( row, width, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			MPI_Send( &rownum, 1, MPI_INT, status.MPI_SOURCE, 1, MPI_COMM_WORLD);
			for (int i= 0; i < width; i++){
				imagearr[status.MPI_TAG][i] = row[i];    
			}
			rownum++;
		}

		s = size - 1;
		//printf("testing);
		//printf("s has the value of %d", s);
		//printf("\n");

		int t = -1;

		while( s != 0 ){
			MPI_Recv(row, width, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			MPI_Send( &t, 1 ,MPI_INT, status.MPI_SOURCE, 1, MPI_COMM_WORLD);
			for(int i = 0; i < width ; i++){
				imagearr[status.MPI_TAG][i] = row[i];
			}
			s--;
			//printf("%d",s);
			//printf("testing\n");
		}

		t2 = MPI_Wtime();
		printf("Time taken: %f  with %d processes \n", (t2-t1),size);



		gil::rgb8_image_t img(height, width);
		auto img_view = gil::view(img);
		for( int i = 0; i < height; i++){ //render image
			for( int j = 0; j < width; j++){
				img_view(j, i) = render( imagearr[i][j]/512.0 );
			}
		}
		gil::png_write_view("mandelbrot_ms.png", const_view(img));


	}	

	else { //slave
		int sender[width];
		double x, y;
		while(true){	
			//receive row number to compute
			MPI_Recv(&rownum, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			if(rownum < 0) 
				break;
          
			y = minY + it*rownum;
			x = minX;
			for (int i = 0; i < width; ++i) {
				sender[i] = mandelbrot(x, y);
				x += jt;
			}
			MPI_Send(sender, width, MPI_INT, 0, rownum, MPI_COMM_WORLD); //send back row
		}  
	}

}
