//////////////////////////////////////////////////////////////////////////////
// OpenCL exercise 6: Prefix sum (Scan)
//////////////////////////////////////////////////////////////////////////////

// includes
#include <stdio.h>

#include <Core/Assert.hpp>
#include <Core/Time.hpp>
#include <Core/Image.hpp>
#include <OpenCL/cl-patched.hpp>
#include <OpenCL/Program.hpp>
#include <OpenCL/Event.hpp>
#include <OpenCL/Device.hpp>

#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <sstream>

#include <boost/lexical_cast.hpp>

//////////////////////////////////////////////////////////////////////////////
// CPU implementation
//////////////////////////////////////////////////////////////////////////////
void prefixSumHost(const std::vector<cl_int>& h_input, std::vector<cl_int>& h_output) {
	if (h_input.size () == 0)
		return;
	cl_int sum = h_input[0];
	h_output[0] = sum;
	for (std::size_t i = 1; i < h_input.size (); i++) {
		sum += h_input[i];
		h_output[i] = sum;
	}
}

//////////////////////////////////////////////////////////////////////////////
// Main function
//////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {
	// Create a context
	//cl::Context context(CL_DEVICE_TYPE_GPU);
        std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	if (platforms.size() == 0) {
		std::cerr << "No platforms found" << std::endl;
		return 1;
	}
	int platformId = 0;
	for (size_t i = 0; i < platforms.size(); i++) {
		if (platforms[i].getInfo<CL_PLATFORM_NAME>() == "AMD Accelerated Parallel Processing") {
			platformId = i;
			break;
		}
	}
	cl_context_properties prop[4] = { CL_CONTEXT_PLATFORM, (cl_context_properties) platforms[platformId] (), 0, 0 };
	std::cout << "Using platform '" << platforms[platformId].getInfo<CL_PLATFORM_NAME>() << "' from '" << platforms[platformId].getInfo<CL_PLATFORM_VENDOR>() << "'" << std::endl;
	cl::Context context(CL_DEVICE_TYPE_GPU, prop);

	// Get a device of the context
	int deviceNr = argc < 2 ? 1 : atoi(argv[1]);
	std::cout << "Using device " << deviceNr << " / " << context.getInfo<CL_CONTEXT_DEVICES>().size() << std::endl;
	ASSERT (deviceNr > 0);
	ASSERT ((size_t) deviceNr <= context.getInfo<CL_CONTEXT_DEVICES>().size());
	cl::Device device = context.getInfo<CL_CONTEXT_DEVICES>()[deviceNr - 1];
	std::vector<cl::Device> devices;
	devices.push_back(device);
	OpenCL::printDeviceInfo(std::cout, device);

	// Create a command queue
	cl::CommandQueue queue(context, device, CL_QUEUE_PROFILING_ENABLE);

	// Declare some values
	std::size_t wgSize = 256; // Number of work items per work group
	std::size_t count = wgSize * wgSize * wgSize; // Number of values

	std::size_t size = count * sizeof (cl_int);

	// Load the source code
	cl::Program program = OpenCL::loadProgramSource(context, "src/OpenCLExercise6_PrefixSum.cl");
	// Compile the source code. This is similar to program.build(devices) but will print more detailed error messages
	// This will pass the value of wgSize as a preprocessor constant "WG_SIZE" to the OpenCL C compiler
	OpenCL::buildProgram(program, devices, "-DWG_SIZE=" + boost::lexical_cast<std::string>(wgSize));

	// Allocate space for output data from CPU and GPU on the host
	std::vector<cl_int> h_input (count);
	std::vector<cl_int> h_outputCpu (count);
	std::vector<cl_int> h_temp1 (wgSize * wgSize);
	std::vector<cl_int> h_temp2 (wgSize);
	std::vector<cl_int> h_outputGpu (count);

	// Allocate space for input and output data on the device
	//TODO

	// Initialize memory to 0xff (useful for debugging because otherwise GPU memory will contain information from last execution)
	memset(h_input.data(), 255, size);
	memset(h_temp1.data(), 255, wgSize * wgSize * sizeof (cl_int));
	memset(h_temp2.data(), 255, wgSize * sizeof (cl_int));
	memset(h_outputCpu.data(), 255, size);
	memset(h_outputGpu.data(), 255, size);
	//TODO: GPU

	//////// Generate input data ////////////////////////////////
	// Use random input data
	for (std::size_t i = 0; i < count; i++)
		h_input[i] = rand() % 100 - 40;
	// Or: Use consecutive integer numbers as data
	/*
	for (std::size_t i = 0; i < count; i++)
		h_input[i] = i;
	// */

	// Do calculation on the host side
	prefixSumHost(h_input, h_outputCpu);

	// Create kernels
	//TODO

	// Copy input data to device
	//TODO

	// Call the kernels
	//TODO

	// Copy output data back to host
	//TODO

	// Print performance data
	//TODO

	// Check whether results are correct
	std::size_t errorCount = 0;
	for (size_t i = 0; i < count; i = i + 1) {
		if (h_outputCpu[i] != h_outputGpu[i]) {
			if (errorCount < 15)
				std::cout << "Result at " << i << " is incorrect: GPU value is " << h_outputGpu[i] << ", CPU value is " << h_outputCpu[i] << std::endl;
			else if (errorCount == 15)
				std::cout << "..." << std::endl;
			errorCount++;
		}
	}
	if (errorCount != 0) {
		std::cout << "Found " << errorCount << " incorrect results" << std::endl;
		return 1;
	}

	std::cout << "Success" << std::endl;

	return 0;
}
