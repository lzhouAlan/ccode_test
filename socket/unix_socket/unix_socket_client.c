#include <unistd.h>
#include <sys/socket.h>
#include <linux/un.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <pthread.h>



#define UNIX_DOMAIN_PATH    "/tmp/unix_server"
#define DEBUG(FMAT,ARGS...) printf("[client]%s %d:"FMAT"\n", __func__, __LINE__, ## ARGS)

pthread_t tid_unix_sk[4];
void thread(void *arg);
int count = 0;
int main()
{
    int i;
    
    for (i = 0; i < sizeof(tid_unix_sk)/sizeof(pthread_t); i++) {
	sleep(1);
        pthread_create(&tid_unix_sk[i], NULL, thread, i);
    }
    sleep(1);
    for (i = 0; i < sizeof(tid_unix_sk)/sizeof(pthread_t); i++) {
	sleep(1);
        pthread_join(tid_unix_sk[i], NULL);
    }
}

void thread(void *arg)
{
    int i = (int)arg;
    int skfd;
    char buf[1024*1024];
    socklen_t sock_len;
    struct sockaddr_un  local;
    struct sockaddr_un  server;
    struct sockaddr_un  from;

    memset(&server, 0x00, sizeof(server));
    server.sun_family = AF_UNIX;
    snprintf(server.sun_path, sizeof(server.sun_path), "%s", UNIX_DOMAIN_PATH);

    if ((skfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        DEBUG("Failed to create socket\n");
        return -1;
    }

    memset(&local, 0x00, sizeof(local));
    local.sun_family = AF_UNIX;
    snprintf(local.sun_path, sizeof(local.sun_path), "%s-%d", UNIX_DOMAIN_PATH, i);
    unlink(local.sun_path);

    if (bind(skfd, (struct sockaddr *)&local, sizeof(local)) < 0) {
	DEBUG("Failed to bind socket to unix path (%s)!\n", local.sun_path);
	perror("error:");
    } else {
	break;
    }

    /* connect */
    if (connect(skfd, &server, sizeof(server)) < 0) {
	printf("Failed to connect sever\n");
	return -1;
    }

    while (1) {
        sock_len = sizeof(from);
        if (i < 3) {
            /* thread(0 ~ 2) send msg */
            if (sendto(skfd, buf, sizeof(buf) - 1, 0, (struct sockaddr *) &server, sizeof(server)) < 0) {
                DEBUG("Thread[%d] Sent msg server!\n", i);
            }
        } else {
            /* thread(3) recv msg */
            if (recvfrom(skfd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&from, &sock_len) < 0) {
                DEBUG("Failed to recvfrom error!\n");
                return -1;
            } else {  
		printf("receive packets from %s\n", from.sun_path);
	    }
	 
        }
#if 0
        if (recvfrom(skfd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&from, &sock_len) < 0) {
            DEBUG("Failed to recvfrom error!\n");
            return -1;
        }
        DEBUG("Recv one connection !\n");
        if (sendto(skfd, buf, sizeof(buf) - 1, 0, (struct sockaddr *) &from, sizeof(from)) < 0) {
            DEBUG("Sent back connect!\n");
        }
#endif
        sleep(5);
    }
}
