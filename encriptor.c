#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

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

ssize_t readFromFile(int* fd, char *buffer, int size)
{
    ssize_t bytesRead = read(*fd, buffer, size);
    if (bytesRead < 0)
    {
        perror("Error reading from the source file");
        exit(1);
    }
    return bytesRead;
}

ssize_t writeToFile(int* fd, char *buffer, int size)
{
    ssize_t bytesWritten = write(*fd, buffer, size);
    if (bytesWritten < 0)
    {
        perror("Error writing to the destination file");
        exit(1);
    }
    return bytesWritten;
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
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead, bytesWritten;

    if(encryptFlag == 0)
    {
        permutationsFile = argv[2];
        return 0;
    }
    else
    {
        openFile(inputFile, O_RDONLY, S_IRUSR, &inputFd);
        createOutputFile(outputFile, &outputFd);
        bytesRead = readFromFile(&inputFd, buffer, sizeof(buffer));
        while(bytesRead > 0)
        {
            bytesWritten = writeToFile(&outputFd, buffer, bytesRead);
            bytesRead = readFromFile(&inputFd, buffer, sizeof(buffer));
        }
        if(bytesWritten < 0)
        {
            perror("Error writing to the destination file");
            closeFiles(inputFd, outputFd);
            return 1;
        }
        if(bytesRead == 0)
        {
            closeFiles(inputFd, outputFd);
        }
    }

    // struct stat sb;
    // if (stat(inputFile, &sb))
    // {
    //     perror(inputFile);
    //     return errno;
    // }


    // ssize_t bytesRead, bytesWritten;

    // while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0)
    // {
    //     bytesWritten = write(new, buffer, bytesRead);

    //     if (bytesWritten < 0)
    //     {
    //         perror("Error writing to the destination file");
    //         close(fd);
    //         close(new);
    //         return 1;
    //     }
    // }

    // if (bytesRead < 0)
    // {
    //     perror("Error reading from the source file");
    //     close(fd);
    //     close(new);
    //     return 1;
    // }

    // close(fd);
    // close(new);

    return 0;
}