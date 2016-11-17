#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "spidrvpxcmds.h"

#define u32 uint32_t
#define u8 uint8_t

void err_die(char *msg, int sock)
{
        perror(msg);
        if(sock != 0) 
                close(sock);
        exit(EXIT_FAILURE); 
}

int main(int argc, char **argv)
{
        struct sockaddr_in sa;
        int res;
        int connect_fd;
        char recvbuff[256];

        connect_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (connect_fd == -1) {
                err_die("cannot create socket", 0);
        }

        memset(&sa, 0, sizeof sa);

        sa.sin_family = AF_INET;
        sa.sin_port = htons(50000);
        res = inet_pton(AF_INET, "192.168.122.11", &sa.sin_addr);
        //res = inet_pton(AF_INET, "127.0.0.1", &sa.sin_addr);

        if (connect(connect_fd, (struct sockaddr *)&sa, sizeof sa) == -1) {
                err_die("connect failed", connect_fd);
        }

        /* perform read write operations ... */
        //u32 cmd =CMD_GET_FIRMWVERSION;// CMD_GET_SOFTWVERSION;
        u32 cmd = CMD_SET_VPXREG;// CMD_GET_SOFTWVERSION;
        u32 addr = 0;
        u8 nbytes = 100;
        u32 len = (4+1)*4 + nbytes; 
        u32 dummy = 0; 
        u32 dev = 1; 
        u8 cnt = 0; 
        u32 param = ((cnt&0xff) <<24) | (nbytes & 0xff) << 16 | (addr & 0xffff); 
        u32 reqmsg[512];
        u32 repmsg[512];
        bzero(reqmsg, sizeof(reqmsg));
        // bytes 0-4 are Server Header 
        reqmsg[0] = htonl(cmd);  // CMD
        reqmsg[1] = htonl(len);  // total len = payload Nbytes + (4+1)*4
        reqmsg[2] = dummy;  // dummy
        reqmsg[3] = htonl(dev);  // device
        reqmsg[4] = htonl(param);  // "param" = [ cnt 8b | size 8b | addr 16b ]
        reqmsg[16] = 0xaaaa;  // reqmsg[5...] = payload bytes
        int ret = write(connect_fd, reqmsg, len);

        fprintf(stderr, "cmd=%x ret=%d\n",  cmd, ret);
        bzero(repmsg, sizeof(repmsg));
        ret = read(connect_fd, repmsg, sizeof(repmsg));
        if (ret < 0){
                err_die("read failed", connect_fd);
        }
        printf(repmsg);
        printf("\n");

        shutdown(connect_fd, SHUT_RDWR);
        close(connect_fd);
        return EXIT_SUCCESS;
}
