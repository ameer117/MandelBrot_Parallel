# MandelBrot_Parallel
Mandelbrot image generator using MPI

Susie program:
Intern Susie Cyclic implements the above computation with P MPI processes. Her strategy is to
make process p compute all of the (valid) rows p + nP for n = 0, 1, 2,... and use MPI gather
operation to collect all the values at the root process to render the fractal.

Joe program:
Intern Joe Block implements the above computation with P MPI processes as well. His strategy is to make process p compute all of the (valid) rows pN, pN + 1, pN + 2,...pN + (N − 1)
where N = height/P and then use MPI gather to collect all of the values at the root process for
rendering the fractal.

