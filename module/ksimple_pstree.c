#include"ksimple_pstree.h"

struct sock *nl_sk = NULL;

void sendnlmsg(int pid, int choose)
{
    struct sk_buff *skb;
    struct nlmsghdr *nlh;
    struct task_struct *p, *see;
    struct list_head *pp=0;
    struct task_struct *psibling;
    char ppid[10]="";
    char ccomm[100]="";
    char ancestor[2048]="";
    int display[2048];
    char disComm[2048][2048];
    char sibling[2048]="";
    char children[2048]="";

    if(!nl_sk) return;

    skb = nlmsg_new(MAX_PAYLOAD_SIZE,GFP_KERNEL);
    if(!skb) printk(KERN_ERR"nlmsg_new eror!\n");
    nlh = nlmsg_put(skb,0,0,0,MAX_PAYLOAD_SIZE,0);

    //current processor
    p = pid_task(find_vpid(pid), PIDTYPE_PID);

    int i=1;
    //-p
    if(choose == 1) {
        if(p == NULL) memcpy(NLMSG_DATA(nlh),ancestor,sizeof(ancestor));
        else {
            //parent
            if(p->parent == NULL) {
//				printk("No Parent\n");
                strcat(ancestor,"\n");
                strcat(ancestor,"    ");
            } else {
                //current
                display[0]=p->pid;
                strcpy(disComm[0],p->comm);
                do {
                    display[i]=p->parent->pid;
                    strcpy(disComm[i],p->parent->comm);
                    p=p->parent;
                    i++;
                } while(p->pid != 0);

                int jj=0,countJJ;
                int j=i-1;
                int trans[2048];
                char transComm[2048][2048];
                while(j>=0) {
                    trans[jj]=display[j];
                    sprintf(ccomm,"%s",disComm[j]);
                    sprintf(ppid,"(%d)",trans[jj]);
                    strcat(ancestor,ccomm);
                    strcat(ancestor,ppid);
                    strcat(ancestor,"\n");
                    countJJ=jj;
                    while(countJJ>=0) {
                        strcat(ancestor,"    ");
                        countJJ--;
                    }
                    j--;
                    jj++;
                }
            }
            memcpy(NLMSG_DATA(nlh),ancestor,sizeof(ancestor));
        }
    }

    //-s
    if(choose == 2) {
        if(p == NULL) memcpy(NLMSG_DATA(nlh),sibling,sizeof(sibling));
        else {
            //sibling
            list_for_each(pp, &p->parent->children) {
                psibling = list_entry(pp, struct task_struct, sibling);
                if(psibling->comm == NULL) {
                    strcat(sibling,"\n");
                    memcpy(NLMSG_DATA(nlh),sibling,sizeof(sibling));
                } else {
                    sprintf(ppid,"(%d)",psibling->pid);
                    strcat(sibling,psibling->comm);
                    strcat(sibling,ppid);
                    strcat(sibling,"\n");
                    //      			printk("    %s(%d)\n", psibling->comm, psibling->pid);
                    memcpy(NLMSG_DATA(nlh),sibling,sizeof(sibling));
                }
            }
        }
    }

    //-c
    if(choose == 3) {
        if(p == NULL) memcpy(NLMSG_DATA(nlh),children,sizeof(children));
        else {
            //current
            sprintf(ppid,"(%d)",p->pid);
            strcat(children,p->comm);
            strcat(children,ppid);
            strcat(children,"\n");
            strcat(children,"    ");
            //children
            list_for_each(pp, &p->children) {
                psibling = list_entry(pp, struct task_struct, sibling);
                if(psibling->comm == NULL) {
                    strcat(children,"\n");
                    memcpy(NLMSG_DATA(nlh),children,sizeof(children));
                } else {
                    sprintf(ppid,"(%d)",psibling->pid);
                    strcat(children,psibling->comm);
                    strcat(children,ppid);
                    strcat(children,"\n");
                    strcat(children,"    ");
                    //    			printk("        %s(%d)\n", psibling->comm, psibling->pid);
                    memcpy(NLMSG_DATA(nlh),children,sizeof(children));
                }
            }
        }
    }
    netlink_unicast(nl_sk,skb,pid,MSG_DONTWAIT);
}

void nl_data_ready(struct sk_buff *__skb)
{
    struct sk_buff *skb;
    struct nlmsghdr *nlh;
    char str[100];

    skb = skb_get (__skb);
    if(skb->len >= NLMSG_SPACE(0)) {
        nlh = nlmsg_hdr(skb);
        memcpy(str, NLMSG_DATA(nlh), sizeof(str));
        //printk("Message received:%s\n",str) ;
        sendnlmsg(nlh->nlmsg_pid, nlh->nlmsg_seq);
        kfree_skb(skb);
    }
}

static int netlink_unicast_init(void)
{
    struct netlink_kernel_cfg netlink_kerncfg = {
        .input = nl_data_ready,
    };
    nl_sk = netlink_kernel_create(&init_net, NETLINK_TEST, &netlink_kerncfg);

    if(!nl_sk) {
        printk(KERN_ERR "netlink_unicast_init: Create netlink socket error.\n");
        return -1;
    }
    printk("netlink_unicast_init: Create netlink socket ok.\n");
    return 0;
}

static void netlink_unicast_exit(void)
{
    if(nl_sk != NULL) {
        sock_release(nl_sk->sk_socket);
    }
    printk("netlink_unicast_exit!\n");
}

module_init(netlink_unicast_init);
module_exit(netlink_unicast_exit);
//MODULE_AUTHOR("X-SLAM XINU");
MODULE_LICENSE("GPL");
