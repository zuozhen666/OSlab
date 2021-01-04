/*********************实验任务**********************
lab1content
//编写程序，演示多进程并发执行和进程软中断，管道通信;
//父进程建立一个无名管道，再创建子进程1和子进程2
//父进程每隔1秒通过管道发送消息（消息个数可设置上限）;
I send you x times.（x初值为1,每次发送后做加一操作）
子进程1,2从管道读出消息，并显示在屏幕上;
//父进程捕捉来中断信号SIGINT（即按Ctrl+C键），
然后向两个子进程分别发出信号SIGUSR1,SIGUSR2;
//子进程捕捉到信号后分别输出收到的消息总数和各自终止信息;
Child Process 1 is Killed by Parent!
Child Process 2 is Killed by Parent!
//父进程等待两个子进程终止后，释放管道并输出发送的消息总数后终止
Parent Process is Killed!
**************************************************/
/*********************前导知识******************	****
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void my_func(int sig_no) {
    if(sig_no == SIGUSR1)
        printf("Receive SIGUSR1.\n");
    if(sig_no == SIGUSR2)
        printf("Receive SIGUSR2.\n");
    if(sig_no == SIGINT) {
        printf("Receive SIGINT.\n");
        exit(0);
    }
}
//进程的软中断机制
//即信号机制，提供一种简单的处理异步事件的方法，
//在一个或多个进程之间传递异步信号

int main() {
    //signal(sig, function)
    //预置信号接收后的处理方式
    //function = 1，屏蔽该类信号
    //function = 0，到sig信号后终结自己
    //function = another integer，执行用户设置的软中断处理程序
    if(signal(SIGUSR1, my_func) == SIG_ERR)
        printf("can't catch SIGUSR1.\n");
    if(signal(SIGUSR2, my_func) == SIG_ERR)
        printf("can't catch SIGUSR2.\n");
    if(signal(SIGINT, my_func) == SIG_ERR)
        printf("can't catch SIGINT.\n");
    kill(getpid(), SIGINT); //getpid()获取目前进程的进程标识
    //int kill(pid, dig)
    //向一个进程或一组进程发送一个信号
    //pid > 0，核心将信号发送给进程pid
    //pid < 0，核心将信号发送给与发送进程同组的所有进程
    //pid = -1，核心将信号发送给所有用户标识符真正等于发送进程的有效用户标识号的进程
    while(1);
    return 0;
}

//int pipefd[2];  int pipe(pipefd);
//pipefd[0]只能用于读，pipefd[1]只能用于写
//write()写入
//受长度限制，管道满时阻塞，fcntl()可设置非阻塞模式
//read()读取（读取后数据自动被清除）
//不能由一个进程同时向多个进程同时传递同一个数据
//fcntl()可设置为非阻塞模式
//close()关闭
//关闭读端口时，在管道上进行写操作的进程将收到SIGPIPE信号
//关闭写端口时，进行读操作的read()函数将返回0
***************************************************/

/*********************实验指导**********************
main( ) {
    创建无名管道；
    设置软中断信号SIGINT；
    创建子进程1、2；
    定时发送数据；
    等待子进程1、2退出；
    关闭管道；
    打印信息、退出；}

父进程信号处理 {
    发SIGUSR1给子进程1；
    发SIGUSR2给子进程2；
    等待子进程1、2退出；
     关闭管道；
    打印信息、退出；
}

子进程1/2 {
 设置信号SIGINT处理；
 设置SIGUSR1或2处理；
    while(1) {
       从管道接收数据；
         显示数据；
       计数器++；
           }
关闭管道；
打印信息、退出；
}

SIGUSR1/2信号处理 {
    关闭管道；
    打印信息；
    退出；
}

问题：如果父进程是通过消息上限发完消息，父子进程如何正常退出？
**************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

pid_t pid1, pid2;				//子进程PID
const int LIST = 20;				//设置阈值
int pipefd[2];

void Father(int sig_no);
void Child1(int sig_no);
void Child2(int sig_no);

int main() {
	pipe(pipefd);				//创建无名管道
	signal(SIGINT, Father);			//父进程对信号SIGINT做反应
	char sendstr[66], recstr[66];		//发送接收信息缓冲区
	int x = 1;				//初始发送次数
	pid1 = fork();				//创建子进程1
	if(pid1 == -1) {
		printf("pid1 created error!\n");
		exit(0);		
	}
	else if(pid1 == 0) {			//子进程1
		signal(SIGINT, SIG_IGN);	//屏蔽信号SIGINT				  
		signal(SIGUSR1, Child1);	//对信号SIGUSR1做反应
		while(1) {
			read(pipefd[0], recstr, sizeof(recstr));	//接收管道数据
        		printf("pid1: %s", recstr);			//显示数据
			sleep(2);		
		}
	}
	else {
		pid2 = fork();						//创建子进程2
		if(pid2 == -1) {
			printf("pid2 created error!\n");
			kill(pid1, SIGUSR1);
			wait(&pid1);
			exit(0);		
		}
		else if(pid2 == 0) {			//子进程2
			signal(SIGINT, SIG_IGN);	//屏蔽信号SIGINT				  
			signal(SIGUSR2, Child2);	//对信号SIGUSR2做反应
			while(1) {
				read(pipefd[0], recstr, sizeof(recstr));	//接收管道数据
        			printf("pid2: %s", recstr);			//显示数据	
				sleep(2);
			}
		}
		else {								//主进程
			while(1) {
				sprintf(sendstr, "I send you %d times\n" ,x);
				write(pipefd[1], sendstr, sizeof(sendstr));

				x++;
				if(x == LIST) {
					printf("Times limit exceeded\n");
					Father(SIGINT);
				}
				sleep(1);
			}
		}	
	}
	return 0;
}

void Father(int sig_no) {
	kill(pid1, SIGUSR1);
	kill(pid2, SIGUSR2);
	wait(&pid1);
	wait(&pid2);
	close(pipefd[0]);
	close(pipefd[1]);
	printf("Parent Process is Killed!\n");
	exit(0);
}

void Child1(int sig_no) {
	printf("\nChild Process 1 is Killed by Parent!\n");
	exit(0);
}

void Child2(int sig_no) {
	printf("\nChild Process 2 is Killed by Parent!\n");
	exit(0);
}
