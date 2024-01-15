#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>

#define BUFFER_SIZE 4096

int isFileEmpty(int fd)
{
    // Move the file pointer to the end of the file
    if (lseek(fd, 0, SEEK_END) == 0)
    {
        // If the file pointer is at the beginning, the file is empty
        return 1; // True (file is empty)
    }
    else
    {
        // If the file pointer is not at the beginning, the file is not empty
        lseek(fd, 0, SEEK_SET); // Reset the file pointer to the beginning
        return 0; // False (file is not empty)
    }
}

void openFile(char *fileName, int flags, mode_t mode, int *fd)
{
    *fd = open(fileName, flags, mode);
    if (*fd < 0)
    {
        perror("Error opening the file");
        exit(1);
    }

    // Check if the file is empty
    if (isFileEmpty(*fd))
    {
        fprintf(stderr, "Error: The file is empty.\n");
        close(*fd);
        exit(1);
    }
}

// Creating output files
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

void permuteWord(char *word, int outputFd, int permutationFd)
{
    // Seed for random number generation
    srand(time(NULL));

    // Get the length of the word
    size_t length = strlen(word);

    // Allocate memory for the permutation array
    int *permutation = (int *)malloc(length * sizeof(int));
    if (permutation == NULL)
    {
        perror("Error allocating memory for permutation");
        exit(1);
    }

    // Initialize the permutation array with the initial positions
    for (size_t i = 0; i < length; i++)
    {
        permutation[i] = i;
    }

    // Perform Fisher-Yates shuffle on both the word and the permutation array
    for (size_t i = length - 1; i > 0; i--)
    {
        // Generate a random index within the remaining unshuffled part of the array
        size_t j = rand() % (i + 1);

        // Swap characters at indices i and j in the word
        char tempChar = word[i];
        word[i] = word[j];
        word[j] = tempChar;

        // Swap indices at positions i and j in the permutation array
        int tempIndex = permutation[i];
        permutation[i] = permutation[j];
        permutation[j] = tempIndex;
    }

    // Write the permuted word to the output file
    ssize_t bytesWritten = write(outputFd, word, length);
    if (bytesWritten < 0)
    {
        perror("Error writing to the destination file");
        exit(1);
    }
    write(outputFd, "\n", 1);

    for(size_t i = 0; i < length; i++)
    {
        char temp[100];
        sprintf(temp, "%d", permutation[i]);
        write(permutationFd, temp, strlen(temp));
    }

    write(permutationFd, "\n", 1);

    // Free the allocated memory for the permutation array
    free(permutation);
}

void processLine(char *line, int outputFd, int permutationFd)
{
    // Tokenize the line into words
    char *word = strtok(line, " \t\n"); // You might need to expand the delimiter list

    while (word != NULL)
    {
        // Fork a new process for each word
        pid_t pid = fork();

        if (pid == -1)
        {
            perror("Error forking process");
            exit(1);
        }
        else if (pid == 0) // Child process
        {
            // Permute the word
            permuteWord(word, outputFd, permutationFd);
            exit(0);
        }
        else // Parent process
        {
            // Wait for the child process to complete
            waitpid(pid, NULL, 0);
        }

        // Move to the next word
        word = strtok(NULL, " \t\n");
    }
}

void processMappedFile(void *mappedFile, off_t fileSize, int outputFd, int permutationFd)
{
    // Cast the void pointer to the appropriate data type
    char *fileContent = (char *)mappedFile;

    // Process the file content line by line
    off_t currentPosition = 0;
    while (currentPosition < fileSize)
    {
        // Find the end of the current line
        off_t lineEnd = currentPosition;
        while (lineEnd < fileSize && fileContent[lineEnd] != '\n')
        {
            lineEnd++;
        }

        // Calculate the length of the current line
        off_t lineLength = lineEnd - currentPosition;

        // Allocate memory for the line and copy it
        char *line = malloc(lineLength + 1); // +1 for null terminator
        if (line == NULL)
        {
            perror("Error allocating memory for line");
            exit(1);
        }

        strncpy(line, fileContent + currentPosition, lineLength);
        line[lineLength] = '\0'; // Null-terminate the line


        // Process the line
        processLine(line, outputFd, permutationFd);

        // Free the allocated memory for the line
        free(line);

        // Move to the next line
        currentPosition = lineEnd + 1;
    }
}

