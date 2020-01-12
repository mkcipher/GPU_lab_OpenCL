################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../lib/HDF5/Array.cpp \
../lib/HDF5/AtomicType.cpp \
../lib/HDF5/AtomicTypes.cpp \
../lib/HDF5/Attribute.cpp \
../lib/HDF5/BaseTypes.cpp \
../lib/HDF5/ComplexConversion.cpp \
../lib/HDF5/CompoundType.cpp \
../lib/HDF5/DataSet.cpp \
../lib/HDF5/DataSpace.cpp \
../lib/HDF5/DataType.cpp \
../lib/HDF5/DataTypes.cpp \
../lib/HDF5/DelayedArray.cpp \
../lib/HDF5/Exception.cpp \
../lib/HDF5/File.cpp \
../lib/HDF5/Group.cpp \
../lib/HDF5/IdComponent.cpp \
../lib/HDF5/Matlab.cpp \
../lib/HDF5/MatlabDiagMatrix3.cpp \
../lib/HDF5/MatlabVector2.cpp \
../lib/HDF5/MatlabVector3.cpp \
../lib/HDF5/Object.cpp \
../lib/HDF5/OpaqueType.cpp \
../lib/HDF5/PropList.cpp \
../lib/HDF5/PropLists.cpp \
../lib/HDF5/ReferenceType.cpp \
../lib/HDF5/SerializationKey.cpp \
../lib/HDF5/Type.cpp \
../lib/HDF5/Util.cpp 

OBJS += \
./lib/HDF5/Array.o \
./lib/HDF5/AtomicType.o \
./lib/HDF5/AtomicTypes.o \
./lib/HDF5/Attribute.o \
./lib/HDF5/BaseTypes.o \
./lib/HDF5/ComplexConversion.o \
./lib/HDF5/CompoundType.o \
./lib/HDF5/DataSet.o \
./lib/HDF5/DataSpace.o \
./lib/HDF5/DataType.o \
./lib/HDF5/DataTypes.o \
./lib/HDF5/DelayedArray.o \
./lib/HDF5/Exception.o \
./lib/HDF5/File.o \
./lib/HDF5/Group.o \
./lib/HDF5/IdComponent.o \
./lib/HDF5/Matlab.o \
./lib/HDF5/MatlabDiagMatrix3.o \
./lib/HDF5/MatlabVector2.o \
./lib/HDF5/MatlabVector3.o \
./lib/HDF5/Object.o \
./lib/HDF5/OpaqueType.o \
./lib/HDF5/PropList.o \
./lib/HDF5/PropLists.o \
./lib/HDF5/ReferenceType.o \
./lib/HDF5/SerializationKey.o \
./lib/HDF5/Type.o \
./lib/HDF5/Util.o 

CPP_DEPS += \
./lib/HDF5/Array.d \
./lib/HDF5/AtomicType.d \
./lib/HDF5/AtomicTypes.d \
./lib/HDF5/Attribute.d \
./lib/HDF5/BaseTypes.d \
./lib/HDF5/ComplexConversion.d \
./lib/HDF5/CompoundType.d \
./lib/HDF5/DataSet.d \
./lib/HDF5/DataSpace.d \
./lib/HDF5/DataType.d \
./lib/HDF5/DataTypes.d \
./lib/HDF5/DelayedArray.d \
./lib/HDF5/Exception.d \
./lib/HDF5/File.d \
./lib/HDF5/Group.d \
./lib/HDF5/IdComponent.d \
./lib/HDF5/Matlab.d \
./lib/HDF5/MatlabDiagMatrix3.d \
./lib/HDF5/MatlabVector2.d \
./lib/HDF5/MatlabVector3.d \
./lib/HDF5/Object.d \
./lib/HDF5/OpaqueType.d \
./lib/HDF5/PropList.d \
./lib/HDF5/PropLists.d \
./lib/HDF5/ReferenceType.d \
./lib/HDF5/SerializationKey.d \
./lib/HDF5/Type.d \
./lib/HDF5/Util.d 


# Each subdirectory must supply rules for building sources it contributes
lib/HDF5/%.o: ../lib/HDF5/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DOMPI_SKIP_MPICXX -DCL_USE_DEPRECATED_OPENCL_1_1_APIS -I"/home/sunkg/workspace/OpenCLExercise5_VolumeRendering/lib" -I/usr/include/hdf5/serial -I/usr/include/mpi -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


