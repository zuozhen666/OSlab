/******************************************************************
1.实验内容
1）线程互斥
编程模拟实现飞机售票：
//创建多个售票线程
//已售票使用共用全局变量
//创建互斥信号灯
//对售票线程临界区施加P，V操作，售票线程打印售票信息
//主进程等待子线程退出，各线程在票卖完时打印售票总数，退出
2）线程同步
设计并实现一个计算线程，两个打印线程共享一个缓冲区的同步与通信，程序要求：
//共享缓冲区使用共享公共变量a;
//计算线程负责计算（1到100的累加，每次加一个数），结果为偶数由打印线程1打印，奇数由打印线程2打印
//主进程等待子线程退出
2.预备知识 
@Linux下的信号灯及其P，V操作
	在Linux中信号灯是一个数据集合，可以单独使用这一集合的每个元素
	//semget：创建或返回一个被内核指定的整型的信号灯集索引
	//semop：执行对信号灯的操作
	//semctl：执行对信号灯的控制操作
	信号灯的定义：
	数据原型struct semid_ds{}在linux/sem.h中定义
	//信号灯创建
		系统调用semget()创建一个信号灯集，或者存取一个已经存在的信号灯集
		int semget(key_t key,int nsems,int semflg);
		如果成功，返回信号灯集的IPC标识符;
		如果失败，返回-1;
		//key是一个常数，为IPC_PRIVATE表明由系统选用一个关键字
		//nsems创建的信号灯个数，信号灯编号为0到nsems-1
		//semflg创建的权限标志，如IPC_CREAT | 0666 表示不存在则创建
		//成功时返回信号灯的ID，否则为-1
	//信号灯操作
		系统调用semop()
		int semop(int semid,struct sembuf *sops,unsigned nsops);
		如果成功，返回0,失败为-1
		struct sembuf {
			ushort sem_num;//使用哪个信号灯
			short sem_op;//操作
			short sem_flg;//操作标志
		}
		sops信号灯集合（数组）nsops信号灯个数
	P操作接口
		void P(int semid, int index)
		{
			struct sembuf sem;
			sem.sem_num = index;
			sem.sem_op = -1;
			sem.sem_flg = 0;
			semop(semid,&sem,1);
			return;
		}
	V操作接口
		void V(int semid, int index)
		{
			struct sembuf sem;
			sem.sem_num = index;
			sem.sem_op = 1;
			sem.sem_flg = 0;
			semop(semid,&sem,1);
			return;
		}
	//信号灯控制
		系统调用semctl()
		int semctl(int semid,int semnum,int cmd,union semun arg);
		如果成功，返回一个正数
		//semid要操作的信号灯集ID
		//semnum信号灯集中信号灯的编号
		cmd命令:
			IPC_RMID将信号灯集semid从内存中删除
			SETALL设置信号灯集中所有的信号灯的值
			SETVAL设置信号灯集中的一个单独信号灯的值
			如
				arg.val = 1;
				semctl(semid,1,SETVAL,arg);
@Linux线程使用
	//线程创建
		pthread_create(pthread_t *thread,pthread_attr_t *attr,void *(*start_routine)(void*),void *arg);
	//线程等待
		pthread_join(pthread_t th,void **thread_return);
		挂起当前线程直到由参数th指定的线程被终止为止
	//线程撤销
		void pthread_exit__P((void *__retval))__attribute__((__noreturn__));
******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <linux/sem.h>

#define N 20

int tickets = N;			//票数
pthread_t p1, p2, p3;		//线程句柄
int semid;					//信号灯

void *subp1();
void *subp2();
void *subp3();
void P(int semid, int index);
void V(int semid, int index);

//显式声明，解决warning
int semget(key_t key,int nsems,int semflg);
int semop(int semid,struct sembuf *sops,unsigned nsops);
int semctl(int semid,int semnum,int cmd,union semun arg);

int main() 
{
	//创建信号灯
	semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
	if(semid == -1) {
		printf("sem created error!\n");
		exit(1);
	}
	
	//信号灯赋初值
	union semun arg;
	arg.val = 1;
	int rc = semctl(semid, 0, SETVAL, arg);
	if(rc < 0) {
		printf("sem initlized error!\n");
		exit(1);
	}
	
	//创建线程
	pthread_create(&p1, NULL, subp1, NULL);
	pthread_create(&p2, NULL, subp2, NULL);
	pthread_create(&p3, NULL, subp3, NULL);
	
	//等待线程运行结束
	pthread_join(p1, NULL);
	pthread_join(p2, NULL);
	pthread_join(p3, NULL);
	
	//删除信号灯
	semctl(semid, 0, IPC_RMID, arg);
	
	printf("\nSold Out.\n");
	return 0;
}

void P(int semid, int index)
{
	struct sembuf sem;
	sem.sem_num = index;
	sem.sem_op = -1;
	sem.sem_flg = 0;
	semop(semid,&sem,1);
	return;
}

void V(int semid, int index)
{
	struct sembuf sem;
	sem.sem_num = index;
	sem.sem_op = 1;
	sem.sem_flg = 0;
	semop(semid,&sem,1);
	return;
}

void *subp1()
{
	int sum = 0;
	while(1) {
		P(semid, 0);
		if(tickets > 0) {
			tickets--;
			sum++;
			printf("线程1售出一张票，剩余%d张票.\n", tickets);
			V(semid, 0);
		}
		else break;
	}
	V(semid, 0);
	printf("线程1共售出%d张票.\n", sum);
}

void *subp2()
{
	int sum = 0;
	while(1) {
		P(semid, 0);
		if(tickets > 0) {
			tickets--;
			sum++;
			printf("线程2售出一张票，剩余%d张票.\n", tickets);
			V(semid, 0);
		}
		else break;
	}
	V(semid, 0);
	printf("线程2共售出%d张票.\n", sum);
}

void *subp3()
{
	int sum = 0;
	while(1) {
		P(semid, 0);
		if(tickets > 0) {
			tickets--;
			sum++;
			printf("线程3售出一张票，剩余%d张票.\n", tickets);
			V(semid, 0);
		}
		else break;
	}
	V(semid, 0);
	printf("线程3共售出%d张票.\n", sum);
}





























































