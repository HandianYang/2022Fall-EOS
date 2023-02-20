LIB = ElevatorAlgorithm/ElevatorProperty.c ElevatorAlgorithm/ElevatorPolicy.c ElevatorAlgorithm/ElevatorOperation.c ElevatorAlgorithm/InterProcessCommunication.c 

MOD = ElevatorGpio

obj-m := $(MOD).o

# Remember to modify the kernel path!!
RPI_KDIR = ../../linux

all:
	gcc main.c -o main.o $(LIB) -lm
build:  # [Note] PC only!
	sudo make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(RPI_KDIR) M=$(shell pwd) modules
load:	# [Note] RPI only!
	sudo insmod $(MOD).ko
unload:	# [Note] RPI only!
	sudo rmmod $(MOD)
clean:
	sudo make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(RPI_KDIR) M=$(shell pwd) clean
	rm -rf main.o