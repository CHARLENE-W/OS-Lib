#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <utime.h>
#include <unistd.h>
#include <sys/time.h>
#include <dirent.h>
void getFileName(char *path, char *objectFile)
{
    int mark = 0;
    for (int i = 0; path[i] != '\0'; i++)
    {
        if (path[i] == '/')
            mark = i;
    }
    if (mark != 0)
    {
        mark++;
    }

    path = path + mark;
    strcat(objectFile, "/");
    strcat(objectFile, path);
}

void copyFile(char *sourceFile, char *objectFile)
{
    getFileName(sourceFile, objectFile);
    struct stat buf;
    stat(sourceFile, &buf);
    long size = 1000;
    int fd_in = open(sourceFile, O_RDONLY);
    if (fd_in == -1)
    {
        printf("打开文件失败\n");
        _exit(0);
    }

    int fd_out = open(objectFile, O_CREAT | O_TRUNC | O_WRONLY, buf.st_mode);
    if (fd_out == -1)
    {
        printf("创建文件失败\n");
        _exit(0);
    }

    char *buffer = new char[1000];
    long num;

    do
    {
        num = read(fd_in, buffer, size);
        if (num == -1)
        {
            printf("读取文件失败\n");
            _exit(0);
        }
        write(fd_out, buffer, num);

    } while (num > 0);
    delete[] buffer;
    printf("文件： %s 已创建...\n", objectFile);
    struct utimbuf timebuf;
    timebuf.actime = buf.st_atime;
    timebuf.modtime = buf.st_mtime;
    utime(objectFile, &timebuf);
    chmod(objectFile, buf.st_mode);
    close(fd_in);
    close(fd_out);
}
void copyLink(char *sourceFile, char *objectFile)
{
    struct stat buf;
    lstat(sourceFile, &buf);
    char *src_buf = new char[2000];
    readlink(sourceFile, src_buf, 2000);
    getFileName(sourceFile, objectFile);
    if (symlink(src_buf, objectFile) == -1)
    {
        printf("创建软链接失败\n");
        _exit(0);
    }

    printf("链接： %s 已创建...\n", objectFile);
    chmod(objectFile, buf.st_mode);
    struct timeval timebuf[2]; //utimes需要使用timeval数组修改时间
    timebuf[0].tv_sec = buf.st_atime;
    timebuf[0].tv_usec = 0;
    timebuf[1].tv_sec = buf.st_mtime;
    timebuf[1].tv_usec = 0;
    lutimes(objectFile, timebuf);
    delete[] src_buf;
}
void copyDir(char *sourceFile, char *objectFile)
{
    struct stat buf;
    DIR *dir = opendir(sourceFile);
    if (dir == NULL)
    {
        printf("打开文件目录失败\n");
        _exit(0);
    }
    struct dirent *dirent = readdir(dir);
    do
    {
        if (dirent->d_type == DT_DIR)
        {
            if (strcmp(dirent->d_name, ".") != 0 && strcmp(dirent->d_name, "..") != 0)
            {
                //若既不是当前目录也不是父目录,则创建新目录
                char *tmp_o = new char[1000];
                char *tmp_s = new char[1000];
                char *tmp = new char[1000];
                strcpy(tmp_o, objectFile);
                strcat(tmp_o, "/");
                strcat(tmp_o, dirent->d_name);
                mkdir(tmp_o, 0777);
                printf("目录：%s 已创建...\n", tmp_o);
                strcpy(tmp_s, sourceFile);
                strcat(tmp_s, "/");
                strcat(tmp_s, dirent->d_name);
                stat(tmp_s, &buf);
                struct utimbuf *timebuf = new struct utimbuf;
                timebuf->actime = buf.st_atime;
                timebuf->modtime = buf.st_mtime;

                strcpy(tmp, tmp_o);

                copyDir(tmp_s, tmp_o);

                utime(tmp, timebuf);

                chmod(tmp_o, buf.st_mode);

                delete timebuf;
                delete[] tmp_o;
                delete[] tmp_s;
                delete[] tmp;
            }
        }
        else if (dirent->d_type == DT_REG)
        {

            char *tmp_s = new char[1000];
            strcpy(tmp_s, sourceFile);
            strcat(tmp_s, "/");
            strcat(tmp_s, dirent->d_name);
            copyFile(tmp_s, objectFile);
            delete[] tmp_s;
        }
        else if (dirent->d_type == DT_LNK)
        {
            char *tmp_s = new char[1000];
            strcpy(tmp_s, sourceFile);
            strcat(tmp_s, "/");
            strcat(tmp_s, dirent->d_name);
            copyLink(tmp_s, objectFile);
            delete[] tmp_s;
        }
    } while ((dirent = readdir(dir)) != NULL);
}
int main(int argc, char *argv[])
{
    char *source = new char[1000];
    char *object = new char[1000];

    char *object_ = new char[1000];
    strcpy(source, argv[1]);
    strcpy(object, argv[2]);

    struct stat buf;

    struct utimbuf *timebuf = new struct utimbuf;
     struct utimbuf *timebuf_ = new struct utimbuf;
    lstat(source, &buf);
    timebuf->actime = buf.st_atime;
    timebuf->modtime = buf.st_mtime;
     timebuf_->actime = buf.st_atime;
    timebuf_->modtime = buf.st_mtime;
    DIR *dir;
    if ((dir = opendir(object)) == NULL)
    {

        mkdir(object, 0777);

        chmod(object, 0777);
    }
    closedir(dir);
    if (S_ISREG(buf.st_mode))
    {
        //文件

        copyFile(source, object);
    }
    else if (S_ISLNK(buf.st_mode))
    {
        //链接

        copyLink(source, object);
    }
    else if (S_ISDIR(buf.st_mode))
    {
        //目录

        getFileName(source, object);
        strcpy(object_, object);
        mkdir(object, buf.st_mode);
        printf("目录：%s 已创建...\n", object);
        chmod(object, buf.st_mode);
        copyDir(source, object);
        
        utime(object_, timebuf);
        delete timebuf;
    }
    utime(argv[2], timebuf_);
    printf("复制已完成\n");
    delete[] source;
    delete[] object;
     delete timebuf_;
    return 0;
}