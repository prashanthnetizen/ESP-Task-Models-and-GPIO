#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include<pthread.h>
#include<signal.h>

#define MAXPIN 3
#define MOUSEPATH "/dev/input/mice"
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define SYSFS_PWM_DIR "/sys/class/pwm/pwmchip0"
#define MAX_BUF 64
#define PERIOD 20000000

typedef struct{
    unsigned int shield;    // Arduino pin
    int pwm;                // PWM signal Pin
    int dir;                // Direction Pin
    unsigned int mux1;      // Multiplexer 1 pin
    unsigned int mv1;       // Multiplexer 2 Pin value
    unsigned int mux2;      // Multiplexer 1 pin
    unsigned int mv2;       // Multiplexer 2 pin value
}ioPin;


//Delay to be given in nanoseconds
unsigned int delay;
// Duty cycle in percentage
float duty;
//LED file descriptors
unsigned int led[MAXPIN];

ioPin pin[MAXPIN];

/****************************************************************
 * gpio_export -  to export the given gpio pin into sysfs
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
 * pwm_export - to export the given pwm pin into sysfs
 ****************************************************************/
int pwm_export(unsigned int pwm)
{
	int fd, len;
	char buf[MAX_BUF];

	fd = open(SYSFS_PWM_DIR "/export", O_WRONLY);
	if (fd < 0) {
		perror("pwm/export");
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", pwm);
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


/****************************************************************
 * pwm_set_enable - enabling or disabling the exported pwm pin
 ****************************************************************/
int pwm_set_enable(unsigned int pwm, unsigned int value)
{
	int fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), SYSFS_PWM_DIR "/pwm%d/enable", pwm);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("pwm/set-enable");
		return fd;
	}

	if (value)
		write(fd, "1", 1);
	else
		write(fd, "0", 1);

	close(fd);
	return 0;
}

/****************************************************************
 * pwm_set_duty - setting the pwm duty cycle value
 ****************************************************************/
int pwm_set_duty(unsigned int pwn, unsigned int duty)
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_PWM_DIR "/pwm%d/duty_cycle", pwn);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("pwm/set-duty");
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", duty);
	write(fd, buf, len);
	close(fd);

	return 0;
}



/****************************************************************
 * pwm_set_period - setting the pwn total period
 ****************************************************************/
int pwm_set_period(unsigned int period)
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_PWM_DIR "/device/pwm_period");

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("pwm/set-period");
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", period);
	write(fd, buf, len);

	close(fd);
	return 0;
}


//Galileo Gen 2 PWM pin configuration
char strings[6][20] = {"3,1,16,76-0,64-1","5,3,18,66-1,0-0","6,5,20,68-1,0-0","9,7,22,70-0,0-0","10,11,26,74-1,0-0","11,9,24,0-0,72-1"};



/**
    The setPinIdentity function is to gather the configuration details of each arduino pin provided as input and
    populates the necessary values for the data structure ioPin
*/

ioPin setPinIdentity(int in){
    ioPin pi;
    char * tok;
    //Search array that includes the eligible PWM pins
    int search[6] = {3,5,6,9,10,11};
    int i;

    //Linear search throughout the array to fetch the index value
    for(i = 0 ; i < 6 ; i++){
        if(in == search[i]){
        break;
        }
    }

    // if pins other than pwm are provided, then the program terminates on error message
    if(i == 6){
        perror("Invalid pin is provided as input. Please check the Pins");
        raise(SIGTERM);
    }

    // Tokenizing the given config file
    tok = strtok(strings[i],",-");

    // Shield pin for given IO
    pi.shield = atoi(tok);
    tok = strtok(NULL,",-");

    // PWM pin for given IO
    pi.pwm = atoi(tok);
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

This Cycler method is execute the given step of the sequence for every 0.5 seconds
*/

void cycler(const char *red, const char *green, const char *blue){

        write(led[0],red,1);
        write(led[1],green,1);
        write(led[2],blue,1);
        //sleeping for 0.5 seconds
        usleep(500000);
}


/**
This function will set the new duty cycle value upon calculation defined on the event trigger
*/

void setDelay(){
 delay = (duty/100) * PERIOD;
 pwm_set_duty(pin[0].pwm,delay);
 pwm_set_duty(pin[1].pwm,delay);
 pwm_set_duty(pin[2].pwm,delay);

}

/**
This method will act as a thread body for the mouse thread to read the events of the mouse clicking and doing necessary action

*/

void *checkClick(void *ptr){

    int fd, hexBytes;
    unsigned char data[3];
    //Getting the device driver path
    const char *mouseDevPath = MOUSEPATH;
    // Open MouseFile
    fd = open(mouseDevPath, O_RDWR);
    if(fd == -1)
    {
        printf("ERROR Opening the path - %s\n", mouseDevPath);
    }

    int left=0, right=0;

    while(1){

        hexBytes = read(fd, data, sizeof(data));

        if(hexBytes > 0)
        {
            left = data[0] & 0x1;
            right = data[0] & 0x2;

            //If Left Click is detected
            if(left == 1 && right == 0){
                duty = duty + 10;
                printf("Left click increases duty %f percent\n",duty);
                if(duty <= 100){
                    setDelay();
                }
            } else if (left == 0 && right == 2){  //for right clicks
                duty = duty - 10;
                printf("Right click decreases duty %f percent\n",duty);

                if(duty >= 0){
                    setDelay();
                }
            }
        }
    }
    return NULL;
}

/*********************************************************

THE MAIN PROGRAM

**********************************************************/


int main(){

    pthread_t mouseThread;
    int index;
    char buf[MAX_BUF];
    // Reading the input values from the console
    scanf("%f %d %d %d", &duty, &pin[0].shield, &pin[1].shield, &pin[2].shield);
    //Setting the total period value for the PWM cycle
    pwm_set_period(PERIOD);
    //calculating the duty cycle in nanoseconds
    delay = (duty/100) * PERIOD;

    /*
    Initiating the necessary pins for perfect input / output operations
    */
    for(index = 0 ; index < MAXPIN ; index++){

        pin[index] = setPinIdentity(pin[index].shield);

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
        // exporting the pwm pin
        pwm_export(pin[index].pwm);

        // Initiating the enable value to
        pwm_set_enable(pin[index].pwm,0);

        //Setting the initial duty cycle value
        pwm_set_duty(pin[index].pwm,delay);

        //inititing the given LED file descriptors for the operation
        snprintf(buf, sizeof(buf), SYSFS_PWM_DIR "/pwm%d/enable", pin[index].pwm);
        led[index] = open(buf, O_WRONLY);

    }

    pthread_create( &mouseThread, NULL, checkClick, NULL);

    while(1){

    // Initiating the Sequence

        cycler("0","0","0");    //  none, none, none
        cycler("1","0","0");    //  Red, none, none
        cycler("0","1","0");    //  none, Green, none
        cycler("0","0","1");    //  none, none, Blue
        cycler("1","1","0");    //  Red, Green, none
        cycler("1","0","1");    //  Red, none, Blue
        cycler("0","1","1");    //  none, Green, Blue
        cycler("1","1","1");    //  Red, Green, Blue

    }
    return 0;
}
