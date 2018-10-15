/*****************************************************************************************************************************
This project was supported by the National Basic Research 973 Program of China under Grant No.2011CB302301
Huazhong University of Science and Technology (HUST)   Wuhan National Laboratory for Optoelectronics

FileName： ssd.c
Author: Hu Yang		Version: 2.1	Date:2011/12/02
Description: 

History:
<contributor>     <time>        <version>       <desc>                   <e-mail>
Yang Hu	        2009/09/25	      1.0		    Creat SSDsim       yanghu@foxmail.com
                2010/05/01        2.x           Change 
Zhiming Zhu     2011/07/01        2.0           Change               812839842@qq.com
Shuangwu Zhang  2011/11/01        2.1           Change               820876427@qq.com
Chao Ren        2011/07/01        2.0           Change               529517386@qq.com
Hao Luo         2011/01/01        2.0           Change               luohao135680@gmail.com
*****************************************************************************************************************************/
#include "ssd.h"
#define WRITE_RATIO_IN_REQUEST_QUEUE  0.3

int printf_gc_node_information(struct gc_operation* gc_node)
{

//    printf(" befor service gc, gc_node information is gc_node, chip %d, die %d, plane %d,  gc_node->victim_block_number is %d, gc copy_page_number is %d, copy_valid_number is %d, pos in copy is %d,  read %d, write %d, erase %d, read_inner %d, write inner %d, read_write_end %d, last_page_in_victim %d, victim_block_freepage %d, chip_status %d, sign for_preemptive %d, chip_next_predict_time %lld\n", gc_node->chip, gc_node->die, gc_node->plane, gc_node->victim_block_number,gc_node->copy_page_number, gc_node->copy_valid_number, gc_node->pos_in_copy, gc_node->read, gc_node->write, gc_node->erase, gc_node->gc_read_inner, gc_node->gc_write_inner, gc_node->read_write_end, gc_node->last_page_in_victim_deal, gc_node->victim_block_freepage_statistic, gc_node->chip_status, gc_node->sign_for_preemptive, gc_node->chip_next_predict_time_diff);

}




int printf_every_chip_static_subrequest(struct ssd_info *ssd)
{
    unsigned int tmp_channel,tmp_chip, i, j;
    unsigned int temp_chip_read_queue_length[100][100];
    unsigned int temp_chip_write_queue_length[100][100];
    struct sub_request *sub_for_queue = NULL;
    struct sub_request *sub_for_read = NULL;
    struct sub_request *sub_for_write = NULL;
#ifdef DEBUG
    //    printf("this simulator has %d channel, and %d chips in each channel, total chips is %d.\n",ssd->parameter->channel_number, ssd->channel_head[i].chip, ssd->parameter->chip_num);
#endif

    for(i = 0; i < 100; i++ ){
        for(j = 0; j < 100; j++ ){
            temp_chip_read_queue_length[i][j] = 0;
            temp_chip_write_queue_length[i][j] = 0;
        }
    }
    //   printf("initial ok\n");
    for(i = 0; i < ssd->parameter->channel_number; i++ ){
        //for read request count here.
        sub_for_read = ssd->channel_head[i].subs_r_head;
        while(sub_for_read != NULL){
            tmp_channel = sub_for_read->location->channel;
            tmp_chip = sub_for_read->location->chip;
            temp_chip_read_queue_length[tmp_channel][tmp_chip] ++;


            sub_for_read = sub_for_read->next_node;

        }//end of read while.

        //for write request count here.
        sub_for_write = ssd->channel_head[i].subs_w_head;
        while(sub_for_write != NULL){
#ifdef DEBUG
            //printf("sub_write address %lld\n",sub_for_write);
#endif


            tmp_channel = sub_for_write->location->channel;
            tmp_chip = sub_for_write->location->chip;
            temp_chip_write_queue_length[tmp_channel][tmp_chip] ++;


            sub_for_write = sub_for_write->next_node;

        }//end of write while.

    }//end of channel loop
    tmp_channel = ssd->parameter->channel_number;
    tmp_chip = ssd->parameter->chip_num / ssd->parameter->channel_number;
    printf("0,");
    for(i = 0; i < tmp_channel; i++ ){
        for(j = 0; j < tmp_chip; j++ ){
            printf(" %d,",temp_chip_write_queue_length[i][j]);

            //            printf("0, %d, %d, %d\n", i, j, temp_chip_write_queue_length[i][j]);
            //            printf("1, %d, %d, %d\n", i, j, temp_chip_read_queue_length[i][j]);

        }
    }
        printf("\n");

        printf("1,");
    for(i = 0; i < tmp_channel; i++ ){
        for(j = 0; j < tmp_chip; j++ ){
            printf(" %d,", temp_chip_read_queue_length[i][j]);
        }

    }

        printf("\n");



    for(i = 0; i < tmp_channel; i++ ){
#ifdef DEBUG  
        printf("channel %d 's sub_r_tail address is %lld\n", i, ssd->channel_head[i].subs_r_tail);
        printf("channel %d 's sub_r_head address is %lld\n", i, ssd->channel_head[i].subs_r_head);
        printf("channel %d 's sub_w_head address is %lld\n", i, ssd->channel_head[i].subs_w_head);
        printf("channel %d 's sub_w_tail address is %lld\n", i, ssd->channel_head[i].subs_w_tail);
#endif
        for(j = 0; j < tmp_chip; j++ ){
        //    printf("collection1 channel %d, chip %d, has %d read sub-requests.\n",i,j, temp_chip_read_queue_length[i][j]);
        //    printf("collection1 channel %d, chip %d, has %d write sub-requests.\n",i,j, temp_chip_write_queue_length[i][j]);

//            printf("0, %d, %d, %d\n", i, j, temp_chip_write_queue_length[i][j]);
//            printf("1, %d, %d, %d\n", i, j, temp_chip_read_queue_length[i][j]);


#ifdef DEBUGQUEUE
//            printf("collection1 channel %d, chip %d, has %d read sub-requests.\n",i,j, temp_chip_read_queue_length[i][j]);
//            printf("collection1 channel %d, chip %d, has %d write sub-requests.\n",i,j, temp_chip_write_queue_length[i][j]);
//            printf("channel %d, chip %d, now,num_w_cycle %d \n",i,j, ssd->channel_head[i].chip_head[j].num_w_cycle);
//            printf("channel %d, chip %d, chip_read_length  %d \n",i,j, ssd->channel_head[i].chip_head[j].chip_read_queue_length);
//            printf("channel %d, chip %d, chip_write_length  %d \n",i,j, ssd->channel_head[i].chip_head[j].chip_write_queue_length);



            //*****************here only for head of queue's state.***********//
            sub_for_queue = ssd->channel_head[i].subs_r_head;

            while(sub_for_queue != NULL){
                if(sub_for_queue->location->chip == j){
                    break;
                }
                sub_for_queue = sub_for_queue->next_node;
            }
            if(sub_for_queue != NULL){
                printf("collection2 channel %d, chip %d, first head sub_read_request state %d\n", i, j, sub_for_queue->current_state);
            }else {
                printf("collection2 channel %d, chip %d, has no sub_read_request now\n", i, j);
            }


            sub_for_queue = ssd->channel_head[i].subs_w_head;

            while(sub_for_queue != NULL){
                if(sub_for_queue->location->chip == j){
                    break;
                }
                sub_for_queue = sub_for_queue->next_node;
            }
            if(sub_for_queue != NULL){
                printf("collection2 channel %d, chip %d, first head sub_write_request state %d\n", i, j, sub_for_queue->current_state);
            }else {
                printf("collection2 channel %d, chip %d, has no sub_write_request now\n", i, j);
            }

            //*****************here only for head of queue's state.***********//
#endif
            if(temp_chip_read_queue_length[i][j] != ssd->channel_head[i].chip_head[j].chip_read_queue_length) {
                printf("lxc,read length is wrong here\n");
            }
            if(temp_chip_write_queue_length[i][j] != ssd->channel_head[i].chip_head[j].chip_write_queue_length) {
                printf("lxc,write length is wrong here\n");
            }
        }
    }
}


int printf_every_chip_read_subrequest(struct ssd_info *ssd)
{
    unsigned int tmp_channel,tmp_chip, i, j;
    unsigned int temp_chip_read_queue_length[100][100];
    unsigned int temp_chip_write_queue_length[100][100];
    struct sub_request *sub_for_queue = NULL;
    struct sub_request *sub_for_read = NULL;
    struct sub_request *sub_for_write = NULL;
#ifdef DEBUG
    //    printf("this simulator has %d channel, and %d chips in each channel, total chips is %d.\n",ssd->parameter->channel_number, ssd->channel_head[i].chip, ssd->parameter->chip_num);
#endif

    for(i = 0; i < 100; i++ ){
        for(j = 0; j < 100; j++ ){
            temp_chip_read_queue_length[i][j] = 0;
            temp_chip_write_queue_length[i][j] = 0;
        }
    }
    //   printf("initial ok\n");
    for(i = 0; i < ssd->parameter->channel_number; i++ ){
        //for read request count here.
        sub_for_read = ssd->channel_head[i].subs_r_head;
        while(sub_for_read != NULL){
            tmp_channel = sub_for_read->location->channel;
            tmp_chip = sub_for_read->location->chip;
            temp_chip_read_queue_length[tmp_channel][tmp_chip] ++;


            sub_for_read = sub_for_read->next_node;

        }//end of read while.

        //for write request count here.
        sub_for_write = ssd->channel_head[i].subs_w_head;
        while(sub_for_write != NULL){

            //printf("sub_write address %lld\n",sub_for_write);



            tmp_channel = sub_for_write->location->channel;
            tmp_chip = sub_for_write->location->chip;
            temp_chip_write_queue_length[tmp_channel][tmp_chip] ++;


            sub_for_write = sub_for_write->next_node;

        }//end of write while.

    }//end of channel loop
    tmp_channel = ssd->parameter->channel_number;
    tmp_chip = ssd->parameter->chip_num / ssd->parameter->channel_number;

    for(i = 0; i < tmp_channel; i++ ){
#ifdef DEBUG  
        printf("channel %d 's sub_r_tail address is %lld\n", i, ssd->channel_head[i].subs_r_tail);
        printf("channel %d 's sub_r_head address is %lld\n", i, ssd->channel_head[i].subs_r_head);
        printf("channel %d 's sub_w_head address is %lld\n", i, ssd->channel_head[i].subs_w_head);
        printf("channel %d 's sub_w_tail address is %lld\n", i, ssd->channel_head[i].subs_w_tail);
#endif
        for(j = 0; j < tmp_chip; j++ ){
//#ifdef DEBUGQUEUE
            if(temp_chip_read_queue_length[i][j] > 0){
                //printf("channel %d, chip %d, has %d read sub-requests.\n",i,j, temp_chip_read_queue_length[i][j]);
              //  printf("channel %d, chip %d, has %d write sub-requests.\n",i,j, temp_chip_write_queue_length[i][j]);
            //    printf("channel %d, chip %d, now,num_w_cycle %d \n",i,j, ssd->channel_head[i].chip_head[j].num_w_cycle);
               // printf("channel %d, chip %d, chip_read_length  %d \n",i,j, ssd->channel_head[i].chip_head[j].chip_read_queue_length);
            }
//            //*****************here only for head of queue's state.***********//
//            sub_for_queue = ssd->channel_head[i].subs_r_head;
//
//            while(sub_for_queue != NULL){
//                if(sub_for_queue->location->chip == j){
//                    break;
//                }
//                sub_for_queue = sub_for_queue->next_node;
//            }
//            if(sub_for_queue != NULL){
//                printf("channel %d, chip %d, first head sub_read_request state %d\n", i, j, sub_for_queue->current_state);
//            }else {
//                printf("channel %d, chip %d, has no sub_read_request now\n", i, j);
//            }
//
//
//            sub_for_queue = ssd->channel_head[i].subs_w_head;
//
//            while(sub_for_queue != NULL){
//                if(sub_for_queue->location->chip == j){
//                    break;
//                }
//                sub_for_queue = sub_for_queue->next_node;
//            }
//            if(sub_for_queue != NULL){
//                printf("channel %d, chip %d, first head sub_write_request state %d\n", i, j, sub_for_queue->current_state);
//            }else {
//                printf("channel %d, chip %d, has no sub_write_request now\n", i, j);
//            }
//
            //*****************here only for head of queue's state.***********//
//#endif
          //  printf("channel %d, chip %d, chip_write_length  %d \n",i,j, ssd->channel_head[i].chip_head[j].chip_write_queue_length);
            if(temp_chip_read_queue_length[i][j] != ssd->channel_head[i].chip_head[j].chip_read_queue_length) {
                printf("lxc,read length is wrong here, new temp length is %d. write in old chip is %d \n",temp_chip_read_queue_length[i][j], ssd->channel_head[i].chip_head[j].chip_read_queue_length);
            }
            if(temp_chip_write_queue_length[i][j] != ssd->channel_head[i].chip_head[j].chip_write_queue_length) {
    //            printf("lxc,write length is wrong here\n");

                printf("lxc,write length is wrong here, new temp length is %d. write in old chip is %d \n",temp_chip_write_queue_length[i][j], ssd->channel_head[i].chip_head[j].chip_write_queue_length);
            }
        }
    }
}




