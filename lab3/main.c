#include "header.h"

int main()
{
	//共享存储区
	int shm = shmget(shmKey, sizeof(struct shm_data), IPC_CREAT|0666);
	if (shm == -1) {
		printf("main: shm got failed\n");
		exit(0);
	}
	SHM_DATA Buf = shmat(shm, NULL, SHM_R|SHM_W);
	Buf->in = 0;
	Buf->out = 0;
	Buf->readbytenum = 0;
	//信号灯
	int semid = semget(semKey, 2,  IPC_CREAT | 0666);
	if (semid == -1) {
		printf("main: sem got failed\n");
		exit(0);
	}
	union semun arg;
	arg.val = num;
	semctl(semid, 0, SETVAL, arg);	//信号灯0：表示空闲存储区的个数，初值为num
	arg.val = 0;
	semctl(semid, 1, SETVAL, arg);	//信号灯1：表示环形缓冲区中存在数据的存储区个数，初值为0
	
	pid_t readbuf, writebuf;	//进程readbuf，writebuf
	
	readbuf = fork();
	if(readbuf == -1) {
		printf("readproc created failed\n");
		exit(0);
	} else if(readbuf == 0) {
		execv("./readbuf", NULL);
	} else {
		writebuf = fork();
		if(writebuf == -1) {
			printf("writeproc created failed\n");
			exit(0);
		} else if(writebuf == 0) {
			execv("./writebuf", NULL);
		}
	}
	wait(&readbuf);
	wait(&writebuf);
	
	semctl(semid, 0, IPC_RMID);
	semctl(shm, IPC_RMID, 0);
	return 0;
}
