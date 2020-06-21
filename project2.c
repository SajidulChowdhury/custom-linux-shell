/* ----------------------------------------------------------------- */
/* PROGRAM  shell.c                                                  */
/*    This program reads in an input line, parses the input line     */
/* into tokens, and use execvp() to execute the command.             */
/* ----------------------------------------------------------------- */

#include  <stdio.h>
#include  <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>


struct node
{
     struct node* next;
     char** data;
};

/* ----------------------------------------------------------------- */
/* FUNCTION  parse:                                                  */
/*    This function takes an input line and parse it into tokens.    */
/* It first replaces all white spaces with zeros until it hits a     */
/* non-white space character which indicates the beginning of an     */
/* argument.  It saves the address to argv[], and then skips all     */
/* non-white spaces which constitute the argument.                   */
/* ----------------------------------------------------------------- */

void  parse(char *line, char **argv)
{
     while (*line != '\0') {       /* if not the end of line ....... */ 
          while (*line == ' ' || *line == '\t' || *line == '\n')
               *line++ = '\0';     /* replace white spaces with 0    */
          *argv++ = line;     /* save the argument position     */
          if(*line !='\"')
          {
               while (*line != '\0' && *line != ' ' && 
                 *line != '\t' && *line != '\n') 
               line++;  /* skip the argument until ...    */
          }
          else
          {
               line++;
               while(*line != '\"'){
                    line++;
               }
               line++;
          }
     }
     *argv = '\0';                 /* mark the end of argument list  */
}
  
/* ----------------------------------------------------------------- */
/* FUNCTION execute_descriptor:                                                 */
/*    This function receives a commend line argument list with the   */
/* first one being a file name followed by its arguments.  Then,     */
/* this function forks a child process to execute the command using  */
/* system call execvp().Utilizes file descriptors                    */
/* ----------------------------------------------------------------- */   
void  execute_descriptor(char **argv)
{
     int status;
     int pid = fork();
     
     if (pid == 0)
     {         
          int fd,fd1,i,in=0,out=0;
          char arr1[64],arr2[64];

          for(i=0;argv[i]!=NULL;i++)
          {
               if(strcmp(argv[i],"<")==0)
               {        
                    argv[i]=argv[i+1];
                    strcpy(arr1,argv[i+1]);
                    argv[i+1]=NULL;
                    in=2;           
               }               

               if(strcmp(argv[i],">")==0)
               {      
                    argv[i]=NULL;
                    strcpy(arr2,argv[i+1]);
                    out=2;
               }         
          }

          if(in)
          {   
               if ((fd = open(arr1, O_RDONLY, 0)) < 0) 
               {
                    perror("No file");
                    exit(0);
               }           
      
               dup2(fd, STDIN_FILENO); 

               close(fd); 
          }


          if (out)
          {
               if ((fd1 = creat(arr2 , 0644)) < 0) 
               {
                    perror("No file");
                    exit(0);
               }           

               dup2(fd1, STDOUT_FILENO); // 1 here can be replaced by STDOUT_FILENO
               close(fd1);
          }

               execvp(argv[0], argv);
               perror("error");
               _exit(1);
     }


    else if((pid) < 0)
    {     
        printf("Failed to fork\n");
        exit(1);
    }

    else 
    {                                
        while (!(wait(&status) == pid)) ; 
    }
}

