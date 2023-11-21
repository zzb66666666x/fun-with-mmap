#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <pthread.h>

// -lrt is needed for shm_open
// g++ mmapIPC.cpp -std=c++11 -o mmapIPC -lrt -lpthread

// with shm_unlink commented out, the shared memory object will not be removed
// use ls /dev/shm to see the shared memory objects

// to remove it, use shm_unlink("my_shared_memory") in the process 
// 

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <string>" << std::endl;
        return 1;
    }

    if (std::string(argv[1]) == "A"){
        // Create a new shared memory object
        int fd = shm_open("my_shared_memory", O_CREAT | O_RDWR, 0666);
        
        // Resize the object to hold a memory size
        int sizeof_shm = sizeof(pthread_mutex_t) + sizeof(int);
        ftruncate(fd, sizeof_shm);
        
        // Map the object into memory
        void* shared_memory_space = mmap(NULL, sizeof_shm, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        int* shared_memory = static_cast<int*>(shared_memory_space) + sizeof(pthread_mutex_t);

        // init pthread mutex
        pthread_mutex_t* mutex = static_cast<pthread_mutex_t*>(shared_memory_space);
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(mutex, &attr);
        pthread_mutexattr_destroy(&attr);

        // init the data to be 6666
        shared_memory[0] = 6666;
        
        // wait for data to be read, check per 0.1 second
        while (1){
            pthread_mutex_lock(mutex);
            if (shared_memory[0] == 0){
                pthread_mutex_unlock(mutex);
                break;
            }
            pthread_mutex_unlock(mutex);
            sleep(0.1);
        }
        // sleep for 1 second
        sleep(0.5);
        std::cout << "The shared memory has been accessed by another process" <<std::endl;
        // Cleanup: The memory should be unmapped and the shared memory object closed
        munmap(shared_memory, sizeof(int));
        close(fd);
        shm_unlink("my_shared_memory");
    }else{
        // Open the existing shared memory object
        int fd = shm_open("my_shared_memory", O_RDWR, 0666);
        
        // Map the object into memory
        void* shared_memory_space = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        // get the mutex
        pthread_mutex_t* mutex = static_cast<pthread_mutex_t*>(shared_memory_space);
        int* shared_memory = static_cast<int*>(shared_memory_space) + sizeof(pthread_mutex_t);

        // lock the mutex
        pthread_mutex_lock(mutex);
        std::cout << "The shared memory contains: " << shared_memory[0] << std::endl;
        shared_memory[0] = 0;   // data read
        pthread_mutex_unlock(mutex);
        
        // Cleanup
        munmap(shared_memory, sizeof(int));
        close(fd);

        return 0;
    }

    return 0;
}