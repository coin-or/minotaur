################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../CDlogdet.cpp \
../MIQCP.cpp \
../relax.cpp 

OBJS += \
./CDlogdet.o \
./MIQCP.o \
./relax.o 

CPP_DEPS += \
./CDlogdet.d \
./MIQCP.d \
./relax.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -ggdb -I/Users/hongbodong/workspace/Research/newbnb/build/include/minotaur -I/Users/hongbodong/workspace/Research/newbnb/minotaur-0.1.1-dev-mac-x86_64/third-party -I/Users/hongbodong/workspace/Research/newbnb/minotaur-0.1.1-dev-mac-x86_64/third-party/asl-20120126 -O0 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


