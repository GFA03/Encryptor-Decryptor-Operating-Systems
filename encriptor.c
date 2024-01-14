#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define BUFFER_SIZE 4096

void openFile(char *fileName, int flags, mode_t mode, int *fd)
{
    *fd = open(fileName, flags, mode);
    if (*fd < 0)
    {
        perror("Error opening the file");
        exit(1);
    }
}

void createOutputFile(char *fileName, int *fd)
{
    *fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (*fd < 0)
    {
        perror("Error opening the destination file");
        close(*fd);
        exit(1);
    }
}


void *mapFileIntoMemory(int fd, off_t *fileSize)
{
    struct stat fileStat;
    if (fstat(fd, &fileStat) < 0)
    {
        perror("Error getting file size");
        exit(1);
    }

    *fileSize = fileStat.st_size;

    void *mappedFile = mmap(NULL, *fileSize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mappedFile == MAP_FAILED)
    {
        perror("Error mapping file into memory");
        exit(1);
    }

    return mappedFile;
}

void processMappedFile(void *mappedFile, off_t fileSize)
{
    char *fileContent = (char *)mappedFile;

    for (off_t i = 0; i < fileSize; ++i)
    {
        putchar(fileContent[i]);
    }
}

void unmapFileFromMemory(void *mappedFile, off_t fileSize)
{
    if (munmap(mappedFile, fileSize) < 0)
    {
        perror("Error unmapping file from memory");
        exit(1);
    }
}

void closeFiles(int inputFd, int outputFd)
{
    if (close(inputFd) < 0)
    {
        perror("Error closing the file");
        exit(1);
    }

    if (close(outputFd) < 0)
    {
        perror("Error closing the file");
        exit(1);
    }
}

int main(int argc, char** argv)
{
    // flag for encryption or decryption (0 for decryption, 1 for encryption)
    int encryptFlag;
    if (argc == 2)
    {
        encryptFlag = 1;   
    }
    else if (argc == 3)
    {
        encryptFlag = 0;
    }
    else
    {
        fprintf(stderr, "There are two types of usage!\n1. Encryptor:\t Usage: %s <input_file>\n2. Decryptor:\t Usage: %s <encrypted_words_file> <permutations_file>\n", argv[0], argv[0]);
        return 1;
    }

    char *inputFile = argv[1];
    char *outputFile = "output.txt";
    char *permutationsFile;
    int inputFd, outputFd;
    char *line = NULL;
    ssize_t bytesRead, bytesWritten;
    off_t fileSize;

    if(encryptFlag == 0)
    {
        permutationsFile = argv[2];
        return 0;
    }
    else
    {
        openFile(inputFile, O_RDONLY, S_IRUSR, &inputFd);
        createOutputFile(outputFile, &outputFd);
        
        void *mappedFile = mapFileIntoMemory(inputFd, &fileSize);

        processMappedFile(mappedFile, fileSize);

        unmapFileFromMemory(mappedFile, fileSize);

        closeFiles(inputFd, outputFd);
    }

    return 0;
}