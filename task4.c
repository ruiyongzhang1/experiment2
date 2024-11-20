#include<stdio.h>
#include<stdlib.h>
#include <time.h>
#define FRAME_SIZE 4// 假设我们有8个页面框架
#define SEQ_SIZE 8  // 页面请求序列的长度

//随机生成一个页面请求序列
int* random_page_queue(){
    int *page_que=(int*)malloc(SEQ_SIZE*sizeof(int));
    for(int i=0;i<SEQ_SIZE;i++){
        page_que[i]=rand()%8;//随机生成一个0-7的页面请求序列。
    }
    return page_que;
}

//使用手动输入生成一个页面请求序列
int* input_page_queue(){
    int *page_que=(int*)malloc(SEQ_SIZE*sizeof(int));
    for(int i=0;i<SEQ_SIZE;i++){
        scanf("%d",&page_que[i]);
    }
    return page_que;
}

//初始化页面为-1
void init_frames(int frames[]){
    for(int i=0;i<FRAME_SIZE;i++){
        frames[i]=-1;
    }
}

//检查是否在页面内
int Is_Inframes(int frames[],int block_num){
    int flag=0;
    for(int i=0;i<FRAME_SIZE;i++){
        if(frames[i]==block_num){
            flag=1;
            break;
        }
    }
    return flag;
}

//打印数组
void print_frames(int frames[]){
    for(int i=0;i<FRAME_SIZE;i++){
        printf("%d ",frames[i]);
    }
    printf("\n");
}
//打印命中率
void print_hit_rate(int hit_num){
    float hit=(float)hit_num;
    float seq=(float)SEQ_SIZE;
    float hit_rate=hit/seq;
    printf("hit rate is:%g\n",hit_rate);
}

//先进先出
void FIFO(int *frames, int *page_que) {
    int mod = 0;        
    int hit_num = 0;    
    for (int i = 0; i < SEQ_SIZE; i++) printf("%d ", page_que[i]);
    printf("\n");
    for (int i = 0; i < SEQ_SIZE; i++) {
        // 检查页面是否命中
        if (Is_Inframes(frames, page_que[i])) {
            hit_num++;  // 页面命中
        } else {
            // 页面未命中，替换页面
            frames[mod] = page_que[i];
            mod = (mod + 1) % FRAME_SIZE;
        }
        print_frames(frames);
    }
    print_hit_rate(hit_num);
}


void LRU(int *frames, int *page_que) {
    int frames_times[FRAME_SIZE] = {0}; // 初始化时间记录
    int current_time = 0; // 记录当前时间
    int hit_num = 0;      // 记录命中次数
    for (int i = 0; i < SEQ_SIZE; i++) printf("%d ", page_que[i]);
    printf("\n");
    for (int i = 0; i < SEQ_SIZE; i++) {
        current_time++; // 每处理一个请求，时间增加
        // 检查页面是否命中
        int hit_index = -1;
        for (int j = 0; j < FRAME_SIZE; j++) {
            if (frames[j] == page_que[i]) {
                hit_index = j;
                break;
            }
        }

        if (hit_index != -1) {
            hit_num++;
            frames_times[hit_index] = current_time;
        } else {
            // 页面未命中，需要替换
            int replace_index = 0;
            int min_time = frames_times[0];

            // 找到最久未被使用的页面
            for (int j = 1; j < FRAME_SIZE; j++) {
                if (frames_times[j] < min_time) {
                    replace_index = j;
                    min_time = frames_times[j];
                }
            }
            // 替换页面并更新时间戳
            frames[replace_index] = page_que[i];
            frames_times[replace_index] = current_time;
        }
        print_frames(frames);
    }
    print_hit_rate(hit_num);
}

int main(){
    int choice1;//选择变量。
    int choice2;
    int *page_queue=(int *)malloc(SEQ_SIZE*sizeof(int));
    int frames[FRAME_SIZE];
    srand(time(NULL));
    printf("随机生成请按1,手动输入请按2\n");
    scanf("%d",&choice1);//输入
    if(choice1==1)page_queue=random_page_queue();
    else if(choice1==2)page_queue=input_page_queue();
    init_frames(frames);
    printf("使用FIFO请按1,使用LRU请按2\n");
    scanf("%d",&choice2);
    if(choice2==1)FIFO(frames,page_queue);
    else if(choice2==2)LRU(frames,page_queue);
    return 0;
}