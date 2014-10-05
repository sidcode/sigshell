#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<sys/types.h>
#include<string.h>
#include<errno.h>


#define CMD_LENGTH_MAX 50
#define JOB_QUEUE_MAX 50

#define RUN_FG 0
#define RUN_BG  1
#define STP_BG 2
#define TERMINATED 3

//Signal received Variable
int sig_received = 0;
int sig_received1 = 0;
//Job Control Structure
struct job_control
{   
    int jid;
    pid_t pid;
    int jstatus;
};

//Job Control Array
struct job_control job_array[JOB_QUEUE_MAX];


//Print Prompt string
void PS()
{

    char domain_name[1024];
    char cwd[1024];//current working directory

    gethostname(domain_name,1024);
    getcwd(cwd, sizeof(cwd));
    printf("%s@%s %s$ ",getenv("LOGNAME"),domain_name,cwd);
}

//Handlers for signals
void sigint_handler(int p)
{
    sig_received = 1;
    sig_received1 = 1;
    printf("\nSIGINT received\nPress Enter to continue.\n");
    
    fflush(stdout);


    return;
}

void sigtstp_handler(int p)
{
    sig_received = 1;
    sig_received1 = 1;
    printf("\nSIGTSTP received\nPress Enter to continue.\n");
    fflush(stdout);
    return;

}

//Function to Print Job Structure -- JID,PID,Status

void print_job(struct job_control job)
{
    char *stat;

    
    switch(job.jstatus)
    {
        case RUN_FG: stat = "Foreground Running"; break;
        case RUN_BG: stat = "Background Running"; break;
        case TERMINATED: stat = "Terminated"; break;
        case STP_BG: stat = "Background Stopped"; break;
    }

    printf("%d\t\t%d\t\t\t%s",job.jid,job.pid,stat);
}


static int job_count;

int main()
{ 
    pid_t prog;//the process id for the entered command
    int status;
    
    job_count = 0;



    signal(SIGINT,sigint_handler);
    signal(SIGTSTP,sigtstp_handler);

    job_array[job_count].jid = 0;
    job_array[job_count].pid = getpid();
    job_array[job_count].jstatus = RUN_FG;

    job_count++;
    
    while(1)
    {
        //Prompt String
        PS();

        int cmd_type = 0; //0 for fg processes & 1 for bg processes
        int valid_process =0;
        int custom_command = 0;
        int trail_present = 0;   
        //Parse the input
        char inp[CMD_LENGTH_MAX];
        
        //Get input from standard input
        fgets(inp,CMD_LENGTH_MAX,stdin);
        
        //Tokenize by whitespace
        char *tok = strtok(inp," ");
        char *comm[CMD_LENGTH_MAX];
        comm[0] = tok;
        int i = 0;

        //tokenizing logic
        while(tok)
        {

            i++;
            tok = strtok(NULL," ");
            comm[i] = tok;
        }
       
       if(comm[0][0] == '\n')
       {
           trail_present = 1;
       } 
        //handle trailing \t \n or any other unintended characters
        size_t ln = strlen(comm[i-1]) -1;
        if(comm[i-1][ln] != '\0')
        {
            comm[i-1][ln]='\0';
            sig_received = 1;
        }

        //background process encountered
        if(comm[i-1][ln-1] == '&')
        {
            //custom_command = 1;
            cmd_type = 1;
            //remove & from the command string
            comm[i-1] = NULL;

        }
    
        //if jobs command encountered
        if(strcmp(comm[0],"jobs") == 0)
        {
            custom_command = 1;
            
            printf("\n--------------------------------------------------\n");
            printf("\t\tBackground Processes");
            printf("\n--------------------------------------------------\n");
            printf("Job ID\t\tProcess ID\t\tStatus\n");
            int count=0;
    
            for(count=0;count<job_count;count++)
            {
                print_job(job_array[count]);
                printf("\n");
            }
        
            printf("\n\n");
        }

        

        //bg command
        if(strcmp(comm[0],"bg") == 0)
        {
            custom_command = 1;    

            int job_to_kill = atoi(comm[1]);
            
            kill(job_array[job_to_kill].pid,SIGCONT);
            job_array[job_to_kill].jstatus = RUN_BG;
        }

        //fg command behaviour
        if(strcmp(comm[0],"fg") == 0)
        {
            custom_command =1;   
            int job_to_kill = atoi(comm[1]);
            
            kill(job_array[job_to_kill].pid,SIGCONT);
            job_array[job_to_kill].jstatus = RUN_FG;
        }

        //kill command
        if(strcmp(comm[0],"kill") == 0)
        {
            custom_command = 1;
            printf("JID to be killed is %s\n",comm[1]);
            int job_to_kill = atoi(comm[1]);
            
            kill(job_array[job_to_kill].pid,SIGKILL);
            job_array[job_to_kill].jstatus = TERMINATED;
            
            
        }

        if(strcmp(comm[0],"exit") == 0)
        {
            custom_command = 1;
            printf("\nShell exiting. Have a nice day ahead. Goodbye!\n\n");
            kill(0,SIGQUIT);
        }

        
        if(strcmp(comm[0],"cd") == 0)
        {
            custom_command = 1;
            chdir(comm[1]);
        }

        //Fork a new child process

        if(custom_command == 0)
        {
        prog = fork();
        }
        
        //Child's execution code
        
        if(prog == 0)
        {
            //execute the command entered
            signal(SIGINT,sigint_handler);
            signal(SIGTSTP,sigtstp_handler);
            
            execvp(comm[0],comm);
            
            
            
            if(custom_command == 0 && cmd_type == 0 && trail_present == 0 )
            {
                printf("Shell: %s command not found\n",comm[0]);
            }
    
            
            
            
        }
    
        //Parent's execution code
        else
        {

            if(sig_received !=0)
            {
            signal(SIGINT,SIG_IGN);
            signal(SIGTSTP,SIG_IGN);
            }
            //waitpid(prog,&status);
           //If the process is a foreground process
            

            if(cmd_type ==0)
            {
                waitpid(prog,&status,NULL);

                printf("PID is %d . Status is %d\n",prog,status);
                
            }

           else
           {
                //Handle background processes

               job_array[job_count].jid = job_count;
               job_array[job_count].pid = prog;
               job_array[job_count].jstatus = RUN_BG;
               

               printf("Job ID = [%d] PID = %d\n",job_count,prog);

               job_count++;
           }

        }

    }

   return(0); 
}