int judging_read_in_gc_chip(struct ssd_info *ssd, int channel)
{   unsigned int tmp_chip;
    struct gc_operation *gc_node;
    struct sub_request *sub_for_read = NULL;
    if(ssd->channel_head[channel].gc_command != NULL){
//        printf("now judging gc, channel is %d\n", channel);
        tmp_chip = ssd->channel_head[channel].gc_command->chip;// because now channel GC only has one GC chip once time.
        sub_for_read = ssd->channel_head[channel].subs_r_head;

        while(sub_for_read != NULL){
            if (sub_for_read->location->chip == tmp_chip){

  //      printf("now has read, so begin interrupt,!!!!!\n");
                return 0;

            } 
            sub_for_read = sub_for_read->next_node;

        }
        if(sub_for_read == NULL){

//        printf("now no read, jumpout from judging\n");
            return 1;
        }
    }
}



void printf_ssd_gc_node(struct ssd_info *ssd)
{
//    struct gc_operation *gc_node = NULL;
//    int i =0;
//    for(i = 0; i < ssd->parameter->channel_number; i++ ){
//    gc_node = ssd->channel_head[i].gc_command;
//#ifdef DEBUG
//printf("now is in gc_node information\n");
//#endif
//if(gc_node == NULL ){
//    continue;
//
//}
//    while(gc_node != NULL){
//        printf("gc_request %d,gc_node chip %d ,die %d, plane %d, block %d, page %d, state %d, priority %d, block and page only for interruptible gc record\n ", ssd->gc_request, gc_node->chip, gc_node->die, gc_node->plane, gc_node->block, gc_node->page, gc_node->state, gc_node->priority);
//
//        printf("the sign of the read %d, write %d , erase %d , victim_block_num %d, read_inner %d, write_inner %d,read_write_end %d, freepage_statistic %d\n ",  gc_node->read, gc_node->write, gc_node->erase, gc_node->victim_block_number, gc_node->gc_read_inner, gc_node->gc_write_inner, gc_node->read_write_end, gc_node->victim_block_freepage_statistic);
//        gc_node = gc_node->next_node;
//    }
//    }
}




void printf_ssd_request_queue(struct ssd_info *ssd)
{
    struct request* temp_request;
    unsigned int temp_num_request = ssd->request_queue_length;
    if(temp_num_request == 0){
        printf("ssd request_queue has none\n");
    }else{
        temp_request = ssd->request_queue;
        printf("ssd request_queue status: request_length is %d,\n", ssd->request_queue_length);
        while(temp_request != NULL){
           printf("ssd request_queue , lsn : %d, size : %d, operation :%d   ***1read,0write,complete_lsn_count :%d, distri_flag :%d, begin_time :%llu\n",temp_request->lsn,temp_request->size,temp_request->operation,temp_request->complete_lsn_count,temp_request->distri_flag,temp_request->begin_time);
            if(temp_request == ssd->request_tail){
                break;
            }else{
                temp_request = temp_request->next_node;
            }
        }

    }
}

//注意，sub_request是在channel结构体中的，而并不是按照chip分的。。////
/////////////add for status  debug lxc ////////////
void printfsub_request_status(struct ssd_info *ssd)
{
//    unsigned int i,j,k;
//    struct sub_request *sub = NULL , *p =NULL;
//
//    for(i=0; i < ssd->parameter->channel_number; i++){
//
//        printf("channel: %d,////channel status:: current_state: %d, current_time: %llu, next_state: %d, next_state_predict_time: %llu \n",i,ssd->channel_head[i].current_state, ssd->channel_head[i].current_time, ssd->channel_head[i].next_state, ssd->channel_head[i].next_state_predict_time);
//
//        printf("chip 0:////chip status:: current_state:%d, current_time: %llu, next_state: %d, next_state_predict_time: %llu \n",ssd->channel_head[i].chip_head[0].current_state,  ssd->channel_head[i].chip_head[0].current_time, ssd->channel_head[i].chip_head[0].next_state, ssd->channel_head[i].chip_head[0].next_state_predict_time);
//
//        printf("chip 1:////chip status:: current_state:%d, current_time: %llu, next_state: %d, next_state_predict_time: %llu \n",ssd->channel_head[i].chip_head[1].current_state,  ssd->channel_head[i].chip_head[1].current_time, ssd->channel_head[i].chip_head[1].next_state, ssd->channel_head[i].chip_head[1].next_state_predict_time);
//        //printf("sub is 0x%x, ssd->channel_head[%d].subs_r_tail is 0x%x , ssd->channel_head[i].subs_r_head is 0x%x\n",sub, i,( ssd->channel_head[i]).subs_r_tail,  ( ssd->channel_head[i]).subs_r_head);
//        if((ssd->channel_head[i]).subs_r_head != NULL){
//            sub = (ssd->channel_head[i]).subs_r_head;
//         //   printf("i is %d, == channel is %d, NULL is %d\n ",i, sub->location->channel,NULL);
//            //printf("11 if\n");
//        }else{
//            sub = NULL;
//            //printf("11 else\n");
//        }
//
//        while(sub != NULL){
//            printf("read operation. chip: %d, lpn: %d, ppn: %d  operation: %d,size: %d,   ////sub_request status:: current_state: %d, current_time: %llu, next_state: %d, next_state_predict_time: %llu, state(only for write): %d\n", sub->location->chip, sub->lpn, sub->ppn,sub->operation, sub->size, sub->current_state, sub->current_time,sub->next_state, sub->next_state_predict_time, sub->state) ;
//        //    printf("sub is 0x%x, ssd->channel_head[%d].subs_r_tail is 0x%x , ssd->channel_head[i].subs_r_head is 0x%x\n",sub,i ,( ssd->channel_head[i]).subs_r_tail,  ( ssd->channel_head[i]).subs_r_head);
//            p = sub;
//            if(sub == (ssd->channel_head[i]).subs_r_tail){
//                //         printf("break\n");
//                break;
//            }else{
//                //       printf("else\n");
//                sub = sub->next_node;
//            }
//        }
//
//
//
//        //print write queue
//        if((ssd->channel_head[i]).subs_w_head != NULL){
//            sub = (ssd->channel_head[i]).subs_w_head;
//            //printf("i is %d, == channel is %d, NULL is %d\n ",i, sub->location->channel,NULL);
//            //printf("11 if\n");
//        }else{
//            sub = NULL;
//            //printf("11 else\n");
//        }
//
//
//        while(sub != NULL){
//            printf("write operation.  chip: %d, lpn: %llu, ppn: %llu  operation: %d,size: %d,   ////sub_request status:: current_state: %d, current_time: %llu, next_state: %d, next_state_predict_time: %llu, state(only for write): %d\n", sub->location->chip, sub->lpn, sub->ppn,sub->operation, sub->size, sub->current_state, sub->current_time,sub->next_state, sub->next_state_predict_time, sub->state) ;
//            p = sub;
//            if(sub == ssd->channel_head[i].subs_w_tail){
//                break;
//            }else{
//                sub = sub->next_node;
//            }
//        }
//
//
//
//    }
}





/////////////add for status  debug lxc ////////////


/********************************************************************************************************************************
1，main函数中initiatio()函数用来初始化ssd,；2，make_aged()函数使SSD成为aged，aged的ssd相当于使用过一段时间的ssd，里面有失效页，
non_aged的ssd是新的ssd，无失效页，失效页的比例可以在初始化参数中设置；3，pre_process_page()函数提前扫一遍读请求，把读请求
的lpn<--->ppn映射关系事先建立好，写请求的lpn<--->ppn映射关系在写的时候再建立，预处理trace防止读请求是读不到数据；4，simulate()是
核心处理函数，trace文件从读进来到处理完成都由这个函数来完成；5，statistic_output()函数将ssd结构中的信息输出到输出文件，输出的是
统计数据和平均数据，输出文件较小，trace_output文件则很大很详细；6，free_all_node()函数释放整个main函数中申请的节点
*********************************************************************************************************************************/
int main() {
    unsigned int i, j, k;
    struct ssd_info *ssd;


#ifdef DEBUG
    printf("enter main\n");
    printf("for test\n");
#endif
    ssd = (struct ssd_info *) malloc(sizeof(struct ssd_info));
    alloc_assert(ssd, "ssd");
    memset(ssd, 0, sizeof(struct ssd_info));
    ssd = initiation(ssd);
#ifdef DEBUG
    printf("this is in main()");
    printfsub_request_status(ssd); 
#endif
    make_aged(ssd);
    pre_process_page(ssd);

    for (i = 0; i < ssd->parameter->channel_number; i++) {
        for (j = 0; j < ssd->parameter->die_chip; j++) {
            for (k = 0; k < ssd->parameter->plane_die; k++) {
                printf("%d,0,%d,%d:  %5d\n", i, j, k,
                       ssd->channel_head[i].chip_head[0].die_head[j].plane_head[k].free_page);
            }
        }
    }
    fprintf(ssd->outputfile, "\t\t\t\t\t\t\t\t\tOUTPUT\n");
    fprintf(ssd->outputfile, "****************** TRACE INFO ******************\n");

    ssd = simulate(ssd);

    //trace_output(ssd);
    statistic_output(ssd);
/*	free_all_node(ssd);*/

    printf("\n");
    printf("the simulation is completed!\n");

    return 1;
/* 	_CrtDumpMemoryLeaks(); */
}


