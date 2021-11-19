#include "thread.h"



int thd_num = 1;
Thread thds[max_thread_num];
//当前线程指针
volatile int thd_ptr = 0;
Thread* thd_ptr_;
volatile int thd_flag = 0;
int thd_stop_flag = 0;



/*
线程调度说明：
每个线程都有一个基础等级,等级越大执行概率越低
基础等级 = level

每个线程运行一次则增加1点等级
其它可用但未执行的线程等级-1




*/

//线程初始化
void thread_init() {
    thd_stop_flag = 0;

    for (int i = 0;i < max_thread_num;i++) {
        thds[i].fp = 0;
        thds[i].sp = 0;
        thds[i].stack = 0;
        thds[i].wait_tick = 0;
        thds[i].available = 0;
    }
    //主线程,主线程优先级为2
    thds[0].available = 1;
    thds[0].wait_tick = 0;
    thds[0].pri = 2;
    thds[0].re_pri = 2;
    thd_ptr_ = &thds[0];
}

//线程创建
int thread_create(Func func, int stack_size, int level) {

    thd_stop;

    for (int i = 0;i < max_thread_num;i++) {

        if (thds[i].available == 0) {

            thds[i].stack = myalloc(stack_size);

            int stack_dep = (stack_size - 1) / 4;

            //设置栈和栈帧
            thds[i].sp = (int)&(thds[i].stack[stack_dep - 31]);
            thds[i].fp = (int)&(thds[i].stack[stack_dep]);

            //msubm所在位置
            thds[i].stack[stack_dep - 31 + 11 + 19] = 64;
            //mecp所在位置
            thds[i].stack[stack_dep - 31 + 11 + 18] = (int)func;
            //mcause所在位置
            thds[i].stack[stack_dep - 31 + 11 + 17] = -1207959478;
            //x1所在位置
            thds[i].stack[stack_dep - 31 + 11 + 0] = (int)func;

            thds[i].available = 1;


            thds[i].pri = level;
            thds[i].re_pri = level;

            thd_cont;

            return i;
        }

    }

    thd_cont;

    return 0;
}
//线程释放
void thread_release(int thd_id) {

    thd_stop;

    thds[thd_id].available = 0;
    myfree((void*)thds[thd_id].stack);

    thd_cont;
}








//线程睡眠,并手动触发中断
void thread_sleep_tick(int tick) {


    thds[thd_ptr].wait_tick = tick;

    //等待中断
    asm volatile("wfi");
}




//线程开启调度
void thread_start() {


    //清除标志位
    TIMER_INTF(TIMER6) = (~1);
    thd_flag = 1;
    //开启时钟
    TIMER_CTL0(TIMER6) |= (uint32_t)TIMER_CTL0_CEN;
    //关闭时钟                                                                    
    //TIMER_CTL0(TIMER6) &= ~(uint32_t)TIMER_CTL0_CEN;
}




int thds_temp[max_thread_num];

//线程切换，由中断程序进行调用
void switch_thread(int sp, int fp) {


    //保存上次中断处的sp、fp
    thds[thd_ptr].sp = sp;
    thds[thd_ptr].fp = fp;

    register int temp_head = 0;
    register int thd_vld = 0;
    while (1) {
        // int p;
        // asm volatile("mv %0,sp":"=r"(p) : );
        //printf("sp%d\n\n", thds[thd_ptr].wait_tick);


        for (int i = 0;i < max_thread_num;i++) {
            if (thds[i].available) {

                if (thds[i].wait_tick == 0) {
                    thds_temp[temp_head++] = i;//寻找是否有空闲线程,如果有，则保存线程索引
                    thd_vld = 1;
                }
                else {//对于非空闲线程
                    thds[i].wait_tick--;//每个线程的等待时间-1ms
                }
            }
        }

        //printf("dd\n\n");
        if (thd_vld)break;


        //等待大概0.9ms
        {
            volatile uint64_t start_mtime, delta_mtime;

            volatile uint64_t tmp = get_timer_value();
            do {
                start_mtime = get_timer_value();
            } while (start_mtime == tmp);


            uint64_t delay_ticks = delay_ticks = SystemCoreClock / 4400;

            do {
                delta_mtime = get_timer_value() - start_mtime;
            } while (delta_mtime < delay_ticks);
        }
    }

    //对可用线程的等级进行从小到大排序
    for (int i = 0;i < temp_head - 1;i++) {
        for (int j = 0;j < temp_head - 1 - i;j++) {
            if (thds[thds_temp[j]].pri > thds[thds_temp[j + 1]].pri) {
                int t = thds_temp[j];
                thds_temp[j] = thds_temp[j + 1];
                thds_temp[j + 1] = t;
            }
            else if (thds[thds_temp[j]].pri == thds[thds_temp[j + 1]].pri) {
                if (mtime_lo() % 2) {//如果有相同的则随机交换
                    int t = thds_temp[j];
                    thds_temp[j] = thds_temp[j + 1];
                    thds_temp[j + 1] = t;
                }

            }
        }
    }

    //取出栈顶索引
    thd_ptr = thds_temp[0];
    thd_ptr_ = &thds[thd_ptr];
    //当前线程被执行，重新设置优先级
    thds[thd_ptr].pri = thds[thd_ptr].re_pri;
    //其它未执行的线程优先级增加
    for (register int i = 1;i < temp_head;i++) {
        thds[thds_temp[i]].pri--;
    }


    //printf("p%d\n\n", thd_ptr);
    //切换到选中的线程    
    asm volatile("mv x30,%0"::"r"(thds[thd_ptr].sp));
    asm volatile("mv x31,%0"::"r"(thds[thd_ptr].fp));
}


