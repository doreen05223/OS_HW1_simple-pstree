#include"ksimple_pstree.h"

struct sock *nl_sk = NULL;

void sendnlmsg(int pid, int choose)
{
    struct sk_buff *skb;
    struct nlmsghdr *nlh;
//	char msg[30] = "Say hello from kernel";

    if(!nl_sk) {
        return;
    }

    skb = nlmsg_new(MAX_PAYLOAD_SIZE,GFP_KERNEL);
    if(!skb) {
        printk(KERN_ERR"nlmsg_new eror!\n");
    }
    nlh = nlmsg_put(skb,0,0,0,MAX_PAYLOAD_SIZE,0);

//	memcpy(NLMSG_DATA(nlh),msg,sizeof(msg));
//	printk("Send message'%s'.\n",(char*)NLMSG_DATA(nlh));

    struct task_struct *p;
    struct list_head *pp=0;
    struct task_struct *psibling;
    char ppid[10]="";
    char parent[2048]="";
    char sibling[2048]="";
    char children[2048]="";

//INIT_LIST_HEAD(pp);

    //current processor
    p = pid_task(find_vpid(pid), PIDTYPE_PID);

    if(choose == 1) {
        if(p == NULL) memcpy(NLMSG_DATA(nlh),parent,sizeof(parent));
        else {
            //parent
            if(p->parent == NULL) {
                printk("No Parent\n");
                strcat(parent,"\n");
                strcat(parent,"    ");
            } else {
                sprintf(ppid,"(%d)",p->parent->pid);
                strcat(parent,p->parent->comm);
                strcat(parent,ppid);
                strcat(parent,"\n");
                strcat(parent,"    ");
                printk("%s(%d)\n", p->parent->comm, p->parent->pid);
            }
            //current
            sprintf(ppid,"(%d)",p->pid);
            strcat(parent,p->comm);
            strcat(parent,ppid);
            strcat(parent,"\n");
            memcpy(NLMSG_DATA(nlh),parent,sizeof(parent));
        }
    }

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
                    printk("    %s(%d)\n", psibling->comm, psibling->pid);
                    memcpy(NLMSG_DATA(nlh),sibling,sizeof(sibling));
                }
            }
        }
    }

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
                    printk("        %s(%d)\n", psibling->comm, psibling->pid);
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
        printk("Message received:%s\n",str) ;
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