/******************simulate() *********************************************************************
*simulate()是核心处理函数，主要实现的功能包括
*1,从trace文件中获取一条请求，挂到ssd->request
*2，根据ssd是否有dram分别处理读出来的请求，把这些请求处理成为读写子请求，挂到ssd->channel或者ssd上
*3，按照事件的先后来处理这些读写子请求。
*4，输出每条请求的子请求都处理完后的相关信息到outputfile文件中
**************************************************************************************************/
struct ssd_info *simulate(struct ssd_info *ssd) {
    int flag = 1, flag1 = 0;
    double output_step = 0;
    unsigned int a = 0, b = 0;
    //errno_t err;

    printf("\n");
    printf("begin simulating.......................\n");
    printf("\n");
    printf("\n");
    printf("   ^o^    OK, please wait a moment, and enjoy music and coffee   ^o^    \n");

    ssd->tracefile = fopen(ssd->tracefilename, "r");
    if (ssd->tracefile == NULL) {
        printf("the trace file can't open\n");
        return NULL;
    }

    fprintf(ssd->outputfile,
            "      arrive           lsn     size ope     begin time    response time    process time\n");
    fflush(ssd->outputfile);
    int i = 0;
    while (flag != 100) {


//just has GC, then printf begin. 
        if(ssd->debug_sign == 1){
//            printf("every get request can debug now\n");
//#define DEBUG 
//#define DEBUGQUEUE 
//#define DEBUGSUSPEND
//#define DEBUGTRACE_OUT 
//#define DEBUG_CURR_TIME
        }


        //printf("%dth\n", i++);
        flag = get_requests(ssd);//lxcv1  get_requests means get trace io and then creat request then insert it into aad->request_queue。
        /*********************************todo**************************************/

//    printf( "flag is %d\n" ,flag);
#ifdef DEBUG //lxcv1
    printf( "flag is %d\n" ,flag);
#endif

        if (flag == 1) {
            //printf("once\n");

            //printf_every_chip_static_subrequest(ssd);
            if (ssd->parameter->dram_capacity != 0) {
                buffer_management(ssd);
                distribute(ssd);
            } else {
                no_buffer_distribute(ssd);
            }
        }
        /*******************************todo****************************************/
        //lxcprogram_gc statistic here
        //printf_every_chip_static_subrequest(ssd);
        process(ssd);
        trace_output(ssd);
        //printf("flag = %d, ssd->request_queue = %d\n", flag, ssd->request_queue_length);
        if (flag == 0 )
            flag = 100;
    }

    fclose(ssd->tracefile);
    return ssd;
}


/********    get_request    ******************************************************
*	1.get requests that arrived already
*	2.add those request node to ssd->reuqest_queue
*	return	0: reach the end of the trace
*			-1: no request has been added
*			1: add one request to list
*SSD模拟器有三种驱动方式:时钟驱动(精确，太慢) 事件驱动(本程序采用) trace驱动()，
*两种方式推进事件：channel/chip状态改变、trace文件请求达到。
*channel/chip状态改变和trace文件请求到达是散布在时间轴上的点，每次从当前状态到达
*下一个状态都要到达最近的一个状态，每到达一个点执行一次process
********************************************************************************/
int get_requests(struct ssd_info *ssd) {
    char buffer[200];
    int64_t lsn = 0;
    int device, size, ope, large_lsn, i = 0, j = 0;
    struct request *request1;
    struct sub_request *temp_sub;
    int flag = 1;
    long filepoint;
    int64_t time_t = 0;
    int64_t lsn_for_record = 0;
    int64_t nearest_event_time;
#ifdef DEBUG_CURR_TIME 
    printf("DEBUG_CURR_TIME is %lld in get_request function\n",ssd->current_time);
#endif


    if (feof(ssd->tracefile)){
        if(ssd->request_queue_length > 0){
            printf("here is dealing end of trace\n");
            nearest_event_time = find_nearest_event(ssd);

            if (nearest_event_time == MAX_INT64) {
                return -1;
            }
            if (nearest_event_time < MAX_INT64){

                ssd->current_time = nearest_event_time;
                return -1;
            }
        }

        return 0;
    }
    filepoint = ftell(ssd->tracefile);
    fgets(buffer, 200, ssd->tracefile);
    sscanf(buffer, "%lld %d %lld %d %d", &time_t, &device, &lsn, &size, &ope);

    if ((device < 0) && (lsn < 0) && (size < 0) && (ope < 0)) {
        return 100;
    }
    lsn_for_record = lsn;
#ifdef DEBUG //lxcv1
    printf("enter get_requests,  ssd->min_lsn %d, ssd->max_lsn %d\n",ssd->min_lsn,ssd->max_lsn);

    printf( "time_t %lld, device %d, lsn  %d,size %d ,ope %d\n" , time_t, device, lsn, size, ope);
#endif


    if (lsn < ssd->min_lsn)
        ssd->min_lsn = lsn;
    if (lsn > ssd->max_lsn)
        ssd->max_lsn = lsn;
    /******************************************************************************************************
    *上层文件系统发送给SSD的任何读写命令包括两个部分（LSN，size） LSN是逻辑扇区号，对于文件系统而言，它所看到的存
    *储空间是一个线性的连续空间。例如，读请求（260，6）表示的是需要读取从扇区号为260的逻辑扇区开始，总共6个扇区。
    *large_lsn: channel下面有多少个subpage，即多少个sector。overprovide系数：SSD中并不是所有的空间都可以给用户使用，
    *比如32G的SSD可能有10%的空间保留下来留作他用，所以乘以1-provide
    ***********************************************************************************************************/
    large_lsn = (int) ((ssd->parameter->subpage_page * ssd->parameter->page_block * ssd->parameter->block_plane *
                        ssd->parameter->plane_die * ssd->parameter->die_chip * ssd->parameter->chip_num) *
                       (1 - ssd->parameter->overprovide));
//    printf("large is %d,%lld, lsn is %d, %lld \n", large_lsn, large_lsn, lsn, lsn);
    lsn = lsn % large_lsn;
//    printf("changed_lsn is %d, %lld\n", lsn, lsn);
    nearest_event_time = find_nearest_event(ssd);
#ifdef DEBUG //lxcv1
    printf("nearest_event_time is %lld\n",nearest_event_time );
#endif

    if (nearest_event_time == MAX_INT64) {


        if(ssd->request_queue_length >0){
            printf("only gc processing can happen\n");
        }
        if(ssd->request_queue_length == ssd->parameter->queue_length){
//  if(ssd->request_queue_length >0){
//printf("ssd first request time is %lld\n",  );
            printf("full of requests in queue now!!\n");
            fseek(ssd->tracefile, filepoint, 0);

//                for (i = 0; i < ssd->parameter->channel_number; i++) {
//
//        printf("channel[%d].next_state is 0x%x,ssd->channel_head[%d].next_state_predict_time is %lld, channel[%d].current_state is 0x%x,ssd->channel_head[%d].current_time is %lld\n",i,ssd->channel_head[i].next_state,i,ssd->channel_head[i].next_state_predict_time  ,i,ssd->channel_head[i].current_state,i,ssd->channel_head[i].current_time  );
//
//        for (j = 0; j < ssd->parameter->chip_channel[i]; j++) {
//
//            printf("ssd->channel_head[%d].chip_head[%d].next_state_predict_time is %lld,  ssd->channel_head[%d].chip_head[%d].next_state is 0x%x  ssd->channel_head[%d].chip_head[%d].current_time is %lld,  ssd->channel_head[%d].chip_head[%d].current_state is 0x%x \n", i,j,ssd->channel_head[i].chip_head[j].next_state_predict_time ,i,j,ssd->channel_head[i].chip_head[j].next_state , i,j,ssd->channel_head[i].chip_head[j].current_time ,i,j,ssd->channel_head[i].chip_head[j].current_state  );
//
//        }
//
//    }
//
//                printf_ssd_request_queue(ssd);
//                for (i = 0; i < ssd->parameter->channel_number; i++){
//                    temp_sub = ssd->channel_head[i].subs_w_head;
//                    while (temp_sub != NULL){
//                        printf("channel %d sub is %lld, sub current_state is %d\n", i, temp_sub, temp_sub->current_state);
//                        temp_sub = temp_sub->next_subs;
//
//                    }
//                    temp_sub = ssd->channel_head[i].subs_r_head;
//                    while (temp_sub != NULL){
//                        printf("channel %d sub is %lld\n", i, temp_sub);
//                        temp_sub = temp_sub->next_subs;
//                    }
//
//
//                }// end of channel loop
                return -1;
        }


        ssd->current_time = time_t;
//        if(ssd->request_queue_length > 0){// this is wrong, for if gc happen, there will no write. so request will stay here.and maybe add another program.
//            printf("error is too biggest!!!\n");
//        }

    printf("ssdcurrent_time  nter nearest_event_time is MAX_INIT64 and ssd->current_time is %lld\n", ssd->current_time );
#ifdef DEBUG //lxcv1
    printf("ssdcurrent_time  nter nearest_event_time is MAX_INIT64 and ssd->current_time is %lld\n", ssd->current_time );
#endif

        //if (ssd->request_queue_length>ssd->parameter->queue_length)    //如果请求队列的长度超过了配置文件中所设置的长度
        //{
        //printf("error in get request , the queue length is too long\n");
        //}
    } else {
        if (nearest_event_time < time_t) {
            /*******************************************************************************
            *回滚，即如果没有把time_t赋给ssd->current_time，则trace文件已读的一条记录回滚
            *filepoint记录了执行fgets之前的文件指针位置，回滚到文件头+filepoint处
            *int fseek(FILE *stream, long offset, int fromwhere);函数设置文件指针stream的位置。
            *如果执行成功，stream将指向以fromwhere（偏移起始位置：文件头0，当前位置1，文件尾2）为基准，
            *偏移offset（指针偏移量）个字节的位置。如果执行失败(比如offset超过文件自身大小)，则不改变stream指向的位置。
            *文本文件只能采用文件头0的定位方式，本程序中打开文件方式是"r":以只读方式打开文本文件
            **********************************************************************************/
#ifdef DEBUG //lxcv1
    printf("enter nearest_event_time <time_t\n" );
#endif

            fseek(ssd->tracefile, filepoint, 0);
            if (ssd->current_time <= nearest_event_time){

//                printf("trace line is later, dont have to add. ssd->CURR_TIME is %lld, nearest_event_time is %lld in get_requests function\n",ssd->current_time, nearest_event_time);
                ssd->current_time = nearest_event_time;

#ifdef DEBUG_CURR_TIME 
                printf("ssdcurrenttime DEBUG_CURR_TIME is %lld in get_requests function\n",ssd->current_time);
#endif

            }
            return -1;
        } else {
#ifdef DEBUG //lxcv1
    printf("enter nearest_event_time > time_t\n" );
#endif

            if (ssd->request_queue_length >= ssd->parameter->queue_length) {
//                printf("here request long enough!!!, length is %d\n", ssd->request_queue_length);

                fseek(ssd->tracefile, filepoint, 0);
                ssd->current_time = nearest_event_time;
#ifdef DEBUG_CURR_TIME 
                printf("ssdcurrenttime  DEBUG_CURR_TIME is %lld in get_requests function and request_queue>=parameter queue_length\n",ssd->current_time);
#endif




            ///*****************judging for times by queue stuffing with only write requests***********************///
            // judging for times stuffing by full queue only with write request.
            unsigned int temp_i;
            struct request *request1;
            struct gc_operation *temp_gc_node;
            unsigned int temp_gc_sign = 0;
            if(ssd->request_queue != NULL){
                request1 = ssd->request_queue;
            }
            for(temp_i = 0; temp_i < ssd->request_queue_length; temp_i ++){

                if(request1->operation != 0 ){
                    request1 = request1->next_node;
                }else{

                    break;
                }
            }//end of for
            if(temp_i == ssd->request_queue_length){
                if(ssd->temp_lba != lsn_for_record ){
                    ssd->temp_lba = lsn_for_record ;

                    ssd->interference_stuffed_length ++;
                    printf("fullreaddeal inner subrequest, current_time %lld, temp_lba = %lld, interference_stuffed_length is %d \n", ssd->current_time, ssd->temp_lba, ssd->interference_stuffed_length);

                    for(temp_i = 0; temp_i < ssd->parameter->channel_number; temp_i ++ ){

                        temp_gc_node = ssd->channel_head[i].gc_command;
                        if(temp_gc_node != NULL){
                            temp_gc_sign = 1;
                            break;
                        }
                    }
                    if(temp_gc_sign == 1){
                        ssd->interference_stuffed_length_during_gc ++;
                        temp_gc_sign = 0;
                        printf("fullreadrequeststuffed during gc is %d\n", ssd->interference_stuffed_length_during_gc);
                    }
                }
            }


            ///*****************judging for times by queue stuffing with only write requests***********************///




            ///*****************judging for times by queue stuffing with only write requests***********************///
            // judging for times stuffing by full queue only with write request.
          //  unsigned int temp_i;
          //  struct request *request1;
          //  struct gc_operation *temp_gc_node;
          //  unsigned int temp_gc_sign = 0;
            if(ssd->request_queue != NULL){
                request1 = ssd->request_queue;
            }
            for(temp_i = 0; temp_i < ssd->request_queue_length; temp_i ++){

                if(request1->operation != 1 ){
                    request1 = request1->next_node;
                }else{

                    break;
                }
            }//end of for
            if(temp_i == ssd->request_queue_length){
                if(ssd->temp_lba != lsn_for_record ){
                    ssd->temp_lba = lsn_for_record ;

                    ssd->interference_stuffed_length ++;
                    printf("fullwritedeal inner subrequest, current_time %lld, temp_lba = %lld, interference_stuffed_length is %d \n", ssd->current_time, ssd->temp_lba, ssd->interference_stuffed_length);

                    for(temp_i = 0; temp_i < ssd->parameter->channel_number; temp_i ++ ){

                        temp_gc_node = ssd->channel_head[i].gc_command;
                        if(temp_gc_node != NULL){
                            temp_gc_sign = 1;
                            break;
                        }
                    }
                    if(temp_gc_sign == 1){
                        ssd->interference_stuffed_length_during_gc ++;
                        temp_gc_sign = 0;
                        printf("fullwritestuffed during gc is %d\n", ssd->interference_stuffed_length_during_gc);
                    }
                }
            }


            ///*****************judging for times by queue stuffing with only write requests***********************///





                return -1;
            } else {
                ssd->current_time = time_t;
#ifdef DEBUG_CURR_TIME 
                printf("ssdcurrenttime  DEBUG_CURR_TIME is %lld in get_requests function request_queue<=parameter queue_length \n",ssd->current_time);
#endif

            }
        }
    }

    if (time_t < 0) {
        printf("error!\n");
        while (1) {}
    }

    if (feof(ssd->tracefile)) {
        request1 = NULL;
        return 0;
    }

    request1 = (struct request *) malloc(sizeof(struct request));
    alloc_assert(request1, "request");
    memset(request1, 0, sizeof(struct request));

    request1->time = time_t;
    request1->lsn = lsn;
    request1->size = size;
    request1->operation = ope;
    request1->begin_time = time_t;
    request1->response_time = 0;
    request1->energy_consumption = 0;
    request1->next_node = NULL;
    request1->distri_flag = 0;              // indicate whether this request has been distributed already
    request1->subs = NULL;
    request1->need_distr_flag = NULL;
    request1->complete_lsn_count = 0;         //record the count of lsn served by buffer
    filepoint = ftell(ssd->tracefile);        // set the file point
#ifdef DEBUG
    printf("before insert into ssd-request queue\n");
    printf_ssd_request_queue(ssd);
#endif


 //  printf("ssd request_queue , lsn : %d, size : %d, operation :%d   ***1read,0write,complete_lsn_count :%d, distri_flag :%d, begin_time :%llu\n",request1->lsn,request1->size,request1->operation,request1->complete_lsn_count,request1->distri_flag,request1->begin_time);
#ifdef DEBUGSUSPEND
   printf("ssd request_queue , lsn : %d, size : %d, operation :%d   ***1read,0write,complete_lsn_count :%d, distri_flag :%d, begin_time :%llu\n",request1->lsn,request1->size,request1->operation,request1->complete_lsn_count,request1->distri_flag,request1->begin_time);
#endif

    if (ssd->request_queue == NULL)          //The queue is empty
    {
        ssd->request_queue = request1;
        ssd->request_tail = request1;
        ssd->request_queue_length++;
        
    } else {
        (ssd->request_tail)->next_node = request1;
        ssd->request_tail = request1;
        ssd->request_queue_length++;
    }
//lxcprogram_for_adjust_write_suspension_number

//    no_greedy_RF_suspension(ssd);

//lxcprogram_for_adjust_write_suspension_number



/*****************************only record longest queue length*************************/

if(ssd->longest_queue_record < ssd->request_queue_length){
    ssd->longest_queue_record = ssd->request_queue_length;

}


    
/*****************************only record longest queue length*************************/



#ifdef DEBUG
    printf("after inserted into ssd-request queue\n");
    printf_ssd_request_queue(ssd);
#endif
    if (request1->operation == 1)             //计算平均请求大小 1为读 0为写
    {
        ssd->ave_read_size =
                (ssd->ave_read_size * ssd->read_request_count + request1->size) / (ssd->read_request_count + 1);
    } else {
        ssd->ave_write_size =
                (ssd->ave_write_size * ssd->write_request_count + request1->size) / (ssd->write_request_count + 1);
    }


    filepoint = ftell(ssd->tracefile);
    fgets(buffer, 200, ssd->tracefile);    //寻找下一条请求的到达时间
    sscanf(buffer, "%lld %d %d %d %d", &time_t, &device, &lsn, &size, &ope);
    ssd->next_request_time = time_t;
    fseek(ssd->tracefile, filepoint, 0);

    return 1;
}

