函数绑定:int sched_setaffinity(pid_t pid, size_t cpusetsize, cpu_size_t* mask);
pid:要绑定的进程ID
	cpusetsize : cpu_size_t的大小
	mask : 放到指定的cpu
	cpu_set_t cpu_affinity; //定义一个cpu变量 
	CPU_ZREO(&affinity); //清空变量cpu_affinity
	CPU_SET(哪个核, &affinity); //绑定到哪个核  查看CPU信息 cat /proc/cpuinfo


#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>

	int main(void) {
		pid_t pid;
		if ((pid = fork()) == 0) {
			cpu_set_t cpu_affinity;
			CPU_ZERO(&cpu_affinity);
			CPU_SET(0, &cpu_affinity);
			sched_setaffinity(0, sizeof(cpu_set_t), &cpu_affinity);
			while (1) {
				printf("child\n");
			}
		}
		else {
			cpu_set_t cpu_affinity;
			CPU_ZERO(&cpu_affinity);
			CPU_SET(1, &cpu_affinity);
			sched_setaffinity(0, sizeof(cpu_set_t), &cpu_affinity);
			while (1) {
				printf("parent\n");
			}
		}
	}
	//ps -eo ruser,pid,ppid,psr,args //查看进程在哪个核运行

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define ERR_ERNO(msg) \
   do{\
        printf("[%s][%d] %s:%s\n ",__FILE__,__LINE__,msg,strerror(errno));\
        exit(EXIT_FAILURE);\
    }while(0)


	int main(void) {
		pid_t pid;
		if ((pid = fork()) == -1) ERR_ERNO("fork");
		if (pid == 0) {
			printf("%d\n", getpid());
			//sleep(1);
			//getchar();//将子进程阻塞
			exit(0);
		}
		else {
			pid_t ret;
			int status;
			sleep(1);
			if ((ret = waitpid(pid, &status, WNOHANG)) == -1) ERR_ERNO("waitpid");
			if (WIFEXITED(status)) { //正常退出返回真
				printf("%d\n", WEXITSTATUS(status));//获取错误码信息
			}
			else if (WIFSIGNALED(status)) { //异常退出返回真
				printf("killed by kill cmd\n");
			}
			printf("%d\n", ret);
			while (1) {
				printf(".");
				fflush(stdout);
				sleep(1);
			}
		}
	}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

	int mysystem(const char* cmd) {
		pid_t pid;
		if ((pid = fork()) == -1) {
			exit(127);
		}
		if (pid == 0) {
			if (execlp("/bin/sh", "/bin/sh", "-c", cmd, NULL) == -1) {
				exit(1);
			}
		}
		//子进程一定走不到这里
		int status;
		while (waitpid(pid, &status, WNOHANG) != pid)
			;
		if (WIFEXITED(status)) {//子进程正常退出,返回真
			return WEXITSTATUS(status);//获取子进程的退出码
		}
		return -1;//子进程不是正常退出返回-1
	}

	int main(void) {
		printf("before system\n");
		int ret = mysystem("ls -l");
		printf("after system\n");

	}


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

	//获取命令行参数
	int get_input(char* buf) {
		memset(buf, 0x00, sizeof(buf));
		if (scanf("%[^\n]%*c", buf) != 1) {
			int c;
			do c = getchar(); while (c != '\n');
		}
		return strlen(buf);
	}
	void do_action(int argc, char* argv[]) {
		pid_t pid;
		switch (pid = fork()) { //创建子进程
		case -1:
			printf("当前shell出现故障,正在维护当中...\n");
			return;
		case 0: {
			int ret = execvp(argv[0], argv);//子进程中用argv[0],也就是我们输入的命令替换子进程空间
			printf("%s : command not found\n", argv[0]);
			exit(1);
		}
				break;
		default: {
			int status;
			waitpid(pid, &status, 0);//父进程等待子进程死亡
		}
				 break;
		}
	}
	//解析命令
	void do_parse(char* buf) {
		int argc = 0;
		char* argv[10] = {};
#define OUT 0
#define IN  1
		int status = OUT;

		int i;
		for (i = 0; buf[i] != '\0'; i++) {
			if (status == OUT && !isspace(buf[i])) {
				argv[argc++] = buf + i;
				status = IN;
			}
			else if (isspace(buf[i])) {
				status = OUT;
				buf[i] = '\0';
			}
		}
		argv[argc] = NULL;//命令行参数最后一定为NULL
		do_action(argc, argv);//执行命令
	}


	//主函数
	int main(void) {
		//循环打印
		char buf[512];
		while (1) {
			printf("[zhangxiaoyin shell]#");
			if (get_input(buf) == 0)
				continue;
			// printf("=>[%s]\n",buf);
			do_parse(buf);
		}

	}
