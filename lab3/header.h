#pragma once

#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define num 2		//环形缓冲区的存储区数
#define size 20		//存储区的byte数

typedef struct shm_data {
	char buf[num][size];	//环形缓冲区
	int in;			//输入指针in
	int out;		//输出指针out
	int readbytenum;	//一次读取的字节数
}*SHM_DATA;

union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short * arry;
};

void P(int semid, int index);
void V(int semid, int index);

void P(int semid,int index)
{
	struct sembuf sem;
	sem.sem_num = index;
	sem.sem_op = -1;
	sem.sem_flg = 0;
	semop(semid, &sem, 1);
}

void V(int semid,int index)
{
	struct sembuf sem;
	sem.sem_num = index;
	sem.sem_op = 1;
	sem.sem_flg = 0;
	semop(semid, &sem, 1);
}

const int shmKey = 23333;	//共享存储区键值
const int semKey = 22333;	//信号灯键值
