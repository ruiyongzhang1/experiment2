#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PROCESS_NAME_LEN 32 /*进程名长度*/
#define MIN_SLICE 10 /*最小碎片的大小*/
#define DEFAULT_MEM_SIZE 1024 /*内存大小*/
#define DEFAULT_MEM_START 0 /*起始位置*/
//三种内存分配算法
#define MA_FF 1
#define MA_BF 2
#define MA_WF 3

//空闲块
typedef struct free_block_type {
    int size;
    int start_addr;
    struct free_block_type *next;
} FreeBlock;
//已分配的内存块
typedef struct allocated_block {
    int pid;
    int size;
    int start_addr;
    char process_name[PROCESS_NAME_LEN];
    struct allocated_block *next;
} AllocatedBlock;

FreeBlock *free_block = NULL;//空闲块链表的头指针
AllocatedBlock *allocated_block_head = NULL;//进程占用块链表的头指针

int mem_size = DEFAULT_MEM_SIZE;//内存大小
int ma_algorithm = 1; //最先适应算法
static int pid = 0;//初始化pid
int flag = 0;//用于设置内存大小的标记
//功能选择菜单
void display_menu() {
    printf("\n");
    printf("1 - Set memory size (default=%d)\n", DEFAULT_MEM_SIZE);
    printf("2 - Select memory allocation algorithm\n");
    printf("3 - New process\n");
    printf("4 - Terminate a process\n");
    printf("5 - Display memory usage\n");
    printf("0 - Exit\n");
}
//空闲块初始化
FreeBlock* init_free_block(int mem_size) {
    FreeBlock *fb = (FreeBlock *)malloc(sizeof(FreeBlock));
    if (fb == NULL) {
        printf("No mem\n");
        return NULL;
    }
    fb->size = mem_size;
    fb->start_addr = DEFAULT_MEM_START;
    fb->next = NULL;
    return fb;
}

//设置内存大小
int set_mem_size() {
    int size;
    if (flag != 0) {
        printf("Cannot set memory size again\n");
        return 0;
    }
    printf("Total memory size = ");
    scanf("%d", &size);
    if (size > 0) {
        mem_size = size;
        if (free_block != NULL) {
            free(free_block);
        }
        free_block = init_free_block(mem_size);//此处不考虑系统分区
    }
    flag = 1;
    return 1;
}

void rearrange_WF() {
    FreeBlock *sorted = NULL;  // 已排序链表的头节点
    FreeBlock *current = free_block;  // 当前处理的空闲块
    FreeBlock *smallest_prev, *smallest, *prev;

    while (current != NULL) {
        // 找到当前链表中的最小块
        smallest = current;
        smallest_prev = NULL;
        prev = current;
        while (prev->next != NULL) {
            if (prev->next->size < smallest->size) {
                smallest_prev = prev;
                smallest = prev->next;
            }
            prev = prev->next;
        }

        // 将找到的最小块从原始链表中移除
        if (smallest_prev == NULL) {  // 说明 smallest 是当前链表头
            current = current->next;
        } else {
            smallest_prev->next = smallest->next;
        }

        // 将最小块添加到排序链表的头部
        smallest->next = sorted;
        sorted = smallest;
    }

    // 更新 free_block 为排序后的链表
    free_block = sorted;
}

void rearrange_BF() {
    FreeBlock *sorted = NULL;  // 已排序链表的头节点
    FreeBlock *current = free_block;  // 当前处理的空闲块
    FreeBlock *largest_prev, *largest, *prev;

    while (current != NULL) {
        // 找到当前链表中的最大块
        largest = current;
        largest_prev = NULL;
        prev = current;
        while (prev->next != NULL) {
            if (prev->next->size > largest->size) {
                largest_prev = prev;
                largest = prev->next;
            }
            prev = prev->next;
        }

        // 将找到的最大块从原始链表中移除
        if (largest_prev == NULL) {  // 说明 largest 是当前链表头
            current = current->next;
        } else {
            largest_prev->next = largest->next;
        }

        // 将最大块添加到排序链表的头部
        largest->next = sorted;
        sorted = largest;
    }

    // 更新 free_block 为排序后的链表
    free_block = sorted;
}

