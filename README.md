# Encrypter/Decrypter Project

This project is a simple encryption and decryption tool designed to process text files. It utilizes a permutation-based approach to provide a reversible transformation of the input text.

## Features

- **Encryption:** The program shuffles the words in the input file using the Fisher-Yates shuffle algorithm, creating an encrypted version.
- **Decryption:** The decryption process reverses the permutation, restoring the original text from the encrypted file.

## Usage

### Compiling the solution

To compile the program, use the following command:
```bash
gcc encriptor.c -o encriptor
```

### Encrypting a File

To encrypt a file, use the following command:

```bash
./encriptor input.txt
```

The encrypted words will be written to `output.txt`.
The permutations of the words will be written to `permutations.txt`.

### Decrypting a File

To decrypt a file, use the following command:

```bash
./encriptor input.txt permutations.txt
```

The initial words will be written to `output.txt`.

## Input and Permutation Files

The decryption process involves two key files:

- `input.txt`: This file should contain the words to be decrypted.


- `permutations.txt`: This file stores the permutations generated during the encryption. Each line corresponds to a word in the `input.txt` file, and the numbers represent the indices after permutation.


Ensure that the order of lines in `permutations.txt` matches the order of words in `input.txt`.