int no_greedy_RF_suspension(struct ssd_info *ssd)
{

//lxcprogram_for_adjust_write_suspension_number

//unsigned int chip_write_record_begin;
//unsigned int chip_write_record_end;
//unsigned int chip_read_stop;
    unsigned int temp_channel,temp_chip, temp_i, temp_j, temp_num_write;
    struct sub_request *temp_sub_for_write = NULL;
    struct gc_operation* temp_gc_node;
    unsigned int chip_has_write = 0;
    unsigned int temp_length = 0;
    temp_channel = ssd->parameter->channel_number;
    temp_chip = ssd->parameter->chip_num / ssd->parameter->channel_number;
    temp_length =  WRITE_RATIO_IN_REQUEST_QUEUE * ssd->parameter->queue_length;


//    if(request->operation == 0){
        // statistic for write requests number
        temp_num_write = statistic_write_request_number(ssd);
        if(temp_num_write == ssd->request_queue_length){
            return 1;
        }
        printf("temp_num_write is %d, and setting write length is %d, and longest length is %d\n", temp_num_write, temp_length, ssd->parameter->queue_length);
        if( temp_num_write >= temp_length ){

            //1)only  the first serviced write corresponding  chips should be set as chip_read_stop for further schedule, and record every chip's write record begin.  // here the most important is that the one chip write subrequest serviced sequence is depend on the chip write sequence. but the original design set the service sequence as the die first. 
            //2) if there is gc, then also dont set the chip_read_stop sign value. this setting is only for normal write instead of GC process.

            // step1, now in order to move write faster, so here choose all chips's first write sub-request to serviced. not only choose the being serviced write request's corresponding chips. 

            for(temp_i = 0; temp_i < temp_channel; temp_i ++ ){

                temp_j = 0;
                temp_gc_node = ssd->channel_head[temp_i].gc_command;

                // if there is gc then skip the write adjusting.
                if(temp_gc_node != NULL){
                    continue;
                }

                while (temp_j < temp_chip){

                    temp_sub_for_write  = ssd->channel_head[temp_i].subs_w_head;
                    printf("chip is %d\n", temp_j);
                    while(temp_sub_for_write != NULL){
                        //if chip was already set, then jump out this chip. 
                        if(ssd->channel_head[temp_i].chip_head[temp_j].chip_read_stop == 1){
                            chip_has_write = 1;
                            temp_j ++;
                            break;

                        }

                        //normally set write interval.
                        if(temp_sub_for_write->location->chip == temp_j){

                            //step2, after finding out the destination chip, then set the chip's chip_write_record_begin and add temp_j number
                            ssd->channel_head[temp_i].chip_head[temp_j].chip_write_record_begin = ssd->channel_head[temp_i].chip_head[temp_j].num_w_cycle;
                            ssd->channel_head[temp_i].chip_head[temp_j].chip_read_stop = 1;
                            chip_has_write = 1;
                            printf("channel %d, chip %d, has record begin is %d\n", temp_i, temp_j, ssd->channel_head[temp_i].chip_head[temp_j].num_w_cycle);


                            temp_j ++;

                            break;
                        }else{
                            temp_sub_for_write = temp_sub_for_write->next_node;

                        }

                        printf("2222\n");
                    }//end of while

                    if(chip_has_write == 1){
                        chip_has_write = 0;
                    }else{
                        temp_j ++;
                    }
                }//end of chip number

                printf("11111\n");
            }//end channel for loop





    }//end of begin of adjusting.


//lxcprogram_for_adjust_write_suspension_number


return 1;

}