void processDecryption(void *mappedFile, off_t fileSize, void *mappedPermutationFile, off_t permutationSize, int outputFd)
{
    // Cast the void pointer to the appropriate data type
    char *fileContent = (char *)mappedFile;
    char *permutationContent = (char *)mappedPermutationFile;

    // Process the file content line by line
    off_t currentPosition = 0;
    while (currentPosition < fileSize)
    {
        // Find the end of the current line
        off_t lineEnd = currentPosition;
        while (lineEnd < fileSize && fileContent[lineEnd] != '\n')
        {
            lineEnd++;
        }

        // Calculate the length of the current line
        off_t lineLength = lineEnd - currentPosition;

        // Allocate memory for the line and copy it
        char *line = malloc(lineLength + 1); // +1 for null terminator
        if (line == NULL)
        {
            perror("Error allocating memory for line");
            exit(1);
        }

        strncpy(line, fileContent + currentPosition, lineLength);
        line[lineLength] = '\0'; // Null-terminate the line

        char *lineCopy = malloc(lineLength + 1);

        lineCopy = strcpy(lineCopy, line);

        // Process the line
        for(int i = 0; i < strlen(line); i++)
        {
            int index = permutationContent[i + currentPosition] - '0';
            line[index] = lineCopy[i];
        }

        // Write the decrypted word to the output file
        ssize_t bytesWritten = write(outputFd, line, lineLength);
        if (bytesWritten < 0)
        {
            perror("Error writing to the destination file");
            exit(1);
        }
        write(outputFd, "\n", 1);

        // Free the allocated memory for the line
        free(line);

        // Move to the next line
        currentPosition = lineEnd + 1;
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

void closeFile(int fd)
{
    if (close(fd) < 0)
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

    // The input file cannot be called output.txt
    char *outputFile = "output.txt";
    char *permutationsFile = "permutations.txt";
    int inputFd, outputFd, permutationFd;
    char *line = NULL;
    ssize_t bytesRead, bytesWritten;
    off_t fileSize, permutationSize;

    if(encryptFlag == 0)
    {
        permutationsFile = argv[2];
        openFile(inputFile, O_RDONLY, S_IRUSR, &inputFd);
        openFile(permutationsFile, O_RDONLY, S_IRUSR, &permutationFd);
        createOutputFile(outputFile, &outputFd);

        void *mappedFile = mapFileIntoMemory(inputFd, &fileSize);
        void *mappedPermutationFile = mapFileIntoMemory(permutationFd, &permutationSize);

        processDecryption(mappedFile, fileSize, mappedPermutationFile, permutationSize, outputFd);

        unmapFileFromMemory(mappedFile, fileSize);
        unmapFileFromMemory(mappedPermutationFile, permutationSize);

        closeFile(inputFd);
        closeFile(permutationFd);
        closeFile(outputFd);
        return 0;
    }
    else
    {
        openFile(inputFile, O_RDONLY, S_IRUSR, &inputFd);
        createOutputFile(outputFile, &outputFd);
        createOutputFile(permutationsFile, &permutationFd);
        
        void *mappedFile = mapFileIntoMemory(inputFd, &fileSize);

        processMappedFile(mappedFile, fileSize, outputFd, permutationFd);

        unmapFileFromMemory(mappedFile, fileSize);

        closeFile(inputFd);
        closeFile(outputFd);
        closeFile(permutationFd);
    }

    return 0;
}