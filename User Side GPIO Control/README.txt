ESP-Team30- Assignment 2. GPIO control in Linux

Steps to run the Program:

1) unzip the package to any working directory in your machine or Galileo Board.

2) Install and setup the board according to the instructions provided in the following link:

https://www.dropbox.com/sh/7t0tf61zkxmtxxw/AAB4HiFN5YmckFe6-l2J6kkTa/Galileo%20Gen%202%20SDK?dl=0&preview=Setting-Up_Galileo_2018.pdf&subfolder_nav_tracking=1

3) There is only a single Makefile in the package which is written to compile both the programs RGBLed_1.c and RGBLed_2.c. Before compiling the program, determine the environment on which the program has to run.

if the given environment is Intel Galileo Gen 2 board, then give the value EXECTION_ENV as "BOARD" in the Makefile. The Programs are primarily written with Galileo Gen 2 board's configuration.

open the working directory and give the command as "make", it should compile both the programs and gives out the object files.

Transfer the object files to the Galileo board by any means you know and get inside the board's terminal using screen or putty.

execute the given programs as follows:

	"./RGBLed_1.o" for the first program
	"./RGBLed_1.o" for the second program

Please provide inputs as described in the assignment description. Provide the duty cycle as a percentage value i.e. between 0 to 100.

Arduino pin configuration varies for both the assignments. Please go to the below link for pin config details:

https://www.dropbox.com/sh/7t0tf61zkxmtxxw/AAD0sE84bTZTjEMybwjc1VW8a/Galileo%20Gen%202%20SDK/Quark_documents?dl=0&preview=Gen2_pins.xlsx&subfolder_nav_tracking=1

The above instructions should provide the output required for the assignment question.

******************************************************
TEAM MEMBERS
******************************************************

1. Prasanth Sukhapalli		-	1215358560
2. Kai Yuan Leong 		- 	1215666010

******************************************************
