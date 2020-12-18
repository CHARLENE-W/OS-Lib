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

    HANDLE hObjectFile = CreateFile(objectPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hObjectFile == INVALID_HANDLE_VALUE)
    {
        printf("创建文件失败\n");
        exit(0);
    }
    long size = GetFileSize(hSourceFile, NULL);
    printf("size=%ld\n", size);
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
    delete[] buffer;
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
    //   printf("1:%s\n", findFileData.cFileName);
    if (hFilePath == INVALID_HANDLE_VALUE)
    {
        return;
    }
    else
    {
        do
        {
            if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0)
            {
                strcat(source, "\\");
                strcat(source, (const char *)(findFileData.cFileName));
                printf("source: %s\n", source);
                if (isFile(source))
                {
                    printf("source: %s  object: %s               begin to copy file\n", source, object);
                    copyFile(source, object);
                }
                else
                {

                    strcat(object, "\\");
                    strcat(object, (const char *)(findFileData.cFileName));
                    printf("source: %s           object: %s\n", source, object);
                    CreateDirectory(object, NULL);
                      copyDirectory(source, object);

                    strcpy(object, objectPath);
                    strcat(object, "\\");
                     strcpy(source, sourcePath);
                }
            }
        } while (FindNextFile(hFilePath, &findFileData));
    }
    delete[] source;
    delete[] object;
}
int main(int argc, char *argv[])
{
    // char a[1000];
    // char b[1000];
    // strcpy(a,"..\\test_for_cp");
    // strcpy(b, "..\\text2314");
    WIN32_FIND_DATA findFileData;
    WIN32_FIND_DATA findFileData_;
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
        printf("debug1+++++++++++++\n");
        strcat(argv[2], "\\");
        strcat(argv[2], findFileData.cFileName);
        CreateDirectory(argv[2], NULL);
        copyDirectory(argv[1], argv[2]);
    }
}