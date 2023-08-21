#define _GNU_SOURCE

#include "shares.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <sys/mman.h>
#include <dirent.h>
#include <sched.h>

#define listen_port 9696
#define picture_getter_name "picture"
#define tags_getter_name "tags"

static char *dirs[] = {
    "/home/music/dzr",
    "/home/music/qbz",
    "/home/music/hack1",
    "/home/music/hack2",
    "/home/music/hack3",
    NULL};

static void meta0_get_albums(int sock)
{
    for (int i = 0; dirs[i]; i++)
    {
        DIR *dp;
        struct dirent *ep;
        dp = opendir(dirs[i]);
        if (dp != NULL)
        {
            write(sock, dirs[i], strlen(dirs[i]));
            write(sock, "\n", 1);
            while ((ep = readdir(dp)))
            {
                if (ep->d_type == DT_DIR && strcmp(ep->d_name, ".") && strcmp(ep->d_name, ".."))
                {
                    write(sock, ep->d_name, strlen(ep->d_name));
                    write(sock, "\n", 1);
                }
            }
            write(sock, "&end_folder\n", 12);
            (void)closedir(dp);
        }
    }
    write(sock, "&the_end\n", 9);
}

static void meta1_get_picture(int sock)
{
    pid_t handl_pid;
    int handl_status;
    ssize_t read_size;
    read_size = read(sock, data_addr, data_size);
    data_addr[read_size] = '\0';
    DIR *dp;
    struct dirent *ep;
    dp = opendir(data_addr);
    if (dp != NULL)
    {
        while ((ep = readdir(dp)))
        {
            if (ep->d_type == DT_REG)
            {
                data_addr[read_size] = '\0';
                strcat(data_addr, "/");
                strcat(data_addr, ep->d_name);
                handl_pid = fork();
                if (!handl_pid)
                {
                    execl(picture_getter_path, picture_getter_name, NULL);
                }
                if (handl_pid > 0)
                {
                    waitpid(handl_pid, &handl_status, 0);
                }
                if (!handl_status)
                {
                    break;
                }
            }
        }
        (void)closedir(dp);
    }
    if (!handl_status)
    {
        write(sock, (int *)length, sizeof(FLAC__uint32));
        read_size = read(sock, data_addr + shm_size() - 4, 3);
        data_addr[shm_size() - 4 + read_size] = '\0';
        write(sock, data_addr, *length);
    }
    else
    {
        write(sock, "!", 1);
    }
    read_size = read(sock, data_addr, data_size);
    data_addr[read_size] = '\0';
}

static inline void send_tags(int sock)
{
    char *str_dst = data_addr_half;
    for ((*length_half)++; *length_half > 0; (*length_half)--)
    {
        write(sock, str_dst, strlen(str_dst));
        write(sock, "\n", 1);
        str_dst = str_dst + strlen(str_dst) + 1;
    }
    write(sock, "&end_tags\n", 10);
}

static void meta2_get_tags(int sock)
{
    pid_t handl_pid;
    int handl_status;
    ssize_t read_size;
    read_size = read(sock, data_addr, data_size);
    data_addr[read_size] = '\0';
    DIR *dp;
    struct dirent *ep;
    dp = opendir(data_addr);
    if (dp != NULL)
    {
        *length_half = 0;
        while ((ep = readdir(dp)))
        {
            if (ep->d_type == DT_REG)
            {
                data_addr[read_size] = '\0';
                strcat(data_addr, "/");
                strcat(data_addr, ep->d_name);
                handl_pid = fork();
                if (!handl_pid)
                {
                    execl(tags_getter_path, tags_getter_name, NULL);
                }
                if (handl_pid > 0)
                {
                    if (*length_half)
                    {
                        send_tags(sock);
                    }
                    waitpid(handl_pid, &handl_status, 0);
                }
                if (!handl_status)
                {
                    char *str_src = data_addr;
                    char *str_dst = data_addr_half;
                    strcpy(str_dst, ep->d_name);
                    *length_half = ++(*length);
                    for (; *length > 0; (*length)--)
                    {
                        str_src = str_src + strlen(str_src) + 1;
                        str_dst = str_dst + strlen(str_dst) + 1;
                        strcpy(str_dst, str_src);
                    }
                }
            }
        }
        send_tags(sock);
        write(sock, "&the_end\n", 9);
        (void)closedir(dp);
    }
}

static void (*action[])(int sock) = {
    meta0_get_albums,
    meta1_get_picture,
    meta2_get_tags};

int main(void)
{
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(0, &cpu_set);
    sched_setaffinity(getpid(), sizeof(cpu_set), &cpu_set);
    int shd = shm_open(shm_file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (shd < 0)
    {
        return 1;
    }
    if (ftruncate(shd, shm_size()))
    {
        return 1;
    }
    void *shd_addr = mmap(NULL, shm_size(), PROT_READ | PROT_WRITE, MAP_SHARED, shd, 0);
    if (shd_addr == MAP_FAILED)
    {
        return 1;
    }
    set_shm_vars();
    int sock_listen, sock;
    sock_listen = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(listen_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    socklen_t addr_size = sizeof(addr);
    if (bind(sock_listen, (struct sockaddr *)&addr, addr_size) < 0)
    {
        return 1;
    }
    if (listen(sock_listen, 1) < 0)
    {
        return 1;
    }
    while (1)
    {
        if ((sock = accept(sock_listen, (struct sockaddr *)&addr, &addr_size)) < 0)
        {
            continue;
        }
        char action_num;
        if (read(sock, &action_num, 1) > 0)
        {
            action[strtol(&action_num, NULL, 10)](sock);
        }
        close(sock);
    }
}
