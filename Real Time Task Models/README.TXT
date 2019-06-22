ESP-Team30-assgn01


Steps to run the Program:

1) unzip the package to any working directory in your machine or Galileo Board.

2) Install and setup the board according to the instructions provided in the following link:

https://www.dropbox.com/sh/7t0tf61zkxmtxxw/AAB4HiFN5YmckFe6-l2J6kkTa/Galileo%20Gen%202%20SDK?dl=0&preview=Setting-Up_Galileo_2018.pdf&subfolder_nav_tracking=1

3) Before planning to run the code, open the terminal and add EXECTION_ENV as an environment variable to your machine and assign values as 

	"HOST" if its your x86 Host Machine
	"BOARD" if its the Galileo Gen 2 Board

you can use the command  EXECTION_ENV=HOST$EXECTION_ENV whenever you start a bash terminal session in your host machine to assign environment variable, also do the same for your board

then provide the command "make" in the directory where you have unzipped the files. This should compile your source code.

Finally to execute provide the command:

sudo ./mainProgram.o

This should execute the program and Check the Log files in the same directory for any Entries



******************************************************
TEAM MEMBERS
******************************************************

1. Prasanth Sukhapalli		-	1215358560
2. Kai Yuan Leong 		- 	1215666010

******************************************************
