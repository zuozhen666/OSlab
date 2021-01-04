#include "header.h"
//#include <time.h>

int main()
{
	int shm = shmget(shmKey, sizeof(struct shm_data), IPC_CREAT|0666);
	if (shm == -1) {
		printf("writebuf: shm got failed\n");
		exit(0);
	}
	SHM_DATA Buf = shmat(shm, NULL, SHM_R|SHM_W);
	
	int semid = semget(semKey, 2,  IPC_CREAT | 0666);
	if (semid == -1) {
		printf("writebuf: sem got failed\n");
		exit(0);
	}

	FILE* fp = fopen("output.txt","w");
	long tmp = 0;
	while(1) {
		//srand((unsigned)time(NULL));
		//sleep(rand()%3 + 1);
		P(semid, 1);//申请有数据存在的存储区
		printf("in writeproc:	");
		printf("current out: %d	", Buf->out);
		if((Buf->out + 1)%num == Buf->in)
		{
			tmp += fwrite(Buf->buf[Buf->out], sizeof(char), Buf->readbytenum, fp);
			if(Buf->readbytenum != size) {
				printf("writebuf: %ld\n", tmp);
				printf("writeproc over\n");
				break;
			}
		} else tmp += fwrite(Buf->buf[Buf->out], sizeof(char), size, fp);
		printf("writebuf: %ld	", tmp);
		Buf->out = (Buf->out + 1)%num;
		printf("out update to %d\n", Buf->out);
		V(semid,0);//环形缓冲区中空闲存储区加1
	}
	return 0;
}
