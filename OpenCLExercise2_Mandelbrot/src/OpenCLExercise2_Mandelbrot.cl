#ifndef __OPENCL_VERSION__
#include <OpenCL/OpenCLKernel.hpp> // Hack to make syntax highlighting in Eclipse work
#endif

//TODO
/* Global identifier makes sure that you are writing to memory shared by all */
__kernel void mandelbrotKernel(const float xmin, const float xmax, const float ymin,
		 const float ymax, const uint niter, __global uint * h_output) {

	float xc = xmin + (xmax - xmin) / (get_global_size(0) - 1) * get_global_id(0); //xc=real(c)

	float yc = ymin + (ymax - ymin) / (get_global_size(1) - 1) * get_global_id(1); //yc=imag(c)

	float x = 0.0; //x=real(z_k

	float y = 0.0; //y=imag(z_k)

	for (size_t k = 0; k < niter; k = k + 1) { // iteration loop
		float tempx = x * x - y * y + xc; // real of z_{n+1}=( z_n)^2+c
		float tempy = 2 * x * y + yc; // imaginary part of z_{n+1}
		x = tempx;
		y = tempy;
		float r2 = x * x + y * y; //r2=|z_{n +1}|^2
		if ((r2 > 4) || k == niter - 1) { // divergence condition
			h_output[get_global_id(0) + get_global_id(1) * get_global_size(0)] = k;
			break;
		}
	}
}
