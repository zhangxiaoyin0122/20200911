������:int sched_setaffinity(pid_t pid, size_t cpusetsize, cpu_size_t* mask);
pid:Ҫ�󶨵Ľ���ID
	cpusetsize : cpu_size_t�Ĵ�С
	mask : �ŵ�ָ����cpu
	cpu_set_t cpu_affinity; //����һ��cpu���� 
	CPU_ZREO(&affinity); //��ձ���cpu_affinity
	CPU_SET(�ĸ���, &affinity); //�󶨵��ĸ���  �鿴CPU��Ϣ cat /proc/cpuinfo


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
	//ps -eo ruser,pid,ppid,psr,args //�鿴�������ĸ�������

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
			//getchar();//���ӽ�������
			exit(0);
		}
		else {
			pid_t ret;
			int status;
			sleep(1);
			if ((ret = waitpid(pid, &status, WNOHANG)) == -1) ERR_ERNO("waitpid");
			if (WIFEXITED(status)) { //�����˳�������
				printf("%d\n", WEXITSTATUS(status));//��ȡ��������Ϣ
			}
			else if (WIFSIGNALED(status)) { //�쳣�˳�������
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
		//�ӽ���һ���߲�������
		int status;
		while (waitpid(pid, &status, WNOHANG) != pid)
			;
		if (WIFEXITED(status)) {//�ӽ��������˳�,������
			return WEXITSTATUS(status);//��ȡ�ӽ��̵��˳���
		}
		return -1;//�ӽ��̲��������˳�����-1
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

	//��ȡ�����в���
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
		switch (pid = fork()) { //�����ӽ���
		case -1:
			printf("��ǰshell���ֹ���,����ά������...\n");
			return;
		case 0: {
			int ret = execvp(argv[0], argv);//�ӽ�������argv[0],Ҳ������������������滻�ӽ��̿ռ�
			printf("%s : command not found\n", argv[0]);
			exit(1);
		}
				break;
		default: {
			int status;
			waitpid(pid, &status, 0);//�����̵ȴ��ӽ�������
		}
				 break;
		}
	}
	//��������
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
		argv[argc] = NULL;//�����в������һ��ΪNULL
		do_action(argc, argv);//ִ������
	}


	//������
	int main(void) {
		//ѭ����ӡ
		char buf[512];
		while (1) {
			printf("[zhangxiaoyin shell]#");
			if (get_input(buf) == 0)
				continue;
			// printf("=>[%s]\n",buf);
			do_parse(buf);
		}

	}
