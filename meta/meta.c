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

#define listen_port 9696
#define picture_getter_name "picture"
#define tags_getter_name "tags"

static ssize_t read_size;
static char *dirs[] = {
    "/home/store/music/dzr",
    "/home/store/music/qbz",
    "/home/store/music/hack1",
    "/home/store/music/hack2",
    "/home/store/music/hack3",
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
        FILE *fl = fopen(picture_path, "r");
        if (fl)
        {
            write(sock, (int *)picture_length, sizeof(FLAC__uint32));
            read_size = read(sock, data_addr, data_size);
            data_addr[read_size] = '\0';
            if (!strcmp(data_addr, "ok"))
            {
                char buf[*picture_length];
                fread(buf, 1, sizeof(buf), fl);
                write(sock, buf, sizeof(buf));
                read_size = read(sock, data_addr, data_size);
                data_addr[read_size] = '\0';
            }
            fclose(fl);
        }
        else
        {
            write(sock, "!", 1);
            read_size = read(sock, data_addr, data_size);
            data_addr[read_size] = '\0';
        }
    }
    else
    {
        write(sock, "!", 1);
        read_size = read(sock, data_addr, data_size);
        data_addr[read_size] = '\0';
    }
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
        while ((ep = readdir(dp)))
        {
            if (ep->d_type == DT_REG)
            {
                write(sock, ep->d_name, strlen(ep->d_name));
                write(sock, "\n", 1);
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
                    waitpid(handl_pid, &handl_status, 0);
                }
                if (!handl_status)
                {
                    char *str = data_addr;
                    for (int i = 0; i < *num_comments + 1; i++)
                    {
                        str = str + strlen(str) + 1;
                        write(sock, str, strlen(str));
                        write(sock, "\n", 1);
                    }
                    write(sock, "&end_tags\n", 10);
                }
            }
        }
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
    int shd = shm_open(shm_file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (shd < 0)
    {
        return 1;
    }
    int page_size = getpagesize();
    if (ftruncate(shd, page_size))
    {
        return 1;
    }
    void *shd_addr = mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, shd, 0);
    if (shd_addr == MAP_FAILED)
    {
        return 1;
    }
    set_shm_addr();
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
    if (listen(sock_listen, 2) < 0)
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
