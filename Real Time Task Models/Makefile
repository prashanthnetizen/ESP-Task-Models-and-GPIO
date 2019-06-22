CC=i586-poky-linux-gcc
HOST_PATH=/opt/iot-devkit/1.7.2/sysroots/x86_64-pokysdk-linux/usr/bin/i586-poky-linux
EXECTION_ENV=HOST

all: mainProgram.c
ifeq ($(EXECTION_ENV),HOST)
	$(HOST_PATH)/$(CC) -pthread -Wall -o mainProgram.o mainProgram.c	
else		
	$(CC) -pthread -Wall -o mainProgram.o mainProgram.c
endif

clean:
	rm -f *.o
