#include "config.h"

#define buff_length (4*1024)
//缓存队列
volatile char buff[buff_length];

//ps指向将保存数据的地址
uint32_t ps = 0;
//pe指向将读取数据的地址
uint32_t pe = 0;
volatile char str_read[buff_length];


void USART0_IRQHandler(void) {
    if (RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE)) {
        /* receive data */
        if (ps >= buff_length) {
            ps = 0;
        }
        buff[ps] = USART_DATA(USART0);
        ps++;
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RBNE);
    }
}


uint32_t avaliable() {
    if (ps > pe) {
        return  ps - pe;
    }
    else if (ps < pe) {
        return (buff_length - pe) + ps;
    }
    return 0;
}

uint32_t readAll() {
    //log("ps:%d，pe:%d\n", ps,pe);
    uint32_t len = 0;
    while (pe != ps) {
        if (pe >= buff_length)
            pe = 0;
        str_read[len] = buff[pe];
        len++;
        pe++;
    }
    str_read[len] = 0;
    return len;
}


void buffClear() {
    pe = ps;
}

uint32_t readLine() {
    uint32_t len = 0;
    while (pe != ps) {
        if (pe >= buff_length)
            pe = 0;
        char a = buff[pe];
        if (a == '\n') {
            break;
        }
        str_read[len] = a;
        len++;
        pe++;
    }
    str_read[len] = 0;
    return len;
}


uint32_t readch() {
    if (pe == ps) {
        return 0;
    }
    int a = buff[pe];
    pe++;
    if (pe >= buff_length)
        pe = 0;
    return a;
}

uint32_t readbyte2() {
    uint32_t a = readch();
    uint32_t b = readch();
    return (a << 8) + b;
}





