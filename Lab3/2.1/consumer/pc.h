#ifndef __PC_H
#define __PC_H

struct data_item {
    unsigned int qid;
    unsigned long long time;
    char * msg; //NULL terminated C string
};

#define DEFAULT_SIZE 32 


#endif 
