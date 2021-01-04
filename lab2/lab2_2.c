/*********************************************
2）线程同步
设计并实现一个计算线程，两个打印线程共享一个缓冲区的同步与通信，程序要求：
//共享缓冲区使用共享公共变量a;
//计算线程负责计算（1到100的累加，每次加一个数），结果为偶数由打印线程1打印，奇数由打印线程2打印
//主进程等待子线程退出
*********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <linux/sem.h>

const int N = 100;
int tmp = 1;
int flag_odd = 1, flag_even = 1;
int a = 0;						//共享公共变量
int semid;						//信号灯
pthread_t p1, p2, p3;			//线程句柄

void P(int semid, int index);	//P操作
void V(int semid, int index);	//V操作
void *calculate();				//计算线程
void *print_even();				//打印线程1
void *print_odd();				//打印线程2

//显式声明，解决warning
int semget(key_t key,int nsems,int semflg);
int semop(int semid,struct sembuf *sops,unsigned nsops);
int semctl(int semid,int semnum,int cmd,union semun arg);

int main()
{
	//创建信号灯
	semid = semget(IPC_PRIVATE, 3, IPC_CREAT|0666);
	if(semid == -1) {
		printf("sem created error!\n");
		exit(1);	
	}

	//信号灯赋初值
	union semun arg;
	arg.val = 1;
	int rc = semctl(semid, 0, SETVAL, arg);	//信号灯0，计算权限，初值为1
	if(rc < 0) {
		printf("sem initialized error!\n");
		exit(1);
	}
	arg.val = 0;
	rc = semctl(semid, 1, SETVAL, arg);		//信号灯1，偶数打印权限，初值为0
	if(rc < 0) {
		printf("sem initialized error!\n");
		exit(1);
	}
	arg.val = 0;
	rc = semctl(semid, 2, SETVAL, arg);		//信号灯2，奇数打印权限，初值为0
	if(rc < 0) {
		printf("sem initialized error!\n");
		exit(1);
	}
	
	//创建线程
	pthread_create(&p1, NULL, calculate, NULL);
	pthread_create(&p2, NULL, print_even, NULL);
	pthread_create(&p3, NULL, print_odd, NULL);
	
	//等待线程结束
	pthread_join(p1, NULL);
	pthread_join(p2, NULL);
	pthread_join(p3, NULL);
	
	//删除信号灯
	semctl(semid, 0, IPC_RMID, arg);
	
	printf("\nCalculation completed!\n");
	return 0;
}

void P(int semid, int index)
{
	struct sembuf sem;
	sem.sem_num = index;
	sem.sem_op = -1;
	sem.sem_flg = 0;
	semop(semid, &sem, 1);
	return;
}

void V(int semid, int index)
{
	struct sembuf sem;
	sem.sem_num = index;
	sem.sem_op = 1;
	sem.sem_flg = 0;
	semop(semid, &sem, 1);
	return;
}

void *calculate()
{
	while(tmp < N + 1) {
		P(semid, 0);					//申请计算权限
		printf("calculating......\n");
		a += tmp;
		tmp++;
		if(a%2 == 0) V(semid, 1);		//释放偶数打印权限
		else V(semid, 2);				//释放奇数打印权限
	}
	if(a%2 == 0) {
		flag_odd = 0;
		V(semid, 2);					//保证线程odd正常结束
	}
	else {
		flag_even = 0;
		V(semid, 1);					//保证线程even正常结束
	}					
	printf("线程calculate结束！\n");
}

void *print_even()
{
	while(tmp < N + 1) {
		P(semid, 1);					//申请偶数打印权限
		if(flag_even)
			printf("printThread_even: a = %d\n", a);
		V(semid, 0);					//释放计算权限
	}
	printf("线程print_even结束！\n");
}

void *print_odd()
{
	while(tmp < N + 1) {
		P(semid, 2);					//申请奇数打印权限
		if(flag_odd)
			printf("printThread_odd: a = %d\n", a);
		V(semid, 0);					//释放计算权限
	}
	printf("线程print_odd结束！\n");
}



























