#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
bool isFile(char *filePath)
{
    WIN32_FIND_DATA findFileDate;

    FindFirstFile(filePath, &findFileDate);
    if (findFileDate.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        return false;
    }
    else
        return true;
}
void copyFile(char *sourcePath, char *objectPath)
{
    WIN32_FIND_DATA findFileData;
    FindFirstFile(sourcePath, &findFileData);
    strcat(objectPath, "\\");
    strcat(objectPath, (const char *)(findFileData.cFileName));
    HANDLE hSourceFile = CreateFile(sourcePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    HANDLE hObjectFile = CreateFile(objectPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hObjectFile == INVALID_HANDLE_VALUE)
    {
        printf("文件 %s 已存在\n", sourcePath);
    }
    else
    {
        long size = GetFileSize(hSourceFile, NULL);
        char *buffer = new char[size];
        DWORD tmp;
        if (!ReadFile(hSourceFile, buffer, size, &tmp, NULL))
        {
            printf("读取源文件失败:%d\n", GetLastError());
            exit(0);
        }
        if (!WriteFile(hObjectFile, buffer, size, &tmp, NULL))
        {
            printf("写入文件失败\n");
            exit(0);
        }
        else
        {
            printf("文件 %s 已复制...\n", sourcePath);
        }
        FILETIME lpCreationTime, lpLastAccessTime, lpLastWriteTime;
        GetFileTime(hSourceFile, &lpCreationTime, &lpLastAccessTime, &lpLastWriteTime);
        SetFileTime(hObjectFile, &lpCreationTime, &lpLastAccessTime, &lpLastWriteTime);
        DWORD attr = GetFileAttributes(sourcePath);
        if (attr != INVALID_FILE_ATTRIBUTES)
            SetFileAttributes(objectPath, attr);
        else
            printf("设置文件属性无效\n");
        delete[] buffer;
    }

    CloseHandle(hSourceFile);
    CloseHandle(hObjectFile);
}
void copyDirectory(char *sourcePath, char *objectPath)
{

    WIN32_FIND_DATA findFileData;
    char *source = new char[1000];
    strcpy(source, sourcePath);
    strcat(source, "\\*.*");
    char *object = new char[1000];
    strcpy(object, objectPath);
    HANDLE hFilePath = FindFirstFile(source, &findFileData);
    strcpy(source, sourcePath);

    if (hFilePath == INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFilePath);
        return;
    }
    else
    {
        do
        {
            if (strcmp((const char *)findFileData.cFileName, ".") != 0 && strcmp((const char *)findFileData.cFileName, "..") != 0)
            {
                strcat(source, "\\");
                strcat(source, (const char *)(findFileData.cFileName));

                if (isFile(source))
                {

                    copyFile(source, object);
                }
                else
                {
                    char *tmp_s = new char[1000];
                    char *tmp_o = new char[1000];

                    strcat(object, "\\");
                    strcat(object, (const char *)(findFileData.cFileName));
                    strcpy(tmp_s, source);
                    strcpy(tmp_o, object);
                    printf("目录 %s 已创建...\n", object);

                    printf("tmp_s:%s\n", tmp_s);
                    printf("tmp_0:%s\n", tmp_o);
                    CreateDirectory((object), NULL);
                    copyDirectory(source, object);
                    strcpy(object, objectPath);
                    strcat(object, "\\");
                    strcpy(source, sourcePath);

                    HANDLE hSourceFile = CreateFile(tmp_s, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_FLAG_BACKUP_SEMANTICS, NULL);
                    HANDLE hObjectFile = CreateFile(tmp_o, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_FLAG_BACKUP_SEMANTICS, NULL);

                    FILETIME lpCreationTime, lpLastAccessTime, lpLastWriteTime;
                    GetFileTime(hSourceFile, &lpCreationTime, &lpLastAccessTime, &lpLastWriteTime);
                    SetFileTime(hObjectFile, &lpCreationTime, &lpLastAccessTime, &lpLastWriteTime);
                    CloseHandle(hSourceFile);
                    CloseHandle(hObjectFile);
                    delete[] tmp_s;
                    delete[] tmp_o;
                }
            }
        } while (FindNextFile(hFilePath, &findFileData));
    }

    delete[] source;
    delete[] object;
    CloseHandle(hFilePath);
}
int main(int argc, char *argv[])
{

    WIN32_FIND_DATA findFileData;
    WIN32_FIND_DATA findFileData_;
    char *tmp_S = new char[1000];
    char *tmp_O = new char[1000];
    strcpy(tmp_S, argv[1]);
    strcpy(tmp_O, argv[2]);
    //判断原文件和目标目录是否存在
    if (FindFirstFile(argv[1], &findFileData) == INVALID_HANDLE_VALUE)
    {
        printf("源文件不存在\n");
    }
    if (FindFirstFile(argv[2], &findFileData_) == INVALID_HANDLE_VALUE)
    {
        printf("目标路径不存在\n");
        if (CreateDirectory(argv[2], NULL))
        {
            printf("已创建目标文件夹\n");
        }
    }
    if (isFile(argv[1]))
    { //文件
        copyFile(argv[1], argv[2]);
    }
    else
    {

        strcat(argv[2], "\\");
        strcat(argv[2], (const char *)findFileData.cFileName);
        CreateDirectory(argv[2], NULL);
        printf("目录 %s 已创建...\n", argv[2]);
        char *tmp_s = new char[1000];
        char *tmp_o = new char[1000];
        strcpy(tmp_s, argv[1]);
        strcpy(tmp_o, argv[2]);
        copyDirectory(argv[1], argv[2]);

        HANDLE hSourceFile = CreateFile(tmp_s, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_FLAG_BACKUP_SEMANTICS, NULL);
        HANDLE hObjectFile = CreateFile(tmp_o, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS,FILE_FLAG_BACKUP_SEMANTICS, NULL);

        FILETIME lpCreationTime, lpLastAccessTime, lpLastWriteTime;
        GetFileTime(hSourceFile, &lpCreationTime, &lpLastAccessTime, &lpLastWriteTime);
        SetFileTime(hObjectFile, &lpCreationTime, &lpLastAccessTime, &lpLastWriteTime);
        CloseHandle(hSourceFile);
        CloseHandle(hObjectFile);
        delete[] tmp_o;
        delete[] tmp_s;
    }
    HANDLE hSourceFile = CreateFile(tmp_S, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    HANDLE hObjectFile = CreateFile(tmp_O, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_FLAG_BACKUP_SEMANTICS, NULL);

    FILETIME lpCreationTime, lpLastAccessTime, lpLastWriteTime;
    GetFileTime(hSourceFile, &lpCreationTime, &lpLastAccessTime, &lpLastWriteTime);
    SetFileTime(hObjectFile, &lpCreationTime, &lpLastAccessTime, &lpLastWriteTime);
    CloseHandle(hSourceFile);
    CloseHandle(hObjectFile);

    delete[] tmp_S;
    delete[] tmp_O;
    return 0;
}