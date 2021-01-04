#include "header.h"
//#include <time.h>

int main() {
	int shm = shmget(shmKey, sizeof(struct shm_data), IPC_CREAT|0666);
	if (shm == -1) {
		printf("readbuf: shm got failed\n");
		exit(0);
	}
	SHM_DATA Buf = shmat(shm, NULL, SHM_R|SHM_W);
	
	int semid = semget(semKey, 2,  IPC_CREAT | 0666);
	if (semid == -1) {
		printf("readbuf: sem got failed\n");
		exit(0);
	}
	
	FILE* fp = fopen("input.txt","r");
	long tmp = 0;
	while(1) {
		//srand((unsigned)time(NULL));
		//sleep(rand()%3 + 1);
		P(semid, 0);//申请空闲存储区
		Buf->readbytenum = fread(Buf->buf[Buf->in], sizeof(char), size, fp);
		tmp += Buf->readbytenum;
		printf("in readproc:	");
		printf("current in: %d	", Buf->in);
		printf("readbuf: %ld	", tmp);
		V(semid, 1);//环形缓冲区中有数据存在的存储区加1
		Buf->in = (Buf->in + 1)%num;
		printf("in update to %d\n", Buf->in);
		if(Buf->readbytenum != size) {
			printf("readproc over\n");
			break;
		}
	}
	return 0;
}
