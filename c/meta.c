#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>

#include <FLAC/metadata.h>

#define listen_port 9696

static char str[2048];
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
    ssize_t read_size;
    read_size = read(sock, str, sizeof(str));
    str[read_size] = '\0';
    FLAC__StreamMetadata *picture = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PICTURE);
    FLAC__metadata_get_picture(str, &picture, -1, NULL, NULL, (uint32_t)(-1), (uint32_t)(-1), (uint32_t)(-1), (uint32_t)(-1));
    sprintf(str, "%u%c", picture->data.picture.data_length, '\n');
    write(sock, str, strlen(str));
    read(sock, str, sizeof(str));
    write(sock, picture->data.picture.data, picture->data.picture.data_length);
    read(sock, str, sizeof(str));
    FLAC__metadata_object_delete(picture);
}

static void (*action[])(int sock) = {
    meta0_get_albums,
    meta1_get_picture};

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
