#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>

int flag = 0; // 用于标记是否接收到信号
pid_t pid1 = -1, pid2 = -1;

// 父进程信号处理函数
void inter_handler(int sig) {
    flag = 1; // 设置flag，表示接收到信号
    if (pid1 != -1) kill(pid1, SIGSTKFLT); // 向子进程1发送信号
    if (pid2 != -1) kill(pid2, SIGCHLD); // 向子进程2发送信号
}

// 子进程信号处理函数
void child_handler(int sig) {
    // 设置flag，使pause()结束
    flag = 1;
}

int main() {
    // 为父进程设置信号处理函数
    signal(SIGINT, inter_handler);
    signal(SIGQUIT, inter_handler);
    signal(SIGALRM, inter_handler);
    
    // 创建子进程1
    while (pid1 == -1) pid1 = fork();
    if (pid1 > 0) {
        // 创建子进程2
        while (pid2 == -1) pid2 = fork();
        
        if (pid2 > 0) {
            printf("Parent process, waiting for signal or timeout.\n");
            alarm(5); // 5秒后发出SIGALRM信号
            while (!flag) pause(); // 等待信号
            wait(NULL); // 等待任意子进程结束
            wait(NULL); // 等待另一个子进程结束
            printf("Parent process is killed!!\n");
        } 
        else {
            // 子进程2的代码
            signal(SIGCHLD, child_handler); // 设置信号处理函数
            printf("wait for signal2\n");
            while (!flag) pause();
            printf("Child process2 is killed by parent!!\n");
            return 0;
        }
    } 
    else {
        // 子进程1的代码
        signal(SIGSTKFLT, child_handler); // 设置信号处理函数
        printf("wait for signal1\n");
        while (!flag) pause();
        printf("Child process1 is killed by parent!!\n");
        return 0;
    }

    return 0;
}
