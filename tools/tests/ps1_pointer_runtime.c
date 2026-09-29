/** @file ps1_pointer_runtime.c
 * @brief Linux runner for the native PS1 pointer tests, isolated from SDK types.
 */
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>

int ps1_test_pointer_ops(void* storage);

/** @brief Map a PS1-range address and run the pointer tests. @return Process status. */
int main(void)
{
    void* expected = (void*)(uintptr_t)0x81000000U;
    void* storage = mmap(expected, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    int result;
    if (storage == MAP_FAILED)
    {
        perror("mmap PS1 test storage");
        return 1;
    }
    if (storage != expected)
    {
        fprintf(stderr, "PS1 test storage mapped at an unexpected address\n");
        munmap(storage, 4096);
        return 1;
    }
    result = ps1_test_pointer_ops(storage);
    munmap(storage, 4096);
    if (result != 0)
    {
        fprintf(stderr, "PS1 pointer check failed at line %d\n", result);
        return 1;
    }
    puts("PS1 pointer storage and game consumer tests passed");
    return 0;
}