//lxcprogram_for_adjust_write_suspension_number
int  statistic_write_request_number(struct ssd_info *ssd)
{
    struct request* temp_request;
    unsigned int temp_num_write_request = 0;
//    unsigned int temp_i = 0;
    temp_request = ssd->request_queue;

    if(ssd->request_queue_length != 0){

        while(temp_request != NULL){
          
            if(temp_request->operation == 0){

                temp_num_write_request ++;
                if(temp_request == ssd->request_tail){
                    break;
                }

                temp_request = temp_request->next_node;

            }else{
                if(temp_request == ssd->request_tail){
                    break;
                }
                temp_request = temp_request->next_node;
            }
      

            printf("3333\n");
        }//end of while
    }
    return temp_num_write_request;
}
/**********************************************************************************************************************************************
*首先buffer是个写buffer，就是为写请求服务的，因为读flash的时间tR为20us，写flash的时间tprog为200us，所以为写服务更能节省时间
*  读操作：如果命中了buffer，从buffer读，不占用channel的I/O总线，没有命中buffer，从flash读，占用channel的I/O总线，但是不进buffer了
*  写操作：首先request分成sub_request子请求，如果是动态分配，sub_request挂到ssd->sub_request上，因为不知道要先挂到哪个channel的sub_request上
*          如果是静态分配则sub_request挂到channel的sub_request链上,同时不管动态分配还是静态分配sub_request都要挂到request的sub_request链上
*		   因为每处理完一个request，都要在traceoutput文件中输出关于这个request的信息。处理完一个sub_request,就将其从channel的sub_request链
*		   或ssd的sub_request链上摘除，但是在traceoutput文件输出一条后再清空request的sub_request链。
*		   sub_request命中buffer则在buffer里面写就行了，并且将该sub_page提到buffer链头(LRU)，若没有命中且buffer满，则先将buffer链尾的sub_request
*		   写入flash(这会产生一个sub_request写请求，挂到这个请求request的sub_request链上，同时视动态分配还是静态分配挂到channel或ssd的
*		   sub_request链上),在将要写的sub_page写入buffer链头
***********************************************************************************************************************************************/
struct ssd_info *buffer_management(struct ssd_info *ssd) {
    unsigned int j, lsn, lpn, last_lpn, first_lpn, index, complete_flag = 0, state, full_page;
    unsigned int flag = 0, need_distb_flag, lsn_flag, flag1 = 1, active_region_flag = 0;
    struct request *new_request;
    struct buffer_group *buffer_node, key;
    unsigned int mask = 0, offset1 = 0, offset2 = 0;
#ifdef DEBUG_CURR_TIME 
    printf("DEBUG_CURR_TIME is %lld in buffer_management function \n",ssd->current_time);
#endif

    ssd->dram->current_time = ssd->current_time;
    full_page = ~(0xffffffff << ssd->parameter->subpage_page);//目前是1111.

    new_request = ssd->request_tail;
    lsn = new_request->lsn;
    lpn = new_request->lsn / ssd->parameter->subpage_page;
    last_lpn = (new_request->lsn + new_request->size - 1) / ssd->parameter->subpage_page;
    first_lpn = new_request->lsn / ssd->parameter->subpage_page;

    new_request->need_distr_flag = (unsigned int *) malloc(
            sizeof(unsigned int) * ((last_lpn - first_lpn + 1) * ssd->parameter->subpage_page / 32 + 1));
//    printf("unsigned int length is %d\n",sizeof(unsigned int));
    alloc_assert(new_request->need_distr_flag, "new_request->need_distr_flag");
    memset(new_request->need_distr_flag, 0,
           sizeof(unsigned int) * ((last_lpn - first_lpn + 1) * ssd->parameter->subpage_page / 32 + 1));
#ifdef DEBUG
    printf("yes, here is the request_tail for new_request ,its lsn is %d, size is %d, operation is %d\n",new_request->lsn, new_request->size, new_request->operation);
#endif

    if (new_request->operation == READ) {
        while (lpn <= last_lpn) {
            /************************************************************************************************
             *need_distb_flag表示是否需要执行distribution函数，1表示需要执行，buffer中没有，0表示不需要执行
             *即1表示需要分发，0表示不需要分发，对应点初始全部赋为1
            *************************************************************************************************/
            need_distb_flag = full_page;
            key.group = lpn;
            buffer_node = (struct buffer_group *) avlTreeFind(ssd->dram->buffer,
                                                              (TREE_NODE *) &key);        // buffer node



            while ((buffer_node != NULL) && (lsn < (lpn + 1) * ssd->parameter->subpage_page) &&
                   (lsn <= (new_request->lsn + new_request->size - 1))) {
#ifdef DEBUG //lxcv1
            printf("buffer_node is not null, find in Tree the buffer_node information is :\n");

            printf("buffer_node. group %d, stored %d, dirty_clean %d, flag %d\n", buffer_node->group, buffer_node->stored, buffer_node->dirty_clean, buffer_node->flag);

#endif

                lsn_flag = full_page;
                mask = 1 << (lsn % ssd->parameter->subpage_page);
                if (mask > 31) {//此处一定是错误的。。lxc。
                    printf("the subpage number is larger than 32!add some cases");
                    getchar();
                } else if ((buffer_node->stored & mask) == mask) {
                    flag = 1;
                    lsn_flag = lsn_flag & (~mask);
#ifdef DEBUG //lxcv1
            printf("lsn_flag is 0x%x, buffer_node->stored？？？where is hole is 0x%x, mask is 0x%x\n",lsn_flag,  buffer_node->stored,mask);
//注意观察其中的stored是否有空洞？？？应该没有，都是整体没有的。
#endif

                }

                if (flag == 1) {    //如果该buffer节点不在buffer的队首，需要将这个节点提到队首，实现了LRU算法，这个是一个双向队列。
                    if (ssd->dram->buffer->buffer_head != buffer_node) {
                        if (ssd->dram->buffer->buffer_tail == buffer_node) {
                            buffer_node->LRU_link_pre->LRU_link_next = NULL;
                            ssd->dram->buffer->buffer_tail = buffer_node->LRU_link_pre;
                        } else {
                            buffer_node->LRU_link_pre->LRU_link_next = buffer_node->LRU_link_next;
                            buffer_node->LRU_link_next->LRU_link_pre = buffer_node->LRU_link_pre;
                        }
                        buffer_node->LRU_link_next = ssd->dram->buffer->buffer_head;
                        ssd->dram->buffer->buffer_head->LRU_link_pre = buffer_node;
                        buffer_node->LRU_link_pre = NULL;
                        ssd->dram->buffer->buffer_head = buffer_node;
                    }
                    ssd->dram->buffer->read_hit++;
                    new_request->complete_lsn_count++;
                } else if (flag == 0) {
                    ssd->dram->buffer->read_miss_hit++;
#ifdef DEBUG

                    printf("buffer->read_miss_hit++ is %d\n",ssd->dram->buffer->read_miss_hit);
#endif
                }

                need_distb_flag = need_distb_flag & lsn_flag;

                flag = 0;
                lsn++;
            }

            index = (lpn - first_lpn) / (32 / ssd->parameter->subpage_page);
            new_request->need_distr_flag[index] = new_request->need_distr_flag[index] | (need_distb_flag
                    << (((lpn - first_lpn) % (32 / ssd->parameter->subpage_page)) * ssd->parameter->subpage_page));
            lpn++;

        }
    } else if (new_request->operation == WRITE) {
        while (lpn <= last_lpn) {
            need_distb_flag = full_page;
            mask = ~(0xffffffff << (ssd->parameter->subpage_page));
            state = mask;

            if (lpn == first_lpn) {
                offset1 = ssd->parameter->subpage_page - ((lpn + 1) * ssd->parameter->subpage_page - new_request->lsn);
                state = state & (0xffffffff << offset1);
            }
            if (lpn == last_lpn) {
                offset2 = ssd->parameter->subpage_page -
                          ((lpn + 1) * ssd->parameter->subpage_page - (new_request->lsn + new_request->size));
                state = state & (~(0xffffffff << offset2));
            }
#ifdef  DEBUG
            printf("write lpn state is 0x%x, offset1 is 0x%x, offset2 is 0x%x\n",state,offset1, offset2 );
#endif
            ssd = insert2buffer(ssd, lpn, state, NULL, new_request);
            lpn++;
        }
    }
    complete_flag = 1;
    for (j = 0; j <= (last_lpn - first_lpn + 1) * ssd->parameter->subpage_page / 32; j++) {
        if (new_request->need_distr_flag[j] != 0) {
            complete_flag = 0;
        }
    }
#ifdef DEBUG
    printf("complete_flag is %d, (0 indicates that not be totally seviced by buffer directely)\n", complete_flag);
#endif
    /*************************************************************
    *如果请求已经被全部由buffer服务，该请求可以被直接响应，输出结果
    *这里假设dram的服务时间为1000ns
    **************************************************************/
    if ((complete_flag == 1) && (new_request->subs == NULL)) {
        new_request->begin_time = ssd->current_time;
        new_request->response_time = ssd->current_time + 1000;
    }

    return ssd;
}

/*****************************
*lpn向ppn的转换
******************************/
unsigned int lpn2ppn(struct ssd_info *ssd, unsigned int lsn) {
    int lpn, ppn;
    struct entry *p_map = ssd->dram->map->map_entry;
#ifdef DEBUG_CURR_TIME 
    printf("enter lpn2ppn,  current time:%lld\n",ssd->current_time);
#endif

    lpn = lsn / ssd->parameter->subpage_page;            //lpn
    ppn = (p_map[lpn]).pn;
    return ppn;
}

/**********************************************************************************
*读请求分配子请求函数，这里只处理读请求，写请求已经在buffer_management()函数中处理了
*根据请求队列和buffer命中的检查，将每个请求分解成子请求，将子请求队列挂在channel上，
*不同的channel有自己的子请求队列
**********************************************************************************/

struct ssd_info *distribute(struct ssd_info *ssd) {
    unsigned int start, end, first_lsn, last_lsn, lpn, flag = 0, flag_attached = 0, full_page;
    unsigned int j, k, sub_size;
    int i = 0;
    struct request *req;
    struct sub_request *sub;
    int *complt;

#ifdef DEBUG_CURR_TIME 
    printf("enter distribute,  current time:%lld\n",ssd->current_time);
    printf_ssd_request_queue(ssd);
#endif
    full_page = ~(0xffffffff << ssd->parameter->subpage_page);

    req = ssd->request_tail;
    if (req->response_time != 0) {
#ifdef DEBUG
        printf("req->response_time !=0\n");
#endif
        return ssd;
    }
    if (req->operation == WRITE) {
#ifdef DEBUG
        printf("ssd->req_tail's operation is write\n");
#endif
        return ssd;
    }

    if (req != NULL) {
        if (req->distri_flag == 0) {
            //如果还有一些读请求需要处理
            if (req->complete_lsn_count != ssd->request_tail->size) {
                first_lsn = req->lsn;
                last_lsn = first_lsn + req->size;
                complt = req->need_distr_flag;
                start = first_lsn - first_lsn % ssd->parameter->subpage_page;
                end = (last_lsn / ssd->parameter->subpage_page + 1) * ssd->parameter->subpage_page;
                i = (end - start) / 32;

                while (i >= 0) {
                    /*************************************************************************************
                    *一个32位的整型数据的每一位代表一个子页，32/ssd->parameter->subpage_page就表示有多少页，
                    *这里的每一页的状态都存放在了 req->need_distr_flag中，也就是complt中，通过比较complt的
                    *每一项与full_page，就可以知道，这一页是否处理完成。如果没处理完成则通过creat_sub_request
                    函数创建子请求。
                    *************************************************************************************/
                    for (j = 0; j < 32 / ssd->parameter->subpage_page; j++) {
                        k = (complt[((end - start) / 32 - i)] >> (ssd->parameter->subpage_page * j)) & full_page;
                        if (k != 0) {
                            lpn = start / ssd->parameter->subpage_page +
                                  ((end - start) / 32 - i) * 32 / ssd->parameter->subpage_page + j;
                            sub_size = transfer_size(ssd, k, lpn, req);
                            if (sub_size == 0) {
                                continue;
                            } else {
                                sub = creat_sub_request(ssd, lpn, sub_size, 0, req, req->operation);


                            }
                        }
                    }
                    i = i - 1;
                }
                //lxcprogram_de-prioritize.
                //1、 req's every sub_request's slack_time.
                //2、 insert the new sub_requests into channel_queue's respective chip
        //        calc_req_slack_time(req);
        //        reordering_req(req);


            } else {
                req->begin_time = ssd->current_time;
                req->response_time = ssd->current_time + 1000;
            }

        }
    }
    return ssd;
}


