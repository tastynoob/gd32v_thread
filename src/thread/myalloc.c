#include "myalloc.h"



volatile u32 buff[16 * (1024 / 4)];//16k

#define alloc_addr (int)buff
#define alloc_size 16*1024

typedef struct {
    volatile int size;//内存块大小
    volatile int avaliable;//是否激活
    volatile void* next;//下一个块
} mem_block;


mem_block* alloc_head;

void myalloc_init() {

    alloc_head = (mem_block*)buff;
    alloc_head->size = 0;
    alloc_head->avaliable = 0;
    alloc_head->next = 0;
}



void* myalloc(u32 sizeofbyte) {

    thd_stop;
    mem_block* temp = alloc_head;//指向当前块

    if (sizeofbyte == 0) {
        thd_cont;
        return 0;
    }
    //实际分配的大小
    int real_size = 4 * (sizeofbyte / 4 + ((sizeofbyte % 4) ? 1 : 0));

    
    while (1) {

        if (temp->avaliable == 0) { //当前块没有被使用

        alloc_new:

            if (temp->next == 0) { //假如下一个块为空，则分配当前块

                //如果剩余空间不足以分配
                if (((int)temp) + real_size > alloc_addr + alloc_size) {
                    thd_cont;;
                    return 0;
                }

                temp->size = real_size;
                temp->avaliable = 1;
                temp->next = ((char*)temp + sizeof(mem_block)) + real_size;
                //没有超界
                if ((temp->next > buff) && (((int)temp->next) + sizeof(mem_block) < (alloc_addr + alloc_size))) {
                    ((mem_block*)temp->next)->size = 0;
                    ((mem_block*)temp->next)->avaliable = 0;
                    ((mem_block*)temp->next)->next = 0;
                }
                break;
            }
            else { //下一个块不为空 
                if (temp->size >= real_size) { //如果当前块的大小满足要求
                    //如果当前块减去分配的内存，剩余的大小还能够再分配一个小块
                    if (temp->size - real_size > sizeof(mem_block) + 4) {
                        //新块
                        mem_block* temp1 = (mem_block*)(((char*)(temp + 1)) + real_size);
                        temp1->size = temp->size - real_size - sizeof(mem_block);
                        temp1->avaliable = 0;
                        temp1->next = temp->next;

                        temp->next = temp1;
                        temp->size = real_size;
                    }

                    temp->avaliable = 1;
                    break;
                }
                else { //不满足要求 (下一个块不为空并且当前块不满足要求)
                    if (((mem_block*)temp->next)->avaliable == 0) { // 如果下一个块没有激活，则可以分配到当前块
                        temp->size = temp->size + ((mem_block*)temp->next)->size + sizeof(mem_block);//重新分配当前块的大小
                        temp->next = ((mem_block*)temp->next)->next;//重新改变指向下一块的指针
                        goto alloc_new; //重新进行内存分配判断
                    }
                    else {//下一个块已被激活，跳过
                    }
                }
            }
        }

        //如果指针指向正确
        if (((temp->next) > (void*)buff) && (temp->next < (void*)(alloc_addr + alloc_size))) {
            temp = (mem_block*)temp->next;
        }
        else {
            thd_cont;
            return 0;
        }
    }
    
    if (temp->size == real_size) {
        thd_cont;
        return ((char*)temp + sizeof(mem_block));
    }
    thd_cont;
    return 0;
}


u32 get_block_size(void* ptr) {
    if ((int)ptr >= alloc_addr) {
        mem_block* temp = ((mem_block*)ptr - 1);
        return temp->size;
    }
    return 0;
}


void* myrealloc(void* ptr, u32 sizeofbyte) {
    int* new_ptr;
    if (ptr == NULL) {
        
        return myalloc(sizeofbyte);
        
    }
    else if (sizeofbyte == 0) {
        return NULL;
    }
    else if ((int)ptr >= alloc_addr && (int)ptr < alloc_addr + alloc_size) {
        new_ptr = myalloc(sizeofbyte);
        for (int i = 0;i < get_block_size(ptr);i++) {
            new_ptr[i] = ((int*)ptr)[i];
        }
    }

    return new_ptr;
}







void myfree(void* ptr) {
    thd_stop;
    if ((int)ptr >= alloc_addr) {
        mem_block* temp = ((mem_block*)ptr - 1);
        temp->avaliable = 0;
    }
    thd_cont;
}


