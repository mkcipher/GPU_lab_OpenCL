################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lib/Core/StrError.c 

CPP_SRCS += \
../lib/Core/Assert.cpp \
../lib/Core/CheckedCast.cpp \
../lib/Core/Error.cpp \
../lib/Core/Exception.cpp \
../lib/Core/Image.cpp \
../lib/Core/Memory.cpp \
../lib/Core/NumericException.cpp \
../lib/Core/Time.cpp \
../lib/Core/TimeSpan.cpp \
../lib/Core/Type.cpp \
../lib/Core/WindowsError.cpp 

OBJS += \
./lib/Core/Assert.o \
./lib/Core/CheckedCast.o \
./lib/Core/Error.o \
./lib/Core/Exception.o \
./lib/Core/Image.o \
./lib/Core/Memory.o \
./lib/Core/NumericException.o \
./lib/Core/StrError.o \
./lib/Core/Time.o \
./lib/Core/TimeSpan.o \
./lib/Core/Type.o \
./lib/Core/WindowsError.o 

C_DEPS += \
./lib/Core/StrError.d 

CPP_DEPS += \
./lib/Core/Assert.d \
./lib/Core/CheckedCast.d \
./lib/Core/Error.d \
./lib/Core/Exception.d \
./lib/Core/Image.d \
./lib/Core/Memory.d \
./lib/Core/NumericException.d \
./lib/Core/Time.d \
./lib/Core/TimeSpan.d \
./lib/Core/Type.d \
./lib/Core/WindowsError.d 


# Each subdirectory must supply rules for building sources it contributes
lib/Core/%.o: ../lib/Core/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -DOMPI_SKIP_MPICXX -DCL_USE_DEPRECATED_OPENCL_1_1_APIS -I"C:\Users\mohit\OneDrive\Documents\UNI STUTTGART MS\SS 19\GPU LAB\Exercise\OpenCLExercise1_Basics\lib" -I/usr/include/mpi -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

lib/Core/%.o: ../lib/Core/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -DOMPI_SKIP_MPICXX -DCL_USE_DEPRECATED_OPENCL_1_1_APIS -I"C:\Users\mohit\OneDrive\Documents\UNI STUTTGART MS\SS 19\GPU LAB\Exercise\OpenCLExercise1_Basics\lib" -I/usr/include/mpi -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


