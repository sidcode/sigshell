/* This file will have the main headers used in the shell
   as well as macros
*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<sys/types.h>
#include<string.h>
#include<errno.h>


#define CMD_LENGTH_MAX 50
#define JOB_QUEUE_MAX 50

//#defines for various process statuses
#define RUN_FG 0
#define RUN_BG  1
#define STP_BG 2
#define TERMINATED 3
