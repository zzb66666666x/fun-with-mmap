#include <iostream>
#include <fstream>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// void createSampleFile(const std::string& filename, const std::string& content) {
//     std::ofstream file(filename);
//     if (file.is_open()) {
//         file << content;
//         file.close();
//     }
// }

int main() {
    const std::string filename = "sample.txt";
    // const std::string initialContent = "Hello, mmap!";
    // createSampleFile(filename, initialContent);

    // Open file
    int fd = open(filename.c_str(), O_RDWR);
    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    // Get file size
    struct stat fileInfo;
    if (fstat(fd, &fileInfo) == -1) {
        perror("Error getting the file size");
        close(fd);
        return 1;
    }

    // Memory map the file
    char* data = static_cast<char*>(mmap(NULL, fileInfo.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (data == MAP_FAILED) {
        perror("Error mmapping the file");
        close(fd);
        return 1;
    }

    // Read from the file
    std::cout << "Original file content: " << std::string(data, fileInfo.st_size) << std::endl;

    // Modify the file (example: change 'mmap' to 'MMAP')
    for (size_t i = 0; i < fileInfo.st_size; ++i) {
        if (data[i] == 'm') {
            data[i] = 'M';
        }
        if (data[i] == '\n'){
            std::cout<<"new line"<<std::endl;
        }
    }

    // Unmap and close file
    if (munmap(data, fileInfo.st_size) == -1) {
        perror("Error un-mmapping the file");
    }
    close(fd);

    return 0;
}