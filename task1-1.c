#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
int flag=0;//用于标记是否受了信号
void inter_handler(int sig) {
    flag=1;
}
void waiting(int sig) {
    printf("stop for %d\n",sig);
    while(!flag){
       pause();
    }
    exit(0);
}
int main() {
    signal(SIGINT,inter_handler);
    signal(SIGQUIT,inter_handler);
    signal(SIGALRM, inter_handler);
    pid_t pid1=-1, pid2=-1;
    while (pid1 == -1)pid1=fork();
    if (pid1 > 0) {
        while (pid2 == -1)pid2=fork();
        if (pid2 > 0) {
            printf("stop for 14");
            alarm(5);//5秒后发出SIGALRM信号
            
            while(!flag)pause();//要么等5s,要么接受到信号。
            if(flag){

                kill(pid1,SIGSTKFLT);//向子进程1发送编号16的信号
                kill(pid2,SIGCHLD);//向子进程2发送编号17的信号
            }
            wait(NULL);
            wait(NULL);
            printf("\nParent process is killed!!\n");
        } 
        else {
            waiting(SIGCHLD);//17
            printf("\nChild process2 is killed by parent!!\n");
            return 0;
        }
    } 
    else {
        waiting(SIGSTKFLT);//16
        printf("\nChild process1 is killed by parent!!\n");
        return 0;
    }
    return 0;
}
