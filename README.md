# simple_memory_allocator

# 重點備註
sbrk(0) gives the current address of program break.
brk(x) with a positive value increments brk by x bytes, as a result allocating memory.
sbrk(-x) with a negative value decrements brk by x bytes, as a result releasing memory.
On failure, sbrk() returns (void*) -1

sbrk()已被認為已經過時了，因為對於multi-thread操作不安全，替代品有mmap()

然而在glibc實作malloc還是有再用sbrk()配置不大的記憶體空間

參考的文章的header結構體是用union來變成16 bytes size，這邊是改成size_t變成unsigned型態


# 參考來源
https://arjunsreedharan.org/post/148675821737/memory-allocators-101-write-a-simple-memory