//整理内存的函数
void rearrange(int algorithm) {
    switch (algorithm) {
        case MA_FF:
            // rearrange_FF();什么都不做就好了
            break;
        case MA_BF:
            rearrange_BF();//由小到大排列
            break;
        case MA_WF:
            rearrange_WF();//由大到小排列
            break;
    }
}
//选择整理内存的算法
void set_algorithm() {
    int algorithm;
    printf("\t1 - First Fit\n");
    printf("\t2 - Best Fit\n");
    printf("\t3 - Worst Fit\n");
    scanf("%d", &algorithm);
    if (algorithm >= 1 && algorithm <= 3)
        ma_algorithm = algorithm;
    rearrange(ma_algorithm);
}

void update_allocated_blocks(int start_addr, int size) {
    AllocatedBlock *ab_temp = allocated_block_head;
    while (ab_temp != NULL) {
        if (ab_temp->start_addr >= start_addr) {
            ab_temp->start_addr += size;
        }
        ab_temp = ab_temp->next;
    }
}

int allocate_mem(struct allocated_block *ab) {
    struct free_block_type *fbt, *pre, *work;
    struct allocated_block *work1,*pre1;
    int request_size = ab->size;

    fbt = free_block;
    pre = NULL;

    // 先尝试正常分配
    while (fbt != NULL) {
        if (fbt->size >= request_size) {
            if (fbt->size - request_size >= MIN_SLICE) {
                // 分配内存并分割空闲块
                ab->start_addr = fbt->start_addr;
                fbt->start_addr += request_size;
                fbt->size -= request_size;
                return 1;
            } else {
                // 整个空闲块被分配
                ab->start_addr = fbt->start_addr;
                ab->size = fbt->size;
                if (pre == NULL) {
                    // fbt是头节点
                    free_block = fbt->next; // 头节点被分配，更新头节点
                } else {
                    // fbt不是头节点，更新前一个节点的next指针
                    pre->next = fbt->next;
                }
                fbt->next = NULL; // 清除fbt的next指针
                free(fbt); // 释放fbt
                return 1;
            }
        }
        pre = fbt;
        fbt = fbt->next; // 移动到下一个节点
    }

    // 如果无法直接分配，检查所有空闲块的总大小是否足够
    int total_free_size = 0;
    for (work = free_block; work != NULL; work = work->next) {
        total_free_size += work->size;
    }
    if (total_free_size >= request_size) {
        // 合并所有空闲块
        work = free_block;
        FreeBlock *tmp;
        fbt = (FreeBlock *)malloc(sizeof(FreeBlock)); // 新的合并后的空闲块

        fbt->start_addr = 0; // 从第一个空闲块的地址开始
        fbt->size = total_free_size;
        fbt->next = NULL;
        // 清空原有的空闲链表
        while (work != NULL) {
            tmp = work;
            work = work->next;
            free(tmp);
        }
        free_block = fbt; // 更新空闲链表为新的合并块
        work1 =allocated_block_head;
        work1->start_addr=total_free_size;
        pre1=work1;
        work1=work1->next;
        for(; work1 != NULL; work1 = work1->next){
            work1->start_addr=pre1->start_addr+pre1->size;
            pre1=pre1->next;
        }
        // 重新分配内存
        return allocate_mem(ab);
    }

    // 所有空闲块加起来也不够
    return -1;
}

