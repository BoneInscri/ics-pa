#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>

int main()
{
    struct timeval time;

    uint64_t cnt = 1;
    while(1) {
        int ret = gettimeofday(&time, NULL);        
        uint64_t us = time.tv_sec * 1000000 + time.tv_usec;
        // printf("us : %lx\n", us);
        if(us > cnt * 1000000 / 2) {
        // if(us > cnt * 1000000) {
            printf("has passed 0.5 * %lu s\n", cnt);
            cnt++;
        }
    }
    return 0;
}
