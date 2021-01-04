/*
    实验内容：
    编程实现目录查询功能：
        （1）功能类似ls-lR;
        （2）查询指定目录下的文件及子目录信息;
        显示文件的类型，大小，时间等信息;
        （3）递归显示子目录中的所有文件信息。
    1.Linux文件属性接口
    int fstat(int fildes,struct stat *buf);
        返回文件描述符相关的文件的状态信息
    int stat(const char *path,struct stat *buf);
        通过文件名获取文件信息，并保存在buf所指的结构体stat中
    int lstat(const char *path,struct stat *buf);
        如读取到了符号连接，lstat读取符号连接本身的状态信息，而stat读取的是符号连接指向文件的信息。
    stat结构体：保存文件状态信息
    2.Linux目录结构接口
    DIR *opendir(const char *name);
    通过路径打开一个目录，返回一个DIR结构体指针（目录流），失败返回NULL;
    struct dirent *readdir(DIR *);
    读取目录中的下一个目录项，没有目录项可以读取时，返回为NULL;
    dirent结构体：目录项结构
    int chdir(const char *path);
    改变目录，与用户通过cd命令改变目录一样，程序也可以通过chdir来改变目录，这样使得fopen(),opendir(),
    这里需要路径的系统调用，可以使用相对于当前目录的相对路径打开文件（目录）
    int closedir(DIR*)
    关闭目录流
程序结构：
	void printdir(char *dir,int depth){
		DIR *dp;
		struct dirent *entry;
		struct stat statbuf;
		if ((dp = 打开dir目录) 不成功){
			打印出错信息;
			返回;
		}
		将dir设置为当前目录;
		while(读到一个目录项){
			以该目录项的名字为参数，调用lstat得到该目录项的相关信息;
			if(是目录){
				if(目录项的名字是“..”或“.”)
					跳过该目录项;
				打印目录项的深度，目录名等信息
				递归调用printdir，打印子目录的信息，其中depth+4;
			}
			else 打印文件的深度，文件名等信息;
		}
		返回父目录;
		关闭目录项;
	}
	
	int main(...){
		......
	}
 */
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

void printinfo(struct stat state) {
	mode_t mode = state.st_mode;
	//输出文件类型
	if(S_ISREG(mode)) putchar('-');
	if(S_ISLNK(mode)) putchar('l');
	if(S_ISDIR(mode)) putchar('d');
	if(S_ISCHR(mode)) putchar('c');
	if(S_ISBLK(mode)) putchar('b');
	if(S_ISFIFO(mode)) putchar('f');
	if(S_ISSOCK(mode)) putchar('s');
	//用户权限
	if(S_IRUSR&mode) putchar('r'); else putchar('-');
	if(S_IWUSR&mode) putchar('w'); else putchar('-');
	if(S_IXUSR&mode) putchar('x'); else putchar('-');
	//组内其他用户权限
	if(S_IRGRP&mode) putchar('r'); else putchar('-');
	if(S_IWGRP&mode) putchar('w'); else putchar('-');
	if(S_IXGRP&mode) putchar('x'); else putchar('-');
	//其他组用户权限
	if(S_IROTH&mode) putchar('r'); else putchar('-');
	if(S_IWOTH&mode) putchar('w'); else putchar('-');
	if(S_IXOTH&mode) putchar('x'); else putchar('-');
	putchar(' ');
	//文件硬连接数
	printf("%d	", (int)state.st_nlink);
	//文件拥有者
	struct passwd* pwd = getpwuid(state.st_uid);
	printf("%s	", pwd->pw_name);
	//所属组
	struct group* grp = getgrgid(state.st_gid);
	printf("%s	", grp->gr_name);
	//文件大小
	printf("%ld	", state.st_size);
	//最后修改内容的时间
	struct tm time;
	localtime_r(&state.st_mtime, &time);
	printf("%d-%d-%d	", time.tm_year + 1900, time.tm_mon + 1, time.tm_mday);
}

void printdir(char* dir, int depth) {
	DIR* dp;
	struct dirent* entry;
	struct stat* statbuf = (struct stat*)malloc(sizeof(struct stat));
	if((dp = opendir(dir)) == NULL) {
		printf("Fail to open the directory:%s\n", dir);
		return;	
	}
	chdir(dir);	//当前目录
	while((entry = readdir(dp))!= NULL) {
		
		lstat(entry->d_name, statbuf);
		if(S_ISDIR(statbuf->st_mode)) {	//目录
			if((strcmp(entry->d_name, "..")) == 0 || strcmp(entry->d_name, ".") == 0)
				continue;
			else {
				printf("depth:%d ",depth);
			  	printinfo(*statbuf);
			  	printf("%s\n", entry->d_name);
			  	printdir(entry->d_name, depth + 4);
			}
		}
		else {	//文件
			printf("depth:%d ", depth);
			printinfo(*statbuf);
			printf("%s\n", entry->d_name);
		}
	}
	chdir("../");//返回父目录
	closedir(dp);//关闭目录项
}

int main(int argc,char* argv[]) {
	if(argc != 2) 
		printf("input error!\n");
	else
		printdir(argv[1], 0);
	return 0;
}





