void exececute_with_pipe(char** arg1, char** arg2) 
{ 
    // 0 is read end, 1 is write end 
     int status1, status2;
     int pipefd[2];  
     pid_t proc1, proc2; 
  
     if (pipe(pipefd) < 0) 
     { 
        printf("\nerror"); 
        return; 
     } 
     
     if ((proc1=fork()) < 0) 
     { 
        printf("\nFailed to fork"); 
        return; 
     } 
  
     if (proc1 == 0) 
     { 
        close(pipefd[0]); 
        dup2(pipefd[1], STDOUT_FILENO); 
        close(pipefd[1]); 
        if (execvp(arg1[0], arg1) < 0) 
        { 
               printf("\nComman1 failed"); 
               exit(0); 
        } 
     } 
     else 
     {  
          if ((proc2 = fork())< 0) 
          { 
               printf("\nFailed to fork"); 
               return; 
          } 
  
          if (proc2 == 0) 
          { 
            close(pipefd[1]); 
            dup2(pipefd[0], STDIN_FILENO); 
            close(pipefd[0]); 
            if (execvp(arg2[0], arg2) < 0) 
            { 
                    printf("\nCommand2 failed"); 
                    exit(0); 
            } 
          } 
          else 
          {    
               close(pipefd[1]);
               waitpid(proc2,&status2,0);
               waitpid(proc1,&status1,0);
          } 
    } 
}

void do_cd(char **argv)
{
     chdir(argv[1]);
     return;
}
    
/* ----------------------------------------------------------------- */
/*                  The main program starts here                     */
/* ----------------------------------------------------------------- */

int main(int argc, char **argv)
{
     char  line[1024];             /* the input line                 */
     int i;
     while (1) {                   /* repeat until done ....         */
          struct node* head = malloc(sizeof(struct node));
          struct node* current = malloc(sizeof(struct node));
          int count = 0; 
          printf("Shell -> ");     /*   display a prompt             */
          gets(line);              /*   read in the command line     */
          printf("\n");
          parse(line, argv);       /*   parse the line               */

          //finding the length of argv//
          while(argv[count] != NULL)
          {
          count++;
          }

          //Setting up linked list//
          current -> data = argv[0];
          current -> next = NULL;
          head = current;
          for(int i=1; i < count; i= i+1)
          {
               struct node* temp = malloc(sizeof(struct node));
               current->next = temp; 
               temp->data = argv[i];
               temp->next = NULL;
               current = temp;
          }
          

          //Printing of linked list in the format expected//
          current = head;
          printf("Commands: %s ",current-> data);
          while (current != NULL)
          {
               if((strcmp(current->data,"|")== 0) && (current-> next != NULL))
                    printf("%s ",current->next->data);
               current = current -> next;
          }

          current = head;
          printf("\n%s : ",current->data);
          while(current->next != NULL)
          {
               current= current->next;
               if(strcmp(current->data,"|")!=0){
                    printf("%s ", current->data);
               }

               if((strcmp(current->data,"|")==0)&& (current->next != NULL))
               {
                    current = current->next;
                    printf("\n%s : ",current->data);

               }
          }


          //finding number of commands. If it is more than 1 there is a pipe//
          int g=0;
          int num_of_commands=1;
          while(argv[g]!= NULL)
          {
               if(strcmp(argv[g],"|")==0)num_of_commands++;
               g++;
          } 


          /* The if statement initialize two array of characters and seperate the
          input to two commands if there is a pipe*/
          char* command1[100];
          char* command2[100];
          if(num_of_commands>1)
          {
               int j=0;
               int pipeIndex;

               while(argv[i] != NULL)
               {
                    if(strcmp(argv[i],"|")==0)
                    {
                         pipeIndex = i;
                    }
                    i++;
               }

          // separate commands
               for(i = 0; i < pipeIndex; i++)
               {
                    command1[i] = argv[i];
               }
               command1[i] = NULL;

               for(i = pipeIndex+1; i < 100; i++)
               {
                    command2[j] = argv[i];
                    j++;
               }    
               command2[i] = NULL;
          }
       

          printf("\n");
          if (strcmp(argv[0], "exit") == 0)  /* is it an "exit"?     */
               exit(0);            /*   exit if it is                */

          if(num_of_commands>1)
          {
          exececute_with_pipe(command1,command2);
          }
          else if(strcmp(argv[0], "cd")==0)
          {
               do_cd(argv);
          }
          else
          {
          execute_descriptor(argv);           /* otherwise, execute the command */
          }
     }

}

