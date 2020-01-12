################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../lib/CT/CTFloat.cpp \
../lib/CT/DataFiles.cpp 

OBJS += \
./lib/CT/CTFloat.o \
./lib/CT/DataFiles.o 

CPP_DEPS += \
./lib/CT/CTFloat.d \
./lib/CT/DataFiles.d 


# Each subdirectory must supply rules for building sources it contributes
lib/CT/%.o: ../lib/CT/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DOMPI_SKIP_MPICXX -DCL_USE_DEPRECATED_OPENCL_1_1_APIS -I"/home/sunkg/workspace/OpenCLExercise5_VolumeRendering/lib" -I/usr/include/hdf5/serial -I/usr/include/mpi -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


