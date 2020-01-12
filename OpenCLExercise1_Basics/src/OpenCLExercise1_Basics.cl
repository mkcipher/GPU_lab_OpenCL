#ifndef __OPENCL_VERSION__
#include <OpenCL/OpenCLKernel.hpp> // Hack to make syntax highlighting in Eclipse work
#endif

__kernel void kernel1 (__global const float* d_input, __global float* d_output) {
	//TODO
	size_t i = get_global_id(0);
	d_output[i]=native_cos(d_input[i]);
}
