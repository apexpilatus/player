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

#include <FLAC/metadata.h>

#define listen_port 9696
#define picture_getter_name "picture"

static volatile FLAC__uint32 *length;
static int length_size = sizeof(FLAC__uint32);
static char *data_addr;
static int data_size;
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
                strcat(data_addr, "/");
                strcat(data_addr, ep->d_name);
                break;
            }
        }
        (void)closedir(dp);
    }
    printf("%s\n", data_addr);
    pid_t handl_pid = fork();
    if (!handl_pid)
    {
        execl(exec_picture_getter_path, picture_getter_name, NULL);
    }
    int handl_status;
    if (handl_pid > 0)
    {
        waitpid(handl_pid, &handl_status, 0);
    }
    printf("%d\n", handl_status);
    if (!handl_status)
    {
        printf("handler ok\n");
        FILE *fl = fopen(picture_path, "r");
        if (fl)
        {
            printf("file ok\n");
            write(sock, (int *)length, sizeof(FLAC__uint32));
            read_size = read(sock, data_addr, data_size);
            data_addr[read_size] = '\0';
            if (!strcmp(data_addr, "ok"))
            {
                char buf[*length];
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
    ssize_t read_size;
    read_size = read(sock, data_addr, data_size);
    data_addr[read_size] = '\0';
    /*FLAC__StreamMetadata *tags = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
    FLAC__metadata_get_tags(str, &tags);
    for (int i = 0; i < tags->data.vorbis_comment.num_comments; i++)
    {
        write(sock, tags->data.vorbis_comment.comments[i].entry, tags->data.vorbis_comment.comments[i].length);
        write(sock, "\n", 1);
    }*/

    write(sock, "ARTIST=tururu\n", 14);
    write(sock, "TRACKNUMBER=3\n", 14);
    write(sock, "TITLE=tururu\n", 13);

    write(sock, "&end_tags\n", 10);
    /*FLAC__metadata_object_delete(tags);
    FLAC__StreamMetadata *rate = FLAC__metadata_object_new(FLAC__METADATA_TYPE_STREAMINFO);
    FLAC__metadata_get_streaminfo(str, rate);
    sprintf(str, "%u/%g%c", rate->data.stream_info.bits_per_sample, rate->data.stream_info.sample_rate / 1000.0, '\n');*/
    write(sock, data_addr, data_size);
    /*FLAC__metadata_object_delete(rate);*/
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
    length = shd_addr;
    data_addr = (char *)shd_addr + length_size;
    data_size = page_size - length_size;
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
