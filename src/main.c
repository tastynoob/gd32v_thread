#include "config.h"
#include "systick.h"
#include "thread.h"
#include "myalloc.h"
#include "lcd.h"
#include "coremark.h"




void mat_mult(int m, int n, int p, int* a, int* b, int* c) {
    int i, j, k;
    c[0] = a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}



//消息队列测试
enum Msg_t {
    mat_mul,mat_add
};

typedef struct {
    int row0;
    int col0;
    int row1;
    int col1;
    int *mats;
} mat_ko;



//用户线程1
void thread1() {
    mat_ko mats = { 1, 3, 3, 1 };
    mats.mats = myalloc(sizeof(int) * (1 * 3 + 3 * 1));
    //初始化矩阵0
    for (int i = 0; i < mats.row0; i++) {
        for (int j = 0; j < mats.col0; j++) {
            mats.mats[i * 3 + j] = i + j;
        }
    }
    //初始化矩阵1
    for (int i = 0; i < mats.row1; i++) {
        for (int j = 0; j < mats.col1; j++) {
            mats.mats[i * 1 + j + mats.col0*mats.row0] = i + j;
        }
    }
    char* result;
    while (1) {
        //发送消息
        msg_queue_set(mat_mul, "t1", (char*)&mats, sizeof(mat_ko));
        //等待消息
        while (msg_queue_get(mat_mul, "s1", &result, 0) == 0) {
            thread_sleep_tick(1);
        }
        //输出结果
        LCD_ShowNum(0, 0, ((int*)result)[0], 4, RED);
        //释放矩阵
        myfree(mats.mats);
        //释放本身线程
        thread_release(thd_ptr);
    }
}


//用户线程2
void thread2() {
    mat_ko mats = { 1, 3, 3, 1 };
    mats.mats = myalloc(sizeof(int) * (1 * 3 + 3 * 1));
    //初始化矩阵0
    for (int i = 0; i < mats.row0; i++) {
        for (int j = 0; j < mats.col0; j++) {
            mats.mats[i * mats.col0 + j] = i;
        }
    }
    //初始化矩阵1
    for (int i = 0; i < mats.row1; i++) {
        for (int j = 0; j < mats.col1; j++) {
            mats.mats[i * mats.col1 + j + mats.col0 * mats.row0] = j;
        }
    }
    char* result;
    while (1) {
        //发送消息
        msg_queue_set(mat_mul, "t2", (char*)&mats, sizeof(mat_ko));
        //等待消息
        while (msg_queue_get(mat_mul, "s2", &result, 0) == 0) {
            thread_sleep_tick(1);
        }
        //输出结果
        LCD_ShowNum(0, 20, ((int*)result)[0], 4, RED);
        //释放矩阵
        myfree(mats.mats);
        //释放本身线程
        thread_release(thd_ptr);
    }
}

//执行线程
void thread_exe() {
    int result1, result2;
    mat_ko* mats;
    int data_len;
    int* mat_res = myalloc(sizeof(int) * (3 * 3));
    while (1) {
        //等待消息
        bool tag = msg_queue_available(mat_mul);
        if (tag) {
            bool tag1 = msg_queue_get(mat_mul, "t1", &mats, &data_len);
            if (tag1) {
                mat_mult(mats->row0, mats->col0, mats->col1, mats->mats, mats->mats + mats->col0 * mats->row0, mat_res);
                
                result1 = mat_res[0];
                msg_queue_set(mat_mul, "s1", (char*)&result1, sizeof(int));
            }
            bool tag2 = msg_queue_get(mat_mul, "t2", &mats, &data_len);
            if (tag2) {
                mat_mult(mats->row0, mats->col0, mats->col1, mats->mats, mats->mats + mats->col0 * mats->row0, mat_res);
                result2 = mat_res[0];
                msg_queue_set(mat_mul, "s2", (char*)&result2, sizeof(int));
            }
        }
        
    }
}
//git repeat

char str[] = "hello";
char str1[] = "world";
int main(void) {
    Init();
    myalloc_init();
    thread_init();

    thread_create(thread1, 2000, 4);
    thread_create(thread2, 2000, 4);
    thread_create(thread_exe, 2000, 1);

    thread_start();

    thread_release(thd_ptr);
    int i = 0;
    //发送消息
    //alloc测试
    while (1);
        
}
