#ifndef __FIFO_H
#define __FIFO_H

struct data_item {
    unsigned int qid;
    unsigned long long time;
    char * msg; //NULL terminated C string
};

#define DEFAULT_SIZE 32 


#endif 