//lxcprogram_de-prioritize
int calc_req_slack_time(struct request * req)
{
    unsigned int channel_max, chip_max;
    struct sub_request *sub;
    struct sub_request *temp_sub;
    int64_t temp_time;
    sub = req->subs;
    temp_time = 0;

    while (sub != NULL){
        if(temp_time < sub->serviced_time){

            temp_time = sub->serviced_time;
            channel_max = sub->location->channel;
            chip_max = sub->location->chip ;
            temp_sub = sub;

            sub = sub->next_subs;
        }

    }

    temp_sub->last_sign = 1;

    sub = req->subs;
    while (sub != NULL){

        sub->slack_time = temp_time - sub->serviced_time;
#ifdef DEBUG 
         printf("req %u, sub slack_time is %d\n", req, sub->slack_time);
#endif

        sub = sub->next_subs;
#ifdef DEBUG 
        if(sub->slack_time == 0) {
            
            printf("channel_max %d, chip_max %d, sub->location->channel %d, sub->location->chip %d, they are should be the same\n", channel_max, chip_max, sub->location->channel, sub->location->chip);
        }
#endif

    }
    return 0;
}




int reordering_req(struct request * req )
{

//    unsigned int channel, chip, req_reorder_sign;
//    unsigned int temp_slack[1000];
//    struct sub_request *sub, temp_sub;
//    int64_t one_read_time;
//    req_reorder_sign = 1;
//    one_read_time = (ssd->parameter->time_characteristics.tR + 7 * ssd->parameter->time_characteristics.tWC + (sub->size * ssd->parameter->subpage_capacity) * ssd->parameter->time_characteristics.tWC);
//    //find req's last sub-request.then get the channel and chip.
//    sub = req->subs;
//    while (sub != NULL){
//
//        if(sub->last_sign == 1){
//            break;
//        }
//        sub = sub->next_subs;
//    }
//
//channel = sub->location->channel;
//chip = sub->location->chip;
////while (req_reorder_sign == 1){
//
//
//// insert last_sign at first, remember to change moved sub-requests's slack_time.
//sub = ssd->channel_head[channel]->subs_r_head;//here the subs_r_tail will not be NULL, since just adding the req request.
//while (sub->location->chip == chip ){
//
//if(sub->slack_time > one_read_time){
//sub = sub->next_node
//
//}


}
// recalculate req 's new slack_time
//find the last but one slack_time as 

//}//end of req_reorder_sign
// then due to change, so find all other (slack_time = 0)'s possible schedule. 








/**********************************************************************
*trace_output()函数是在每一条请求的所有子请求经过process()函数处理完后，
*打印输出相关的运行结果到outputfile文件中，这里的结果主要是运行的时间
**********************************************************************/
void trace_output(struct ssd_info *ssd) {
    int flag = 1;
    int64_t start_time, end_time;
    struct request *req, *pre_node;
    struct sub_request *sub, *tmp;

#ifdef DEBUG_CURR_TIME 
    printf("enter trace_output,  current time:%lld\n",ssd->current_time);
#endif

    req = ssd->request_queue;
    pre_node = req;
    start_time = 0;
    end_time = 0;

    if (req == NULL){
#ifdef DEBUG
        printf("ssd->request_queue is NULL\n");
#endif
        return;
    }
    while (req != NULL) {
#ifdef DEBUGTRACE_OUT
            printf("one entire request begin judging whether finishment here ,req address is %lld\n",req);
#endif


//    printf("request queue length is %d\n", ssd->request_queue_length);

        flag = 1;
        start_time = 0;
        end_time = 0;

/////***************************begin the request has response time*******************************///
        if (req->response_time != 0) {

//            printf("responding time has response_time here. ssd->request_queue is 0x%x\n", ssd->request_queue);
#ifdef DEBUGTRACE_OUT
            printf("only has entire req->response_time one time,dont care req's subrequest. \n");
#endif



#ifdef DEBUGSUSPEND
            printf("%16lld %10d %6d %2d %16lld %16lld %10lld\n", req->time, req->lsn, req->size,
                    req->operation, req->begin_time, req->response_time, req->response_time - req->time);

#endif
            fprintf(ssd->outputfile, "%16lld %10d %6d %2d %16lld %16lld %10lld\n", req->time, req->lsn, req->size,
                    req->operation, req->begin_time, req->response_time, req->response_time - req->time);
            fflush(ssd->outputfile);

            if (req->response_time - req->begin_time == 0) {
                printf("the response time is 0?? \n");
                getchar();
            }

            if (req->operation == READ) {
                ssd->read_request_count++;
                ssd->read_avg = ssd->read_avg + (req->response_time - req->time);
            } else {
                ssd->write_request_count++;
                ssd->write_avg = ssd->write_avg + (req->response_time - req->time);
            }
            // when the first request delete
            if(req == ssd->request_queue){


       free(req->need_distr_flag);
                req->need_distr_flag = NULL;
                free(req);
                ssd->request_queue_length--;

                ssd->request_queue = pre_node->next_node;
                pre_node =pre_node->next_node;
          
                req = pre_node;

                continue;

            }else {
                
                
                
                if (req == ssd->request_tail){

                    pre_node->next_node = req->next_node;
                    ssd->request_tail = pre_node;

                    free(req->need_distr_flag);
                    req->need_distr_flag = NULL;
                    free(req);
                    req = NULL;
                    ssd->request_queue_length--;
                    break; // here jump out all the requesting.

                }else {


                    pre_node->next_node = req->next_node;
                    free(req->need_distr_flag);
                    req->need_distr_flag = NULL;
                    free(req);
                    req = pre_node->next_node;
                    ssd->request_queue_length--;
                    continue;

                }
            }//end of no begin of the request.
        }//end of request 1000
        // end of request 1000 

/////***************************end of the request has response time*******************************///

/////***************************begin the request has no response time*******************************///
        // begin the request has sub_request judgement.
        else {

            flag = 1;// 1 means already finished.
            sub = req->subs;

//            printf("responding time has no value enter else,have to consider the sub request here. ssd->request_queue is 0x%x\n", ssd->request_queue);
#ifdef DEBUGSUSPEND
            printf("responding time has no value enter else,have to consider the sub request here.\n");
            printf("sub begin_time is %lld, complete_time is %lld,sub->next state %d, sub_next_time %lld,  this request's sub, only little one sub, operation[%d] 0 is write,lpn [%d],channel[%d], chip[%d], die[%d], block[%d], page[%d]\n", sub->begin_time, sub->complete_time, sub->next_state, sub->next_state_predict_time,  sub->operation, sub->lpn, sub->location->channel, sub->location->chip, sub->location->die, sub->location->block, sub->location->page);
#endif


            while (sub != NULL) {
                if (start_time == 0)
                    start_time = sub->begin_time;
                if (start_time > sub->begin_time)
                    start_time = sub->begin_time;
                if (end_time < sub->complete_time)
                    end_time = sub->complete_time;
                if ((sub->current_state == SR_COMPLETE) || ((sub->next_state == SR_COMPLETE) &&
                                                            (sub->next_state_predict_time <=
                                                             ssd->current_time)))    // if any sub-request is not completed, the request is not completed
                {
#ifdef DEBUGSUSPEND
            printf("sub begin_time is %lld, complete_time is %lld,sub->next state %d, sub_next_time %lld, this request's sub, only little one sub, operation[%d] 0 is write,lpn [%d],channel[%d], chip[%d], die[%d], block[%d], page[%d],here has been finished\n",sub->begin_time, sub->complete_time, sub->next_state, sub->next_state_predict_time, sub->operation, sub->lpn, sub->location->channel, sub->location->chip, sub->location->die, sub->location->block, sub->location->page);
#endif

                    sub = sub->next_subs;
                } else {
                    flag = 0;
                    break;
                }

            }
#ifdef DEBUGSUSPEND
printf("flag here is %d, *** 1 means finished.0 no\n",flag);
#endif
            if (flag == 1) {
                //fprintf(ssd->outputfile,"%10I64u %10u %6u %2u %16I64u %16I64u %10I64u\n",req->time,req->lsn, req->size, req->operation, start_time, end_time, end_time-req->time);

#ifdef DEBUGSUSPEND
                printf("now begin to clear the ssd->req. %16lld %10d %6d %2d %16lld %16lld %10lld\n", req->time, req->lsn, req->size,
                        req->operation, start_time, end_time, end_time - req->time);
#endif

                fprintf(ssd->outputfile, "%16lld %10d %6d %2d %16lld %16lld %10lld\n", req->time, req->lsn, req->size,
                        req->operation, start_time, end_time, end_time - req->time);
                fflush(ssd->outputfile);

                if (end_time - start_time == 0) {
                    printf("the response time is 0?? \n");
                    getchar();
                }

                if (req->operation == READ) {
                    ssd->read_request_count++;
                    ssd->read_avg = ssd->read_avg + (end_time - req->time);
                } else {
                    ssd->write_request_count++;
                    ssd->write_avg = ssd->write_avg + (end_time - req->time);
                }

      // when the first request delete
            if(req == ssd->request_queue){

                free(req->need_distr_flag);
                req->need_distr_flag = NULL;
                free(req);
                ssd->request_queue_length--;

                ssd->request_queue = pre_node->next_node;
                pre_node =pre_node->next_node;
          
                req = pre_node;


                continue;
            }else {
                
                
                
                if (req == ssd->request_tail){

                    pre_node->next_node = req->next_node;
                    ssd->request_tail = pre_node;

                    free(req->need_distr_flag);
                    req->need_distr_flag = NULL;
                    free(req);
                    req = NULL;
                    ssd->request_queue_length--;
                    break; // here jump out all the requesting.

                }else {


                    pre_node->next_node = req->next_node;
                    free(req->need_distr_flag);
                    req->need_distr_flag = NULL;
                    free(req);
                    req = pre_node->next_node;
                    ssd->request_queue_length--;
                    continue;

                }
            }//end of no begin of the request.

            } else {//除了上述两种情况，只剩下带有sub且没有完成的了。当没有完成，就敢于往后移动。只有删除的时候需要考虑是否是末尾。 // if it is not finished, then go next req.

//#ifdef DEBUGTRACE_OUT
//printf("have to wait for furthure finishment and judge another request . subrequest's next_node req \n");
//#endif
                pre_node = req;
                req = req->next_node;
            }
        }//end of request has sub_requests judegement.

/////***************************end of the request has no response time*******************************///

    }//end of request for loop
}


