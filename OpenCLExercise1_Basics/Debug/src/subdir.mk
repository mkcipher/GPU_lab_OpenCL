################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/OpenCLExercise1_Basics.cpp 

OBJS += \
./src/OpenCLExercise1_Basics.o 

CPP_DEPS += \
./src/OpenCLExercise1_Basics.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -DOMPI_SKIP_MPICXX -DCL_USE_DEPRECATED_OPENCL_1_1_APIS -I"C:\Users\mohit\OneDrive\Documents\UNI STUTTGART MS\SS 19\GPU LAB\Exercise\OpenCLExercise1_Basics\lib" -I/usr/include/mpi -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