//创建新的进程
void new_process() {
    AllocatedBlock *ab = (AllocatedBlock *)malloc(sizeof(AllocatedBlock));
    if (!ab) exit(-5);//失败则退出
    ab->next = NULL;
    pid++;//进程数计数
    sprintf(ab->process_name, "PROCESS-%02d", pid);
    ab->pid = pid;
    printf("Memory for %s:", ab->process_name);
    scanf("%d", &ab->size);
    if (ab->size > 0) {
        int ret = allocate_mem(ab);//分配空间
        if (ret == 1) {//返回值为1说明成功分配
            if (allocated_block_head == NULL) {
                allocated_block_head = ab;
            } else {
                ab->next = allocated_block_head;
                allocated_block_head = ab;
            }
        } else {
            printf("Allocation fail\n");
            free(ab);
        }
    }
}
//归还空间
int free_mem(struct allocated_block *ab) {
    FreeBlock *fbt, *pre, *work;
    // 创建一个新的空闲块节点，用于插入到空闲块链表中
    fbt = (FreeBlock *)malloc(sizeof(FreeBlock));
    if (!fbt) return -1; // 如果内存分配失败，返回-1
    fbt->start_addr = ab->start_addr;
    fbt->size = ab->size;
    fbt->next = NULL;

    // 将新释放的节点插入到空闲分区队列，并按地址排序
    pre = NULL;
    for (work = free_block; work != NULL && work->start_addr < fbt->start_addr; pre = work, work = work->next);
    if (pre == NULL) {
        // 插入到链表头部
        fbt->next = free_block;
        free_block = fbt;
    } else {
        // 插入到链表中间或尾部
        fbt->next = pre->next;
        pre->next = fbt;
    }

    // 检查并合并相邻的空闲分区
    pre = NULL;
    work = free_block;
    while (work != NULL && work->next != NULL) {
        if (work->start_addr + work->size == work->next->start_addr) {
            // 合并相邻的空闲块
            work->size += work->next->size;
            FreeBlock *tmp = work->next;
            work->next = work->next->next; // 删除原来的work->next节点
            free(tmp);
            // 回到链表头部，因为合并可能导致新的合并
            pre = NULL;
            work = free_block;
        } else {
            pre = work;
            work = work->next;
        }
    }

    return 1; // 释放成功
}
//释放某个进程块
void dispose(AllocatedBlock *ab) {
    AllocatedBlock *pre, *tmp;
    if (ab == allocated_block_head) {
        tmp = allocated_block_head;
        allocated_block_head = allocated_block_head->next;
        free(tmp);
    } else {
        pre = allocated_block_head;
        tmp = allocated_block_head->next;
        while (tmp != ab) {
            pre = tmp;
            tmp = tmp->next;
        }
        pre->next = tmp->next;
        free(tmp);
    }
}
//寻找进程
AllocatedBlock* find_process(int pid){
    AllocatedBlock* ans=allocated_block_head;
    while(ans!=NULL){
        if(ans->pid==pid)return ans;
        ans=ans->next;
    }
    return ans;
}
//结束进程
void kill_process(){
    AllocatedBlock*ab;
    int pid;
    printf("Kill Process, pid=");
    scanf("%d", &pid);
    ab=find_process(pid);
    if(ab!=NULL){
    free_mem(ab); /*释放 ab 所表示的分配区*/
    dispose(ab); /*释放 ab 数据结构节点*/
    }
}

//显示当前内存的使用情况，包括空闲区的情况和已经分配的情况 
void display_mem_usage() {
    FreeBlock *fbt = free_block;
    AllocatedBlock *ab = allocated_block_head;
    if (fbt == NULL) return;
    printf("----------------------------------------------------------\n");
    printf("Free Memory:\n");
    printf("%20s %20s\n", "start_addr", "size");
    while (fbt != NULL) {
        printf("%20d %20d\n", fbt->start_addr, fbt->size);
        fbt = fbt->next;
    }
    printf("\nUsed Memory:\n");
    printf("%10s %20s %10s %10s\n", "PID", "ProcessName", "start_addr", "size");
    while (ab != NULL) {
        printf("%10d %20s %10d %10d\n", ab->pid, ab->process_name, ab->start_addr, ab->size);
        ab = ab->next;
    }
    printf("----------------------------------------------------------\n");
}

void do_exit() {
    while (free_block != NULL) {
        FreeBlock *tmp = free_block;
        free_block = free_block->next;
        free(tmp);
    }
    while (allocated_block_head != NULL) {
        AllocatedBlock *tmp = allocated_block_head;
        allocated_block_head = allocated_block_head->next;
        free(tmp);
    }
}

int main() {
    char choice;
    free_block = init_free_block(mem_size);
    while (1) {
        display_menu();
        fflush(stdin);
        choice = getchar();
        switch (choice) {
            case '1':
                set_mem_size();
                getchar();
                break;
            case '2':
                set_algorithm();
                getchar();
                break;
            case '3':
                new_process();
                getchar();
                break;
            case '4':
                kill_process();
                getchar();
                break;
            case '5':
                display_mem_usage();
                getchar();
                break;
            case '0':
                do_exit();
                exit(0);
                break;
            default:
                break;
        }
    }
    return 0;
}