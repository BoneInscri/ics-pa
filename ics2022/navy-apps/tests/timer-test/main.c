#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <NDL.h>

int main()
{
    uint64_t cnt = 1;
    uint32_t us = 0;
    NDL_Init(0);
    while(1) {
        us = NDL_GetTicks();
        // printf("us : %lx\n", us);
        if(us > cnt * 1000000 / 2) {
        // if(us > cnt * 1000000) {
            printf("has passed 0.5 * %lu s\n", cnt);
            cnt++;
        }
    }
    NDL_Quit();
    return 0;
}
