#include "config.h"
#include "systick.h"
#include "thread.h"
#include "myalloc.h"
#include "lcd.h"
#include "coremark.h"







   





int main(void) {
    Init();
    myalloc_init();
    thread_init();

    printf("start\n");

    thread_start();

    while (1) {

        

    }
}
