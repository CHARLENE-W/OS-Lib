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
        printf("�ļ� %s �Ѵ���\n", sourcePath);
    }
    else
    {
        long size = GetFileSize(hSourceFile, NULL);
        char *buffer = new char[size];
        DWORD tmp;
        if (!ReadFile(hSourceFile, buffer, size, &tmp, NULL))
        {
            printf("��ȡԴ�ļ�ʧ��:%d\n", GetLastError());
            exit(0);
        }
        if (!WriteFile(hObjectFile, buffer, size, &tmp, NULL))
        {
            printf("д���ļ�ʧ��\n");
            exit(0);
        }
        else
        {
            printf("�ļ� %s �Ѹ���...\n", sourcePath);
        }
        FILETIME lpCreationTime, lpLastAccessTime, lpLastWriteTime;
        GetFileTime(hSourceFile, &lpCreationTime, &lpLastAccessTime, &lpLastWriteTime);
        SetFileTime(hObjectFile, &lpCreationTime, &lpLastAccessTime, &lpLastWriteTime);
        DWORD attr=GetFileAttributes(sourcePath);
        if(attr!=INVALID_FILE_ATTRIBUTES)
        SetFileAttributes(objectPath,attr);
        else printf("�����ļ�������Ч\n");
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
            if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0)
            {
                strcat(source, "\\");
                strcat(source, (const char *)(findFileData.cFileName));

                if (isFile(source))
                {

                    copyFile(source, object);
                }
                else
                {
                    strcat(object, "\\");
                    strcat(object, (const char *)(findFileData.cFileName));
                    printf("Ŀ¼ %s �Ѵ���...\n", object);
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
    CloseHandle(hFilePath);
}
int main(int argc, char *argv[])
{

    WIN32_FIND_DATA findFileData;
    WIN32_FIND_DATA findFileData_;
    //�ж�ԭ�ļ���Ŀ��Ŀ¼�Ƿ����
    if (FindFirstFile(argv[1], &findFileData) == INVALID_HANDLE_VALUE)
    {
        printf("Դ�ļ�������\n");
    }
    if (FindFirstFile(argv[2], &findFileData_) == INVALID_HANDLE_VALUE)
    {
        printf("Ŀ��·��������\n");
        if (CreateDirectory(argv[2], NULL))
        {
            printf("�Ѵ���Ŀ���ļ���\n");
        }
    }
    if (isFile(argv[1]))
    { //�ļ�
        copyFile(argv[1], argv[2]);
    }
    else
    {

        strcat(argv[2], "\\");
        strcat(argv[2], findFileData.cFileName);
        CreateDirectory(argv[2], NULL);
        printf("Ŀ¼ %s �Ѵ���...\n", argv[2]);
        copyDirectory(argv[1], argv[2]);
    }

    return 0;
}