/*******************************************************************************
*statistic_output()函数主要是输出处理完一条请求后的相关处理信息。
*1，计算出每个plane的擦除次数即plane_erase和总的擦除次数即erase
*2，打印min_lsn，max_lsn，read_count，program_count等统计信息到文件outputfile中。
*3，打印相同的信息到文件statisticfile中
*******************************************************************************/
void statistic_output(struct ssd_info *ssd) {
    unsigned int lpn_count = 0, i, j, k, l, m, erase = 0, plane_erase = 0, temp_chip_channel = 0;
    double gc_energy = 0.0;
#ifdef DEBUG_CURR_TIME  
    printf("enter statistic_output,  current time:%lld\n",ssd->current_time);
#endif
temp_chip_channel = ssd->parameter->chip_num / ssd->parameter->channel_number;

for (i = 0; i < ssd->parameter->channel_number; i++) {
    for (l = 0; l < temp_chip_channel; l++) {
        for (j = 0; j < ssd->parameter->die_chip; j++) {
            for (k = 0; k < ssd->parameter->plane_die; k++) {
                plane_erase = 0;
                for (m = 0; m < ssd->parameter->block_plane; m++) {
                    if (ssd->channel_head[i].chip_head[l].die_head[j].plane_head[k].blk_head[m].erase_count > 0) {
                        erase = erase +
                            ssd->channel_head[i].chip_head[l].die_head[j].plane_head[k].blk_head[m].erase_count;
                        plane_erase += ssd->channel_head[i].chip_head[l].die_head[j].plane_head[k].blk_head[m].erase_count;
                    }
                }
                fprintf(ssd->outputfile, "the %d channel, %d chip, %d die, %d plane has : %13d erase operations\n", i, l, j, k, plane_erase);
                fprintf(ssd->statisticfile, "the %d channel, %d chip, %d die, %d plane has : %13d erase operations\n",
                        i, l, j, k, plane_erase);
            }
        }
    }
}
    fprintf(ssd->outputfile, "\n");
    fprintf(ssd->outputfile, "\n");
    fprintf(ssd->outputfile, "---------------------------statistic data---------------------------\n");
    fprintf(ssd->outputfile, "gc suspend times: %13d\n", ssd->gc_suspend_times);
    fprintf(ssd->outputfile, "write suspend times: %13d\n", ssd->write_suspend_times);
    fprintf(ssd->outputfile, "longest request queue length used : %13d\n", ssd->longest_queue_record);


    fprintf(ssd->outputfile, "min lsn: %13d\n", ssd->min_lsn);
    fprintf(ssd->outputfile, "max lsn: %13d\n", ssd->max_lsn);
    fprintf(ssd->outputfile, "read count: %13d\n", ssd->read_count);
    fprintf(ssd->outputfile, "program count: %13d", ssd->program_count);
    fprintf(ssd->outputfile, "                        include the flash write count leaded by read requests\n");
    fprintf(ssd->outputfile, "the read operation leaded by un-covered update count: %13d\n", ssd->update_read_count);
    fprintf(ssd->outputfile, "erase count: %13d\n", ssd->erase_count);
    fprintf(ssd->outputfile, "direct erase count: %13d\n", ssd->direct_erase_count);
    fprintf(ssd->outputfile, "copy back count: %13d\n", ssd->copy_back_count);
    fprintf(ssd->outputfile, "multi-plane program count: %13d\n", ssd->m_plane_prog_count);
    fprintf(ssd->outputfile, "multi-plane read count: %13d\n", ssd->m_plane_read_count);
    fprintf(ssd->outputfile, "interleave write count: %13d\n", ssd->interleave_count);
    fprintf(ssd->outputfile, "interleave read count: %13d\n", ssd->interleave_read_count);
    fprintf(ssd->outputfile, "interleave two plane and one program count: %13d\n", ssd->inter_mplane_prog_count);
    fprintf(ssd->outputfile, "interleave two plane count: %13d\n", ssd->inter_mplane_count);
    fprintf(ssd->outputfile, "gc copy back count: %13d\n", ssd->gc_copy_back);
    fprintf(ssd->outputfile, "write flash count: %13d\n", ssd->write_flash_count);
    fprintf(ssd->outputfile, "interleave erase count: %13d\n", ssd->interleave_erase_count);
    fprintf(ssd->outputfile, "multiple plane erase count: %13d\n", ssd->mplane_erase_conut);
    fprintf(ssd->outputfile, "interleave multiple plane erase count: %13d\n", ssd->interleave_mplane_erase_count);
    fprintf(ssd->outputfile, "read request count: %13d\n", ssd->read_request_count);
    fprintf(ssd->outputfile, "write request count: %13d\n", ssd->write_request_count);
    fprintf(ssd->outputfile, "read request average size: %13f\n", ssd->ave_read_size);
    fprintf(ssd->outputfile, "write request average size: %13f\n", ssd->ave_write_size);
    fprintf(ssd->outputfile, "read request average response time: %lld\n", ssd->read_avg / ssd->read_request_count);
    fprintf(ssd->outputfile, "write request average response time: %lld\n", ssd->write_avg / ssd->write_request_count);
    fprintf(ssd->outputfile, "buffer read hits: %13d\n", ssd->dram->buffer->read_hit);
    fprintf(ssd->outputfile, "buffer read miss: %13d\n", ssd->dram->buffer->read_miss_hit);
    fprintf(ssd->outputfile, "buffer write hits: %13d\n", ssd->dram->buffer->write_hit);
    fprintf(ssd->outputfile, "buffer write miss: %13d\n", ssd->dram->buffer->write_miss_hit);
    fprintf(ssd->outputfile, "erase: %13d\n", erase);

    fprintf(ssd->outputfile, "original total pages is %13d\n", ssd->original_total_pages_number);
    fprintf(ssd->outputfile, "only pre_processed pages number is %13d\n", ssd->after_pre_process_pages_number);
    fprintf(ssd->outputfile, "after prep rocessed read part and aged pages , then left the available number is %13d\n", ssd->after_pre_processe_and_aged_pages_number);

    for (i = 0; i < ssd->parameter->channel_number; i++){
        for (l = 0; l < temp_chip_channel; l++) {
            for (j = 0; j < ssd->parameter->die_chip; j++){
                for (k = 0; k < ssd->parameter->plane_die; k++) {
                    fprintf(ssd->outputfile, "channel %d, chip:%d, die:%d, plane:%d have free page: %d\n", i, l, j, k,
                            ssd->channel_head[i].chip_head[l].die_head[j].plane_head[k].free_page);
                    ssd->last_total_available_pages_number +=  ssd->channel_head[i].chip_head[l].die_head[j].plane_head[k].free_page;
                }
            }
        }
    }

 //   fprintf(ssd->outputfile, "only write used pages number is %13d\n",  ssd->after_pre_process_pages_number - ssd->last_total_available_pages_number);
    fprintf(ssd->outputfile, "last_total_available_pages number is %13d\n", ssd->last_total_available_pages_number);






    fflush(ssd->outputfile);

    fclose(ssd->outputfile);


    fprintf(ssd->statisticfile, "\n");
    fprintf(ssd->statisticfile, "\n");
    fprintf(ssd->statisticfile, "---------------------------statistic data---------------------------\n");
    fprintf(ssd->statisticfile, "min lsn: %13d\n", ssd->min_lsn);
    fprintf(ssd->statisticfile, "max lsn: %13d\n", ssd->max_lsn);
    fprintf(ssd->statisticfile, "read count: %13d\n", ssd->read_count);
    fprintf(ssd->statisticfile, "program count: %13d", ssd->program_count);
    fprintf(ssd->statisticfile, "                        include the flash write count leaded by read requests\n");
    fprintf(ssd->statisticfile, "the read operation leaded by un-covered update count: %13d\n", ssd->update_read_count);
    fprintf(ssd->statisticfile, "erase count: %13d\n", ssd->erase_count);
    fprintf(ssd->statisticfile, "direct erase count: %13d\n", ssd->direct_erase_count);
    fprintf(ssd->statisticfile, "copy back count: %13d\n", ssd->copy_back_count);
    fprintf(ssd->statisticfile, "multi-plane program count: %13d\n", ssd->m_plane_prog_count);
    fprintf(ssd->statisticfile, "multi-plane read count: %13d\n", ssd->m_plane_read_count);
    fprintf(ssd->statisticfile, "interleave count: %13d\n", ssd->interleave_count);
    fprintf(ssd->statisticfile, "interleave read count: %13d\n", ssd->interleave_read_count);
    fprintf(ssd->statisticfile, "interleave two plane and one program count: %13d\n", ssd->inter_mplane_prog_count);
    fprintf(ssd->statisticfile, "interleave two plane count: %13d\n", ssd->inter_mplane_count);
    fprintf(ssd->statisticfile, "gc copy back count: %13d\n", ssd->gc_copy_back);
    fprintf(ssd->statisticfile, "write flash count: %13d\n", ssd->write_flash_count);
    fprintf(ssd->statisticfile, "waste page count: %13d\n", ssd->waste_page_count);
    fprintf(ssd->statisticfile, "interleave erase count: %13d\n", ssd->interleave_erase_count);
    fprintf(ssd->statisticfile, "multiple plane erase count: %13d\n", ssd->mplane_erase_conut);
    fprintf(ssd->statisticfile, "interleave multiple plane erase count: %13d\n", ssd->interleave_mplane_erase_count);
    fprintf(ssd->statisticfile, "read request count: %13d\n", ssd->read_request_count);
    fprintf(ssd->statisticfile, "write request count: %13d\n", ssd->write_request_count);
    fprintf(ssd->statisticfile, "read request average size: %13f\n", ssd->ave_read_size);
    fprintf(ssd->statisticfile, "write request average size: %13f\n", ssd->ave_write_size);
    fprintf(ssd->statisticfile, "read request average response time: %lld\n", ssd->read_avg / ssd->read_request_count);
    fprintf(ssd->statisticfile, "write request average response time: %lld\n",
            ssd->write_avg / ssd->write_request_count);
    fprintf(ssd->statisticfile, "buffer read hits: %13d\n", ssd->dram->buffer->read_hit);
    fprintf(ssd->statisticfile, "buffer read miss: %13d\n", ssd->dram->buffer->read_miss_hit);
    fprintf(ssd->statisticfile, "buffer write hits: %13d\n", ssd->dram->buffer->write_hit);
    fprintf(ssd->statisticfile, "buffer write miss: %13d\n", ssd->dram->buffer->write_miss_hit);
    fprintf(ssd->statisticfile, "erase: %13d\n", erase);


    fflush(ssd->statisticfile);

    fclose(ssd->statisticfile);
}


/***********************************************************************************
*根据每一页的状态计算出每一需要处理的子页的数目，也就是一个子请求需要处理的子页的页数
************************************************************************************/
unsigned int size(unsigned int stored) {
    unsigned int i, total = 0, mask = 0x80000000;

#ifdef DEBUG
   printf("enter size\n");
#endif
    for (i = 1; i <= 32; i++) {
        if (stored & mask) total++;
        stored <<= 1;
    }
#ifdef DEBUG
    printf("leave size total lsn number in request : %d\n",total);
#endif
    return total;
}


