#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>

#define listen_port 9696

char *dirs[] = {
    "/home/store/music/dzr",
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
            while ((ep = readdir(dp)))
            {
                if (ep->d_type == DT_DIR && strcmp(ep->d_name, ".") && strcmp(ep->d_name, ".."))
                {
                    printf("%s\n", ep->d_name);
                }
            }
            (void)closedir(dp);
        }
    }
    write(sock, "&the_end\n", 9);
}

static void (*action[])(int sock) = {
    meta0_get_albums};

int main(void)
{
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