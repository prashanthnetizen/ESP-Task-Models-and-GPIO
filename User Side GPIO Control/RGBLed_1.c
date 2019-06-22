#include <stdio.h>
#include <unistd.h>

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>

#define MAXPIN 3
#define CYCLE_DUR 20000
#define MOUSEPATH "/dev/input/mice"
#define STEP_CYCLES 25
#define TIME_PER_CYCLE 20000
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define MAX_BUF 64

typedef struct{
int shield;     // Arduino pin
int lin;        // GPIO Linux Pin
int dir;        // Direction Pin
int mux1;       // Multiplexer 1 pin
int mv1;        // Multiplexer 1 Pin value
int mux2;       // Multiplexer 2 pin
int mv2;        // Multiplexer 2 pin value
}ioPin;

//Global variables
int delay, universal = 1;
// File descriptors for the LED files
int led[MAX_BUF];

/****************************************************************
 * gpio_export to export the given gpio pin into sysfs
 ****************************************************************/
int gpio_export(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];

	fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);

	return 0;
}


/****************************************************************
 * gpio_set_dir - setting direction to the Direction pin
 ****************************************************************/
int gpio_set_dir(unsigned int gpio, unsigned int out_flag)
{
	int fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/direction");
		return fd;
	}

	if (out_flag)
		write(fd, "out", 3);
	else
		write(fd, "in", 2);

	close(fd);
	return 0;
}

/****************************************************************
 * gpio_set_value - setting value to the pin
 ****************************************************************/
int gpio_set_value(unsigned int gpio, unsigned int value)
{
	int fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-value");
		return fd;
	}

	if (value)
		write(fd, "1", 1);
	else
		write(fd, "0", 1);

	close(fd);
	return 0;
}

// Pin Configuration for GPIO Programming in Galileo Gen 2 board
char strings[20][20] = {"11,32,0-0,0-0","12,28,45-0,0-0","13,34,77-0,0-0","14,16,76-0,64-0","6,36,0-0,0-0","0,18,66-0,0-0","1,20,68-0,0-0","38,-1,0-0,0-0","40,0,0-0,0-0","4,22,70-0,0-0","10,26,74-0,0-0","5,24,44-0,72-0","15,42,0-0,0-0","7,30,46-0,0-0","48,-1,0-0,0-0","50,-1,0-0,0-0","52,-1,0-0,0-0","54,-1,0-0,0-0","56,-1,60-1,78-1","58,-1,60-1,79-1"};


/**
    The setPinIdentity function is to gather the configuration details of each arduino pin provided as input and
    populates the necessary values for the data structure ioPin
*/

ioPin setPinIdentity(int pin){
    ioPin pi;
    char * tok;

    // if pins other than pwm are provided, then the program terminates on error message
    if(pin < 0 || pin > 19){
        perror("Invalid pin is provided as input. Please check the Pins");
        raise(SIGTERM);
    }

    // Tokenizing the given config file
    tok = strtok(strings[pin],",-");

    // Linux pin for given IO
    pi.lin = atoi(tok);
    tok = strtok(NULL,",-");

    // GPIO direction pin for given IO
    pi.dir = atoi(tok);
    tok = strtok(NULL,",-");

    //Mux1 pin for given IO
    pi.mux1 = atoi(tok);
    tok = strtok(NULL,",-");

    //Mux1 pin value for given IO
    pi.mv1 = atoi(tok);
    tok = strtok(NULL,",-");

    //Mux2 pin for given IO
    pi.mux2 = atoi(tok);
    tok = strtok(NULL,",-");

    //Mux2 pin value for given IO
    pi.mv2 = atoi(tok);
    tok = strtok(NULL,",-");

    return pi;

}

/**
This method will act as a thread body for the mouse thread to read the events of the mouse clicking and doing necessary action

*/

void *checkClick(void *ptr){

    int fd, hexBytes;
    unsigned char data[3];
    const char *mouseDevPath = MOUSEPATH;
    // Open MouseFile
    fd = open(mouseDevPath, O_RDWR);
    if(fd == -1)
    {
        printf("ERROR Opening the path - %s\n", mouseDevPath);
    }

    int left=0, right=0;

    while(1){
        //Reading content from the mouse file
        hexBytes = read(fd, data, sizeof(data));

        if(hexBytes > 0)
        {
            left = data[0] & 0x1;
            right = data[0] & 0x2;
            //Restart the sequence when user clicks left or right. set the flag accordingly
            if(left == 1 || right == 2){
                universal = 0;
            }
        }
    }
    return NULL;
}

/**

This Cycler method is execute the given step of the sequence for every 20 milliseconds for 25 cycles totalling upto 0.5 seconds
*/

void cycler(const char *red, const char *green, const char *blue){;
    int i;
    for(i = 0 ; (i < STEP_CYCLES && universal > 0); i++){
        // Ton Period
        write(led[0],red,1);
        write(led[1],green,1);
        write(led[2],blue,1);
        usleep(delay);
        // Toff Period
        write(led[0],"0",1);
        write(led[1],"0",1);
        write(led[2],"0",1);
        usleep(TIME_PER_CYCLE - delay);
    }
}

/*********************************************************

THE MAIN PROGRAM

**********************************************************/


int main(){

    int index;
    char buf[MAX_BUF];
    // declaring the mouse thread
    pthread_t mouseThread;
    // duty to receive input
    float duty;

    ioPin pin[3];

    // Reading the input values from the console
    scanf("%f %d %d %d", &duty, &pin[0].shield, &pin[1].shield, &pin[2].shield);

    //Initiating the setup for the given LEDs
    for(index = 0 ; index < MAXPIN ; index++){
    //Setting the Pin Identity
        pin[index] = setPinIdentity(pin[index].shield);

        // Exporting the given linux pins
        gpio_export(pin[index].lin);
        gpio_set_dir(pin[index].lin,1);

        //checking if there is a direction pin for the given shield pin
        if(pin[index].dir >= 0){
            gpio_export(pin[index].dir);
            gpio_set_dir(pin[index].dir,1);
            gpio_set_value(pin[index].dir,0);
        }

        //checking if there are multiplexer pins for the given shield pin
        if(pin[index].mux1 > 0){
            gpio_export(pin[index].mux1);
            gpio_set_value(pin[index].mux1,pin[index].mv1);
        }

        if(pin[index].mux2 > 0){
            gpio_export(pin[index].mux2);
            gpio_set_value(pin[index].mux2,pin[index].mv2);
        }

        //initiating the given LED file descriptors for the operation
        snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", pin[index].lin);
        led[index] = open(buf, O_WRONLY);
    }

    //Calculating the time delays to provide for Ton Period
    duty = (duty/100) * TIME_PER_CYCLE;
    delay = (int)duty;

    pthread_create( &mouseThread, NULL, checkClick, NULL);

    while(1){

        while(universal > 0){

        // None
        cycler("0","0","0");    //  none, none, none
        cycler("1","0","0");    //  Red, none, none
        cycler("0","1","0");    //  none, Green, none
        cycler("0","0","1");    //  none, none, Blue
        cycler("1","1","0");    //  Red, Green, none
        cycler("1","0","1");    //  Red, none, Blue
        cycler("0","1","1");    //  none, Green, Blue
        cycler("1","1","1");    //  Red, Green, Blue

        }
        universal = 1;
    }

    return 0;

}
