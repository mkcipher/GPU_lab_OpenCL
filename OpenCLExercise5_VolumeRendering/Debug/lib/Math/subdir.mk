################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../lib/Math/Abs.cpp \
../lib/Math/Array.cpp \
../lib/Math/DiagMatrix3.cpp \
../lib/Math/Float.cpp \
../lib/Math/Vector2.cpp \
../lib/Math/Vector3.cpp 

OBJS += \
./lib/Math/Abs.o \
./lib/Math/Array.o \
./lib/Math/DiagMatrix3.o \
./lib/Math/Float.o \
./lib/Math/Vector2.o \
./lib/Math/Vector3.o 

CPP_DEPS += \
./lib/Math/Abs.d \
./lib/Math/Array.d \
./lib/Math/DiagMatrix3.d \
./lib/Math/Float.d \
./lib/Math/Vector2.d \
./lib/Math/Vector3.d 


# Each subdirectory must supply rules for building sources it contributes
lib/Math/%.o: ../lib/Math/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DOMPI_SKIP_MPICXX -DCL_USE_DEPRECATED_OPENCL_1_1_APIS -I"/home/sunkg/workspace/OpenCLExercise5_VolumeRendering/lib" -I/usr/include/hdf5/serial -I/usr/include/mpi -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


