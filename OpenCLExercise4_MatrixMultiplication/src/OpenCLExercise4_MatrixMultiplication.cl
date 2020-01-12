#ifndef __OPENCL_VERSION__
#include <OpenCL/OpenCLKernel.hpp> // Hack to make syntax highlighting in Eclipse work
#endif

__kernel void matrixMulKernel1(/*...*/) {
	//TODO
}

// The preprocessor constant WG_SIZE will contain the size of a work group in X/Y-direction

__attribute__((reqd_work_group_size(WG_SIZE, WG_SIZE, 1)))
__kernel void matrixMulKernel2(/*...*/) {
	//TODO
}