/*********************************************************
*transfer_size()函数的作用就是计算出子请求的需要处理的size
*函数中单独处理了first_lpn，last_lpn这两个特别情况，因为这
*两种情况下很有可能不是处理一整页而是处理一页的一部分，因
*为lsn有可能不是一页的第一个子页。
*********************************************************/
unsigned int transfer_size(struct ssd_info *ssd, int need_distribute, unsigned int lpn, struct request *req) {
    unsigned int first_lpn, last_lpn, state, trans_size;
    unsigned int mask = 0, offset1 = 0, offset2 = 0;

    first_lpn = req->lsn / ssd->parameter->subpage_page;
    last_lpn = (req->lsn + req->size - 1) / ssd->parameter->subpage_page;

    mask = ~(0xffffffff << (ssd->parameter->subpage_page));
    state = mask;
    if (lpn == first_lpn) {
        offset1 = ssd->parameter->subpage_page - ((lpn + 1) * ssd->parameter->subpage_page - req->lsn);
        state = state & (0xffffffff << offset1);
    }
    if (lpn == last_lpn) {
        offset2 = ssd->parameter->subpage_page - ((lpn + 1) * ssd->parameter->subpage_page - (req->lsn + req->size));
        state = state & (~(0xffffffff << offset2));
    }

    trans_size = size(state & need_distribute);

    return trans_size;
}


/**********************************************************************************************************  
*int64_t find_nearest_event(struct ssd_info *ssd)       
*寻找所有子请求的最早到达的下个状态时间,首先看请求的下一个状态时间，如果请求的下个状态时间小于等于当前时间，
*说明请求被阻塞，需要查看channel或者对应die的下一状态时间。Int64是有符号 64 位整数数据类型，值类型表示值介于
*-2^63 ( -9,223,372,036,854,775,808)到2^63-1(+9,223,372,036,854,775,807 )之间的整数。存储空间占 8 字节。
*channel,die是事件向前推进的关键因素，三种情况可以使事件继续向前推进，channel，die分别回到idle状态，die中的
*读数据准备好了
***********************************************************************************************************/
int64_t find_nearest_event(struct ssd_info *ssd) {
    unsigned int i, j;
    int64_t time = MAX_INT64;
    int64_t time1 = MAX_INT64;
    int64_t time2 = MAX_INT64;
#ifdef DEBUG_CURR_TIME  //lxcv1
    printf("enter find_nearest_event, ssd->current_time is %lld\n",ssd->current_time);

    for (i = 0; i < ssd->parameter->channel_number; i++) {

        printf("channel[%d].next_state is 0x%x,ssd->channel_head[%d].next_state_predict_time is %lld, channel[%d].current_state is 0x%x,ssd->channel_head[%d].current_time is %lld\n",i,ssd->channel_head[i].next_state,i,ssd->channel_head[i].next_state_predict_time  ,i,ssd->channel_head[i].current_state,i,ssd->channel_head[i].current_time  );

        for (j = 0; j < ssd->parameter->chip_channel[i]; j++) {

            printf("ssd->channel_head[%d].chip_head[%d].next_state_predict_time is %lld,  ssd->channel_head[%d].chip_head[%d].next_state is 0x%x  ssd->channel_head[%d].chip_head[%d].current_time is %lld,  ssd->channel_head[%d].chip_head[%d].current_state is 0x%x \n", i,j,ssd->channel_head[i].chip_head[j].next_state_predict_time ,i,j,ssd->channel_head[i].chip_head[j].next_state , i,j,ssd->channel_head[i].chip_head[j].current_time ,i,j,ssd->channel_head[i].chip_head[j].current_state  );

        }

    }

#endif


    for (i = 0; i < ssd->parameter->channel_number; i++) {
        if (ssd->channel_head[i].next_state == CHANNEL_IDLE)
            if (time1 > ssd->channel_head[i].next_state_predict_time)
                if (ssd->channel_head[i].next_state_predict_time > ssd->current_time)
                    time1 = ssd->channel_head[i].next_state_predict_time;
        for (j = 0; j < ssd->parameter->chip_channel[i]; j++) {

            //lxc_programgc orignal wrong
            //    if ((ssd->channel_head[i].chip_head[j].next_state == CHIP_IDLE) ||
            //          (ssd->channel_head[i].chip_head[j].next_state == CHIP_DATA_TRANSFER))
            if (time2 > ssd->channel_head[i].chip_head[j].next_state_predict_time)
                if (ssd->channel_head[i].chip_head[j].next_state_predict_time > ssd->current_time)
                        time2 = ssd->channel_head[i].chip_head[j].next_state_predict_time;
        }
    }

    /*****************************************************************************************************
     *time为所有 A.下一状态为CHANNEL_IDLE且下一状态预计时间大于ssd当前时间的CHANNEL的下一状态预计时间
     *           B.下一状态为CHIP_IDLE且下一状态预计时间大于ssd当前时间的DIE的下一状态预计时间
     *		     C.下一状态为CHIP_DATA_TRANSFER且下一状态预计时间大于ssd当前时间的DIE的下一状态预计时间
     *CHIP_DATA_TRANSFER读准备好状态，数据已从介质传到了register，下一状态是从register传往buffer中的最小值
     *注意可能都没有满足要求的time，这时time返回0x7fffffffffffffff 。
     *****************************************************************************************************/
#ifdef DEBUG //lxcv1
    printf("result time1 is %lld, time2 is %lld\n", time1, time2);

#endif

    time = (time1 > time2) ? time2 : time1;
    return time;
}

/***********************************************
*free_all_node()函数的作用就是释放所有申请的节点
************************************************/
void free_all_node(struct ssd_info *ssd) {
    unsigned int i, j, k, l, n;
    struct buffer_group *pt = NULL;
    struct direct_erase *erase_node = NULL;
    for (i = 0; i < ssd->parameter->channel_number; i++) {
        for (j = 0; j < ssd->parameter->chip_channel[0]; j++) {
            for (k = 0; k < ssd->parameter->die_chip; k++) {
                for (l = 0; l < ssd->parameter->plane_die; l++) {
                    for (n = 0; n < ssd->parameter->block_plane; n++) {
                        free(ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[n].page_head);
                        ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[n].page_head = NULL;
                    }
                    free(ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head);
                    ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head = NULL;
                    while (ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].erase_node != NULL) {
                        erase_node = ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].erase_node;
                        ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].erase_node = erase_node->next_node;
                        free(erase_node);
                        erase_node = NULL;
                    }
                }

                free(ssd->channel_head[i].chip_head[j].die_head[k].plane_head);
                ssd->channel_head[i].chip_head[j].die_head[k].plane_head = NULL;
            }
            free(ssd->channel_head[i].chip_head[j].die_head);
            ssd->channel_head[i].chip_head[j].die_head = NULL;
        }
        free(ssd->channel_head[i].chip_head);
        ssd->channel_head[i].chip_head = NULL;
    }
    free(ssd->channel_head);
    ssd->channel_head = NULL;

    avlTreeDestroy(ssd->dram->buffer);
    ssd->dram->buffer = NULL;

    free(ssd->dram->map->map_entry);
    ssd->dram->map->map_entry = NULL;
    free(ssd->dram->map);
    ssd->dram->map = NULL;
    free(ssd->dram);
    ssd->dram = NULL;
    free(ssd->parameter);
    ssd->parameter = NULL;

    free(ssd);
    ssd = NULL;
}


/*****************************************************************************
*make_aged()函数的作用就死模拟真实的用过一段时间的ssd，
*那么这个ssd的相应的参数就要改变，所以这个函数实质上就是对ssd中各个参数的赋值。
******************************************************************************/
struct ssd_info *make_aged(struct ssd_info *ssd) {
    unsigned int i, j, k, l, m, n, ppn;
    int threshould, flag = 0;

    if (ssd->parameter->aged == 1) {
        //threshold表示一个plane中有多少页需要提前置为失效
        threshould = (int) (ssd->parameter->block_plane * ssd->parameter->page_block * ssd->parameter->aged_ratio);
        for (i = 0; i < ssd->parameter->channel_number; i++)
            for (j = 0; j < ssd->parameter->chip_channel[i]; j++)
                for (k = 0; k < ssd->parameter->die_chip; k++)
                    for (l = 0; l < ssd->parameter->plane_die; l++) {
                        flag = 0;
                        for (m = 0; m < ssd->parameter->block_plane; m++) {
                            if (flag >= threshould) {
                                break;
                            }
                            for (n = 0; n < (ssd->parameter->page_block * ssd->parameter->aged_ratio + 1); n++) {
                                ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].page_head[n].valid_state = 0;        //表示某一页失效，同时标记valid和free状态都为0
                                ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].page_head[n].free_state = 0;         //表示某一页失效，同时标记valid和free状态都为0
                                ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].page_head[n].lpn = 0;  //把valid_state free_state lpn都置为0表示页失效，检测的时候三项都检测，单独lpn=0可以是有效页
                                ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].free_page_num--;
                                ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].invalid_page_num++;
                                ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].blk_head[m].last_write_page++;
                                ssd->channel_head[i].chip_head[j].die_head[k].plane_head[l].free_page--;
                                flag++;

                                ppn = find_ppn(ssd, i, j, k, l, m, n);
#ifdef DEBUGONLYGC
                   //             printf("channel %d, chip %d, die %d, plane %d, block %d, page %d\n", i, j, k, l, m, n);
#endif

                            }
                        }
                    }
    } else {
        return ssd;
    }

    return ssd;
}


/*********************************************************************************************
*no_buffer_distribute()函数是处理当ssd没有dram的时候，
*这是读写请求就不必再需要在buffer里面寻找，直接利用creat_sub_request()函数创建子请求，再处理。
*********************************************************************************************/
struct ssd_info *no_buffer_distribute(struct ssd_info *ssd) {
    unsigned int lsn, lpn, last_lpn, first_lpn, complete_flag = 0, state;
    unsigned int flag = 0, flag1 = 1, active_region_flag = 0;           //to indicate the lsn is hitted or not
    struct request *req = NULL;
    struct sub_request *sub = NULL, *sub_r = NULL, *update = NULL;
    struct local *loc = NULL;
    struct channel_info *p_ch = NULL;
    unsigned int mask = 0;
    unsigned int offset1 = 0, offset2 = 0;
    unsigned int sub_size = 0;
    unsigned int sub_state = 0;
    ssd->dram->current_time = ssd->current_time;

#ifdef DEBUG_CURR_TIME  //lxcv1
printf("current_time is %lld in no_buffer_distribute function\n", ssd->current_time);
#endif
    req = ssd->request_tail;
    lsn = req->lsn;
    lpn = req->lsn / ssd->parameter->subpage_page;
    last_lpn = (req->lsn + req->size - 1) / ssd->parameter->subpage_page;
    first_lpn = req->lsn / ssd->parameter->subpage_page;

    if (req->operation == READ) {
        while (lpn <= last_lpn) {
            sub_state = (ssd->dram->map->map_entry[lpn].state & 0x7fffffff);
            sub_size = size(sub_state);
            sub = creat_sub_request(ssd, lpn, sub_size, sub_state, req, req->operation);
            lpn++;
        }
    } else if (req->operation == WRITE) {
        while (lpn <= last_lpn) {
            mask = ~(0xffffffff << (ssd->parameter->subpage_page));
            state = mask;
            if (lpn == first_lpn) {
                offset1 = ssd->parameter->subpage_page - ((lpn + 1) * ssd->parameter->subpage_page - req->lsn);
                state = state & (0xffffffff << offset1);
            }
            if (lpn == last_lpn) {
                offset2 = ssd->parameter->subpage_page -
                          ((lpn + 1) * ssd->parameter->subpage_page - (req->lsn + req->size));
                state = state & (~(0xffffffff << offset2));
            }
            sub_size = size(state);

            sub = creat_sub_request(ssd, lpn, sub_size, state, req, req->operation);
            lpn++;
        }
    }

    return ssd;
}


