CC=i586-poky-linux-gcc
HOST_PATH=/opt/iot-devkit/1.7.2/sysroots/x86_64-pokysdk-linux/usr/bin/i586-poky-linux
EXECTION_ENV=BOARD
GCC=gcc

all: 
ifeq ($(EXECTION_ENV),BOARD)
	$(HOST_PATH)/$(CC) -pthread -Wall -o RGBLed_1.o RGBLed_1.c
	$(HOST_PATH)/$(CC) -pthread -Wall -o RGBLed_2.o RGBLed_2.c	
else		
	$(GCC) -pthread -Wall -o RGBLed_1.o RGBLed_1.c
	$(GCC) -pthread -Wall -o RGBLed_2.o RGBLed_2.c
endif

clean:
	rm -f *.o
