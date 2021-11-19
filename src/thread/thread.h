#ifndef THREAD_H
#define THREAD_H
#include "config.h"
#include "myalloc.h"
#include "systick.h"

//最大可用线程数
#define max_thread_num 5

typedef void(*Func)();

typedef struct {
    volatile int sp;
    volatile int fp;
    volatile int* stack;
    volatile int wait_tick;//等待tick
    volatile int available;//是否激活
    volatile int pri;//运行优先级
    volatile int re_pri;//重装载优先级,每当一个线程运行完毕就会重置pri
}Thread;

extern volatile int thd_flag;

extern volatile int thd_ptr;
extern Thread* thd_ptr_;
extern int thd_stop_flag;
extern Thread thds[max_thread_num];

//线程初始化
void thread_init();
//线程创建
int thread_create(Func func, int stack_size, int level);
void thread_release(int thd_id);//线程释放
//线程睡眠
void thread_sleep_tick(int tick);
//线程开启调度
void thread_start();


//在进行一些不可分割操作时，调用该函数进行暂停线程调度
//关闭定时器中断 
//停止计数
#define thd_stop \
if (thd_flag == 1)\
{\
TIMER_DMAINTEN(TIMER6) &= (~(uint32_t)TIMER_INT_UP); \
thd_stop_flag++;\
}


//在进行一些不可分割操作时，调用该函数进行继续线程调度
//开启定时器中断
//继续计数
#define thd_cont \
if (thd_flag == 1)\
{ \
thd_stop_flag--;\
if (thd_stop_flag == 0)\
TIMER_DMAINTEN(TIMER6) |= (uint32_t)TIMER_INT_UP; \
}









#endif // !THREAD_H