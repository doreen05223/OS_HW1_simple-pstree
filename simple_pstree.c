#include"simple_pstree.h"

int main(int argc, char *argv[])
{
    int state;
    //int retval;
    //int state_smg=0;
    struct sockaddr_nl src_addr, dest_addr;
    struct nlmsghdr *nlh = NULL;
    struct iovec iov;
    struct msghdr msg;
    int sock_fd;
    char in[20];

    sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_TEST);
    /*if(sock_fd == -1){
                printf("error getting socket: %s", strerror(errno));
                return -1;
            }*/

    memset(&msg,0,sizeof(msg));
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    if(argv[1]==NULL) {
        src_addr.nl_pid = 1;
    } else {
        strcpy(in,argv[1]);
        if(!in[2] && in[1]=='c') {
            src_addr.nl_pid = 1;
        } else if(!in[2] && in[1]=='s') {
            src_addr.nl_pid = getpid();
        } else if(!in[2] && in[1]=='p') {
            src_addr.nl_pid = getpid();
        } else {
            for(int i=0; i<18; i++) {
                in[i]=in[i+2];
            }
            int pid = atoi(in);
            src_addr.nl_pid = pid;
        }
    }
    src_addr.nl_groups = 0;		//not in mcast groups
    bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));
    /*if(retval < 0){
                printf("bind failed: %s", strerror(errno));
                close(sock_fd);
                return -1;
            }*/

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD_SIZE));
    /*if(!nlh){
                printf("malloc nlmsghdr error!\n");
                close(sock_fd);
                return -1;
            }*/

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;		//for linux kernel
    dest_addr.nl_groups = 0;	//unicast

    //fill the netlink message header
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD_SIZE);
    nlh->nlmsg_flags = 0;
    if(argv[1]==NULL) {
        nlh->nlmsg_pid = 1;
        nlh->nlmsg_seq = 3;
    } else {
        strcpy(in,argv[1]);
        if(!in[2] && in[1]=='c') {
            nlh->nlmsg_pid = 1;
            nlh->nlmsg_seq = 3;
        } else if(!in[2] && in[1]=='s') {
            nlh->nlmsg_pid = getpid();
            nlh->nlmsg_seq = 2;
        } else if(!in[2] && in[1]=='p') {
            nlh->nlmsg_pid = getpid();
            nlh->nlmsg_seq = 1;
        } else {
            if(in[1]=='p') nlh->nlmsg_seq = 1;
            else if(in[1] == 's') nlh->nlmsg_seq = 2;
            else if(in[1] == 'c') nlh->nlmsg_seq = 3;
            for(int i=0; i<18; i++) {
                in[i]=in[i+2];
            }
            int pid = atoi(in);
            nlh->nlmsg_pid=pid;
        }
    }
    //fill in the netlink message payload
//	strcpy(NLMSG_DATA(nlh), "Say hello from user application!");

    iov.iov_base = (void *)nlh;
    iov.iov_len=NLMSG_SPACE(MAX_PAYLOAD_SIZE);

    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

//	printf("Sending message to kernel\n");
    sendmsg(sock_fd, &msg,0);
    /*if(state_smg == -1){
                printf("get error sendmsg = %s\n",strerror(errno));
        }*/

    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD_SIZE));
//	printf("Waiting for message from kernel\n");

    //read message from kernel
    state = recvmsg(sock_fd, &msg, 0);
    if(state < 0)	printf("recvmsg state < 1");
    else	printf("%s\n",NLMSG_DATA(nlh));

    //Close netlink socket
    close(sock_fd);
    return 0;
}
