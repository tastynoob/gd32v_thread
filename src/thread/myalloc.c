#include "myalloc.h"



volatile u32 buff[16 * (1024 / 4)];//16k

#define alloc_addr (int)buff
#define alloc_size 16*1024

typedef struct {
    volatile int size;//�ڴ���С
    volatile int avaliable;//�Ƿ񼤻�
    volatile void* next;//��һ����
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
    mem_block* temp = alloc_head;//ָ��ǰ��

    if (sizeofbyte == 0) {
        thd_cont;
        return 0;
    }
    //ʵ�ʷ���Ĵ�С
    int real_size = 4 * (sizeofbyte / 4 + ((sizeofbyte % 4) ? 1 : 0));

    
    while (1) {

        if (temp->avaliable == 0) { //��ǰ��û�б�ʹ��

        alloc_new:

            if (temp->next == 0) { //������һ����Ϊ�գ�����䵱ǰ��

                //���ʣ��ռ䲻���Է���
                if (((int)temp) + real_size > alloc_addr + alloc_size) {
                    thd_cont;;
                    return 0;
                }

                temp->size = real_size;
                temp->avaliable = 1;
                temp->next = ((char*)temp + sizeof(mem_block)) + real_size;
                //û�г���
                if ((temp->next > buff) && (((int)temp->next) + sizeof(mem_block) < (alloc_addr + alloc_size))) {
                    ((mem_block*)temp->next)->size = 0;
                    ((mem_block*)temp->next)->avaliable = 0;
                    ((mem_block*)temp->next)->next = 0;
                }
                break;
            }
            else { //��һ���鲻Ϊ�� 
                if (temp->size >= real_size) { //�����ǰ��Ĵ�С����Ҫ��
                    //�����ǰ���ȥ������ڴ棬ʣ��Ĵ�С���ܹ��ٷ���һ��С��
                    if (temp->size - real_size > sizeof(mem_block) + 4) {
                        //�¿�
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
                else { //������Ҫ�� (��һ���鲻Ϊ�ղ��ҵ�ǰ�鲻����Ҫ��)
                    if (((mem_block*)temp->next)->avaliable == 0) { // �����һ����û�м������Է��䵽��ǰ��
                        temp->size = temp->size + ((mem_block*)temp->next)->size + sizeof(mem_block);//���·��䵱ǰ��Ĵ�С
                        temp->next = ((mem_block*)temp->next)->next;//���¸ı�ָ����һ���ָ��
                        goto alloc_new; //���½����ڴ�����ж�
                    }
                    else {//��һ�����ѱ��������
                    }
                }
            }
        }

        //���ָ��ָ����ȷ
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


