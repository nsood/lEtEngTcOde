#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#define PORT    17777
#pragma pack(1)
struct router_msg{
    uint32_t m_bussId;
    uint16_t m_version;
    uint16_t m_type;
    uint16_t m_contentLen;
    char m_content[0];
};
struct router_msg_test{
    uint32_t m_bussId;
    uint16_t m_version;
    uint16_t m_type;
    uint16_t m_contentLen;
    char m_content[0];
};

int main(void)
{
    int sockfd = 0;
    struct sockaddr_in serveraddr;
    int addrlen = 0;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serveraddr.sin_addr);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    addrlen = sizeof(serveraddr);
    int n = 0;
    int len = 11;
    struct router_msg *msg = (struct router_msg*)malloc(sizeof(struct router_msg) + len);
    memset(msg, 0, sizeof(struct router_msg) + len);
    msg->m_bussId = 123456;
    msg->m_version = 1;
    msg->m_type = 5;
    msg->m_contentLen = len;
    memcpy(msg->m_content, "1234567890", 10);
    char  recvline[1024];
    printf("len:%d\n", sizeof(struct router_msg));
    while(1)
    {
        int ret = sendto(sockfd, (char*)(msg), sizeof(struct router_msg_test) + len, 0, (const struct sockaddr*)&serveraddr, addrlen);
        if(ret == -1)
        {
            perror("sendto");
            exit(1);
        }
        printf("ret = %d\n", ret);
        n = recvfrom(sockfd, recvline, 1024, 0, NULL, NULL);
        printf("recv:%s\n", recvline);
    }
}
