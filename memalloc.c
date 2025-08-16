#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>


//simple malloc
// void *my_malloc(size_t size){
//     void *block;
//     block=sbrk(size);
//     if (block == (void *)-1){
//         return NULL;
//     }
//     return block;
// }

// typedef char ALIGN[16];


//基本記憶體分配節點結構
struct header_t{ //整個header_t的size是16bytes
     //原本文章是size_t類型，修改為unsigned，原本是size_t
     // size_t是8bytes，unsigned是4bytes，所以需要改為unsigned
    unsigned size;
    unsigned is_free;
    struct header_t *next;
};


//節點的頭節點和尾節點
struct header_t *head,*tail;

//全域鎖，用於多執行緒環境下的記憶體分配
pthread_mutex_t global_malloc_lock;

//尋找有大於等於size空閒的記憶體節點
struct header_t *get_free_block(size_t size){
    struct header_t *curr=head;
    while (curr){
        if (curr->is_free && curr->size>=size){
            return curr;
        }
        curr=curr->next;
    }
    return NULL;
}


void *my_malloc(size_t size){
    size_t total_size;
    void *block;
    struct header_t *header;
    if (!size){ //size 0 , return NULL
        return NULL;
    }

    //lock
    pthread_mutex_lock(&global_malloc_lock);
    header=get_free_block(size); //find the free block
    
    if (header){ //if find the free block,unlock,and return headeraddress
        header->is_free=0;
        pthread_mutex_unlock(&global_malloc_lock);
        return (void*)(header+1);
    }
    
    //if not find the free block , allocate new memory
    total_size=sizeof(struct header_t)+size;
    block=sbrk(total_size);//set total_size block

    if (block==(void*)-1){//block not success,unlock,and return NULL
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL;
    }

    //header record the block information
    header=block;
    header->size=size;
    header->is_free=0;
    header->next=NULL;

    //if head is NULL, set header to head
    if (!head){
        head=header;
    }

    //if tail is not NULL，set tail next node to header
    if (tail){
        tail->next=header;
    }


    tail=header;
    pthread_mutex_unlock(&global_malloc_lock);
    return (void*)(header+1);
}

void my_free(void *block){

    struct header_t *header, *tmp;
    void *programbreak;
    
    if (!block){//can't free NULL
        return;
    }
    
    pthread_mutex_lock(&global_malloc_lock);
    header=(struct header_t *)block-1; //find the  last block
    
    programbreak=sbrk(0);
    if ((char*)block+header->size==programbreak){ //if the block is the last block
        if (head==tail){ /
            head=tail=NULL;
        }
        else{ 
            tmp=head;
            while (tmp){
                if (tmp->next==tail){
                    tmp->next=NULL;
                    tail=tmp;
                }
                tmp=tmp->next;
            }
        }
        sbrk(0-sizeof(struct header_t)-header->size);
        pthread_mutex_unlock(&global_malloc_lock);
        return;
    }
    //memory block is not the last block
    header->is_free=1; //set the block to free， can reuse again
    pthread_mutex_unlock(&global_malloc_lock);
}

void *my_calloc(size_t num,size_t nsize){
    size_t size;
    void *block;
    if (!num || !nsize){
        return NULL;
    }

    size = num * nsize;

    //check will overflow or not
    if (nsize!=size/num){
        return NULL;
    }
    block=my_malloc(size);
    if (!block){
        return NULL;
    }
    memset(block,0,size);
    return block;

}

void *my_realloc(void *block, size_t size){
    struct header_t *header;
    void *ret;
    if (!block || !size){
        return my_malloc(size);
    }
    header = (struct header_t *)block-1;
    if (header->size>=size){
        return block;
    }
    ret=my_malloc(size);
    if (ret){
        memcpy(ret,block,header->size);
        my_free(block);
    }
    return ret;
}

int main(){
  

    printf("%ld\n",sizeof(struct header_t));
    //測試一般malloc和free
    printf("malloc test\n");
    int *a=my_malloc(2*sizeof(int));
    *a=10;
    printf("%p %d\n", a, *a);
    // my_free(a);

    printf("--------------------------------\n");


    //測試calloc，calloc會把memory清空，並設定為0
    printf("calloc test\n");
    int *b=my_calloc(2,sizeof(int));
    printf("%p %d\n", b, *b);
    printf("%p %d\n", b+1, *(b+1));
    my_free(b);

    printf("--------------------------------\n");


    //測試realloc，realloc會把memory重新分配size
   printf("realloc test\n");
   a=my_realloc(a,3*sizeof(int));
   *a=10;
   printf("%p %d\n", a, *a);
   *(a+1)=20;
   printf("%p %d\n", a+1, *(a+1));
   *(a+2)=30;
   printf("%p %d\n", a+2, *(a+2));
   my_free(a);


    return 0;

}