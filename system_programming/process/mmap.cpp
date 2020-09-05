#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/mman.h>
#include <string>
#include <iostream>
#include <memory>

std::shared_ptr<char> getSharedCharMemory(int bytes)
{
    void *mem;
    int fd = open("testmmap", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        throw std::string(strerror(errno));
    }

    ftruncate(fd, bytes);
    int len = lseek(fd, 0, SEEK_END);

    mem = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mem == MAP_FAILED)
    {
        throw std::string(strerror(errno));
    }

    return std::shared_ptr<char>(static_cast<char *>(mem), [=](char *) { munmap(mem, len); });
}
int main()
{
    auto smp = getSharedCharMemory(20);

    strcpy(smp.get(), "Hello, mmap");

    std::cout << smp.get() << std::endl;
    smp.reset();
}
