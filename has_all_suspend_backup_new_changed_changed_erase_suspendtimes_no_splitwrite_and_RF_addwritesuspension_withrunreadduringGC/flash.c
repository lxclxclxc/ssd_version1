/*****************************************************************************************************************************
This project was supported by the National Basic Research 973 Program of China under Grant No.2011CB302301
Huazhong University of Science and Technology (HUST)   Wuhan National Laboratory for Optoelectronics

FileName： flash.c
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

#include "flash.h"
#include "ssd.h"
#define NUMBER_OF_WRITE_ITERATION 3
/**********************
*这个函数只作用于写请求
***********************/
Status allocate_location(struct ssd_info *ssd, struct sub_request *sub_req) {
    struct sub_request *update = NULL;
    unsigned int channel_num = 0, chip_num = 0, die_num = 0, plane_num = 0;
    struct local *location = NULL;

    channel_num = ssd->parameter->channel_number;
    chip_num = ssd->parameter->chip_channel[0];
    die_num = ssd->parameter->die_chip;
    plane_num = ssd->parameter->plane_die;
#ifdef DEBUG_CURR_TIME  //lxcv1
printf("current_time is %lld in allocate_location function, and ssdsim parameter here is  channelnum %d, chipnum %d, dienum %d, plane num%d\n", ssd->current_time, channel_num, chip_num, die_num, plane_num);
#endif


    if (ssd->parameter->allocation_scheme == 0)                                          /*动态分配的情况*/
    {
        /******************************************************************
		* 在动态分配中，因为页的更新操作使用不了copyback操作，
		*需要产生一个读请求，并且只有这个读请求完成后才能进行这个页的写操作
		*******************************************************************/
        if (ssd->dram->map->map_entry[sub_req->lpn].state != 0) {
            if ((sub_req->state & ssd->dram->map->map_entry[sub_req->lpn].state) !=
                ssd->dram->map->map_entry[sub_req->lpn].state) {
#ifdef DEBUG  //lxcv1
                printf("at least , write request should be flushed into flashcurrent_time is %lld in allocate_location function\n", ssd->current_time);
#endif

                ssd->read_count++;
                ssd->update_read_count++;

                update = (struct sub_request *) malloc(sizeof(struct sub_request));
                alloc_assert(update, "update");
                memset(update, 0, sizeof(struct sub_request));

                if (update == NULL) {
                    return ERROR;
                }
                update->location = NULL;
                update->next_node = NULL;
                update->next_subs = NULL;
                update->update = NULL;
                location = find_location(ssd, ssd->dram->map->map_entry[sub_req->lpn].pn);
                update->location = location;
                update->begin_time = ssd->current_time;
                update->current_state = SR_WAIT;
                update->current_time = MAX_INT64;
#ifdef DEBUG_CURR_TIME  //lxcv1
                printf("current_time is %lld in  allocate_location function map_entry[xx].state!=0 and dynamic allocation\n", ssd->current_time);
#endif

                update->next_state = SR_R_C_A_TRANSFER;
                update->next_state_predict_time = MAX_INT64;
                update->lpn = sub_req->lpn;
                update->state = ((ssd->dram->map->map_entry[sub_req->lpn].state ^ sub_req->state) & 0x7fffffff);
                update->size = size(update->state);
                update->ppn = ssd->dram->map->map_entry[sub_req->lpn].pn;
                update->operation = READ;
                //printf("added update %lld, location is %lld\n",update,  update->location);
               // printf("now channel %d, chip %d length is %d,  head address %lld, tail address %lld \n", location->channel, location->chip, ssd->channel_head[location->channel].chip_head[location->chip].chip_read_queue_length, ssd->channel_head[location->channel].subs_r_head,ssd->channel_head[location->channel].subs_r_tail);
                if ((ssd->channel_head[location->channel].subs_r_head != NULL )&& (ssd->channel_head[location->channel].subs_r_tail !=
                    NULL) )           /*产生新的读请求，并且挂到channel的subs_r_tail队列尾*/
                {
//                    printf("here length is not 0  \n");
                    ssd->channel_head[location->channel].subs_r_tail->next_node = update;
                    ssd->channel_head[location->channel].subs_r_tail = update;
                } else {
if(ssd->channel_head[location->channel].chip_head[location->chip].chip_read_queue_length != 0){
                    printf("lxclxclxc,wrong \n");
}
                    ssd->channel_head[location->channel].subs_r_tail = update;
                    ssd->channel_head[location->channel].subs_r_head = update;
                }
            }
        }
        /***************************************
		*一下是动态分配的几种情况
		*0：全动态分配
		*1：表示channel定package，die，plane动态
		****************************************/
        switch (ssd->parameter->dynamic_allocation) {
            case 0: {
                sub_req->location->channel = -1;
                sub_req->location->chip = -1;
                sub_req->location->die = -1;
                sub_req->location->plane = -1;
                sub_req->location->block = -1;
                sub_req->location->page = -1;

                if (ssd->subs_w_tail != NULL) {
                    ssd->subs_w_tail->next_node = sub_req;
                    ssd->subs_w_tail = sub_req;
                } else {
                    ssd->subs_w_tail = sub_req;
                    ssd->subs_w_head = sub_req;
                }

                if (update != NULL) {
                    sub_req->update = update;
                }

                break;
            }
            case 1: {

                sub_req->location->channel = sub_req->lpn % ssd->parameter->channel_number;
                sub_req->location->chip = -1;
                sub_req->location->die = -1;
                sub_req->location->plane = -1;
                sub_req->location->block = -1;
                sub_req->location->page = -1;

                if (update != NULL) {
                    sub_req->update = update;
                }

                break;
            }
            case 2: {
                break;
            }
            case 3: {
                break;
            }
        }

    } else {   /***************************************************************************
		*是静态分配方式，所以可以将这个子请求的最终channel，chip，die，plane全部得出
		*总共有0,1,2,3,4,5,这六种静态分配方式。
		****************************************************************************/
#ifdef DEBUGSUSPEND
        printf("here is static_allocation in allocate location....,and strategy is %d, \n", ssd->parameter->static_allocation);
#endif

        switch (ssd->parameter->static_allocation) {
            case 0:         //no striping static allocation
            {
                sub_req->location->channel = (sub_req->lpn / (plane_num * die_num * chip_num)) % channel_num;
                sub_req->location->chip = sub_req->lpn % chip_num;
                sub_req->location->die = (sub_req->lpn / chip_num) % die_num;
                sub_req->location->plane = (sub_req->lpn / (die_num * chip_num)) % plane_num;
                break;
            }
            case 1: {
                sub_req->location->channel = sub_req->lpn % channel_num;
                sub_req->location->chip = (sub_req->lpn / channel_num) % chip_num;
                sub_req->location->die = (sub_req->lpn / (chip_num * channel_num)) % die_num;
                sub_req->location->plane = (sub_req->lpn / (die_num * chip_num * channel_num)) % plane_num;

                break;
            }
            case 2: {
                sub_req->location->channel = sub_req->lpn % channel_num;
                sub_req->location->chip = (sub_req->lpn / (plane_num * channel_num)) % chip_num;
                sub_req->location->die = (sub_req->lpn / (plane_num * chip_num * channel_num)) % die_num;
                sub_req->location->plane = (sub_req->lpn / channel_num) % plane_num;
                break;
            }
            case 3: {
                sub_req->location->channel = sub_req->lpn % channel_num;
                sub_req->location->chip = (sub_req->lpn / (die_num * channel_num)) % chip_num;
                sub_req->location->die = (sub_req->lpn / channel_num) % die_num;
                sub_req->location->plane = (sub_req->lpn / (die_num * chip_num * channel_num)) % plane_num;
                break;
            }
            case 4: {
                sub_req->location->channel = sub_req->lpn % channel_num;
                sub_req->location->chip = (sub_req->lpn / (plane_num * die_num * channel_num)) % chip_num;
                sub_req->location->die = (sub_req->lpn / (plane_num * channel_num)) % die_num;
                sub_req->location->plane = (sub_req->lpn / channel_num) % plane_num;

                break;
            }
            case 5: {
                sub_req->location->channel = sub_req->lpn % channel_num;
                sub_req->location->chip = (sub_req->lpn / (plane_num * die_num * channel_num)) % chip_num;
                sub_req->location->die = (sub_req->lpn / channel_num) % die_num;
                sub_req->location->plane = (sub_req->lpn / (die_num * channel_num)) % plane_num;

                break;
            }
            default :
                return ERROR;

        }

#ifdef DEBUGSUSPEND
        printf("after static allocation.....now, write request created in channel %d, chip %d, die %d, plane %d\n",sub_req->location->channel,sub_req->location->chip,sub_req->location->die, sub_req->location->plane);
#endif


        if (ssd->dram->map->map_entry[sub_req->lpn].state !=
            0) {                                                                              /*这个写回的子请求的逻辑页不可以覆盖之前被写回的数据 需要产生读请求*/
            if ((sub_req->state & ssd->dram->map->map_entry[sub_req->lpn].state) !=
                ssd->dram->map->map_entry[sub_req->lpn].state) {
#ifdef DEBUGSUSPEND
                printf("should create update for write now.\n");
#endif

                ssd->read_count++;
                ssd->update_read_count++;
                update = (struct sub_request *) malloc(sizeof(struct sub_request));
                alloc_assert(update, "update");
                memset(update, 0, sizeof(struct sub_request));

                if (update == 0x0) {
#ifdef DEBUGSUSPEND
                    printf("lxclxc, wrong in creating update read for write .\n");
#endif
                    return ERROR;
                }
                update->location = NULL;
                update->next_node = NULL;
                update->next_subs = NULL;
                update->update = NULL;
                location = find_location(ssd, ssd->dram->map->map_entry[sub_req->lpn].pn);
                if(location == 0x0){

                    printf("lxc, write's update lpn %d, size %d, request %lld, operation %d has wrong now\n", sub_req->lpn,sub_req->size, sub_req,sub_req->operation);
                    printf("lxclxc, wrong in creating update read for write .\n");
                }
                update->location = location;
                update->begin_time = ssd->current_time;
#ifdef DEBUG_CURR_TIME  //lxcv1
printf("current_time is %lld in  allocate_location function map_entry[xx].state!=0 and static allocation\n", ssd->current_time);
#endif


                update->current_state = SR_WAIT;
                update->current_time = MAX_INT64;
                update->next_state = SR_R_C_A_TRANSFER;
                update->next_state_predict_time = MAX_INT64;
                update->lpn = sub_req->lpn;
                update->state = ((ssd->dram->map->map_entry[sub_req->lpn].state ^ sub_req->state) & 0x7fffffff);
                update->size = size(update->state);
                update->ppn = ssd->dram->map->map_entry[sub_req->lpn].pn;
                update->operation = READ;
#ifdef DEBUGSUSPEND  //lxcv1
printf("here but special for write!! add  read, first service read,now. read location is channel%d, chip %d, die %d,plane %d, ppn %d\n",update->location->channel, update->location->chip,update->location->die,update->location->plane ,update->ppn );
#endif

//lxcprogram_gc original wrong
                if( (ssd->channel_head[update->location->channel].subs_r_head != NULL)&&(ssd->channel_head[update->location->channel].subs_r_tail != NULL) ){
 #ifdef DEBUGSUSPEND  //lxcv1
printf("here means sus_r_tail is not NULL , here is channel [%d], chip [%d],has the tail address %lld,update address is %lld\n",update->location->channel, update->location->chip,ssd->channel_head[update->location->channel].subs_r_tail, update );
#endif
                //  printf("read length not 0"); 
                    ssd->channel_head[update->location->channel].subs_r_tail->next_node = update;
                    ssd->channel_head[update->location->channel].subs_r_tail = update;
                } else if ((ssd->channel_head[update->location->channel].subs_r_head == NULL) && (ssd->channel_head[update->location->channel].subs_r_tail == NULL)){
                    if(ssd->channel_head[location->channel].chip_head[location->chip].chip_read_queue_length != 0){
                printf("lxclxclxc, wrong\n");
}

                    ssd->channel_head[update->location->channel].subs_r_tail = update;
                    ssd->channel_head[update->location->channel].subs_r_head = update;
                }else {
                    printf("lxclxclxc2, added read wrong\n");
                }
#ifdef DEBUGSUSPEND  //lxcv1
printf("added read here is channel [%d], chip [%d],has the tail address %lld,update address is %lld\n",update->location->channel, update->location->chip,ssd->channel_head[update->location->channel].subs_r_tail, update );
#endif
            }

            if (update != NULL) {
                //lxcprogram_gc 
                sub_req->update = update;
                sub_req->state = (sub_req->state | update->state);
                sub_req->size = size(sub_req->state);

//lxcprogram_gc
                ssd->channel_head[update->location->channel].chip_head[update->location->chip].chip_read_queue_length ++;
                printf("added update %lld, location is %lld\n",update,  update->location);
                if(update->location == NULL){
                    printf("lxclxclxc_updatelocation wrong\n");
                }

                if(update->lpn > 16777216){
                    printf("lxclxclxc_lpnwrong in now\n");
                }

                printf("now channel %d, chip %d length is %d,  head address %lld, tail address %lld , sub %lld\n", location->channel, location->chip, ssd->channel_head[location->channel].chip_head[location->chip].chip_read_queue_length, ssd->channel_head[location->channel].subs_r_head,ssd->channel_head[location->channel].subs_r_tail, update);

if(update->location == NULL){
//#ifdef DEBUGSUSPEND  //lxcv1
printf("new creat read update  location is channel %d, chip %d, die %d, plane %d\n",location->channel,location->chip,location->die,location->plane);
//#endif
}
              //  printf_every_chip_read_subrequest(ssd);

            }

        }
    }

if(sub_req->location == NULL){
//#ifdef DEBUGSUSPEND  //lxcv1
printf("new creat write location is channel %d, chip %d, die %d, plane %d\n",sub_req->location->channel,sub_req->location->chip,sub_req->location->die,sub_req->location->plane);
//#endif
}
//lxcprogram_gc
ssd->channel_head[sub_req->location->channel].chip_head[sub_req->location->chip].chip_write_queue_length ++;
#ifdef DEBUGSUSPEND  //lxcv1
printf("channel[%d], chip[%d], write length add one is [%d]\n",sub_req->location->channel, sub_req->location->chip, ssd->channel_head[sub_req->location->channel].chip_head[sub_req->location->chip].chip_write_queue_length);
#endif

    if ((ssd->parameter->allocation_scheme != 0) || (ssd->parameter->dynamic_allocation != 0)) {
        if( (ssd->channel_head[sub_req->location->channel].subs_w_tail != NULL) && (ssd->channel_head[sub_req->location->channel].subs_w_head != NULL) ) {
            ssd->channel_head[sub_req->location->channel].subs_w_tail->next_node = sub_req;
            ssd->channel_head[sub_req->location->channel].subs_w_tail = sub_req;
        } else if( (ssd->channel_head[sub_req->location->channel].subs_w_tail == NULL) && (ssd->channel_head[sub_req->location->channel].subs_w_head == NULL)){
            ssd->channel_head[sub_req->location->channel].subs_w_tail = sub_req;
            ssd->channel_head[sub_req->location->channel].subs_w_head = sub_req;
        }else{
            printf("writewrong\n");
        }
    }

//#ifdef DEBUGSUSPEND  //lxcv1
//printf("here is make sure adding write subrequest is right place, added sub_write address is %lld\n",sub_req);
//printf_every_chip_read_subrequest(ssd);
//#endif


    return SUCCESS;
}


/*******************************************************************************
*insert2buffer这个函数是专门为写请求分配子请求服务的在buffer_management中被调用。
********************************************************************************/
struct ssd_info *
insert2buffer(struct ssd_info *ssd, unsigned int lpn, int state, struct sub_request *sub, struct request *req) {
    int write_back_count, flag = 0;                                                             /*flag表示为写入新数据腾空间是否完成，0表示需要进一步腾，1表示已经腾空*/
    unsigned int i, lsn, hit_flag, add_flag, sector_count, active_region_flag = 0, free_sector = 0;
    struct buffer_group *buffer_node = NULL, *pt, *new_node = NULL, key;
    struct sub_request *sub_req = NULL, *update = NULL;


    unsigned int sub_req_state = 0, sub_req_size = 0, sub_req_lpn = 0;

#ifdef DEBUG_CURR_TIME
    printf("enter insert2buffer, this is real lpn (one by one) from real ssd->request_tail, that means also the new request. current time:%lld, lpn:%d, state:%d, .so buffer->tail req's lsn is %d, size is %d, operation is %d\n",ssd->current_time,lpn,state, req->lsn, req->size, req->operation);
#endif

    sector_count = size(state);                                                                /*需要写到buffer的sector个数*/
    key.group = lpn;
    buffer_node = (struct buffer_group *) avlTreeFind(ssd->dram->buffer,
                                                      (TREE_NODE *) &key);    /*在平衡二叉树中寻找buffer node*/

    /************************************************************************************************
	*没有命中。
	*第一步根据这个lpn有多少子页需要写到buffer，去除已写回的lsn，为该lpn腾出位置，
	*首先即要计算出free sector（表示还有多少可以直接写的buffer节点）。
	*如果free_sector>=sector_count，即有多余的空间够lpn子请求写，不需要产生写回请求
	*否则，没有多余的空间供lpn子请求写，这时需要释放一部分空间，产生写回请求。就要creat_sub_request()
	*************************************************************************************************/
#ifdef DEBUG
    printf("buffer_node is %d\n",buffer_node);
#endif
    if (buffer_node == NULL) {
#ifdef DEBUG
        printf("buffer_node has not been hitted\n");
#endif

        free_sector = ssd->dram->buffer->max_buffer_sector - ssd->dram->buffer->buffer_sector_count;
        if (free_sector >= sector_count) {
            flag = 1;
        }
        if (flag == 0) {
#ifdef DEBUG
            printf("buffer not hit, free_sector remained is %d,need write into sector_count is %d\n",free_sector, sector_count);
#endif

            write_back_count = sector_count - free_sector;
            ssd->dram->buffer->write_miss_hit = ssd->dram->buffer->write_miss_hit + write_back_count;
            while (write_back_count > 0) {
                sub_req = NULL;
                sub_req_state = ssd->dram->buffer->buffer_tail->stored;
#ifdef DEBUG
                printf("next is write back buffer size amount.\n");
#endif
                sub_req_size = size(ssd->dram->buffer->buffer_tail->stored);
                sub_req_lpn = ssd->dram->buffer->buffer_tail->group;
                sub_req = creat_sub_request(ssd, sub_req_lpn, sub_req_size, sub_req_state, req, WRITE);

                /**********************************************************************************
				*req不为空，表示这个insert2buffer函数是在buffer_management中调用，传递了request进来
				*req为空，表示这个函数是在process函数中处理一对多映射关系的读的时候，需要将这个读出
				*的数据加到buffer中，这可能产生实时的写回操作，需要将这个实时的写回操作的子请求挂在
				*这个读请求的总请求上
                注意在buffer_management中的sub是NULL。
				***********************************************************************************/
                if (req != NULL) {
                } else {
#ifdef DEBUG
                    printf("opps, never happen \n");
#endif
                    sub_req->next_subs = sub->next_subs;//sub与req的区别。
                    sub->next_subs = sub_req;
                }

                /*********************************************************************
				*写请求插入到了平衡二叉树，这时就要修改dram的buffer_sector_count；
				*维持平衡二叉树调用avlTreeDel()和AVL_TREENODE_FREE()函数；维持LRU算法；
				**********************************************************************/
                //ssd->dram->buffer->buffer_sector_count = ssd->dram->buffer->buffer_sector_count - sub_req->size;
                //lxcprogram_gc original wrong.
                ssd->dram->buffer->buffer_sector_count = ssd->dram->buffer->buffer_sector_count - sub_req_size;

                pt = ssd->dram->buffer->buffer_tail;
                avlTreeDel(ssd->dram->buffer, (TREE_NODE *) pt);
                if (ssd->dram->buffer->buffer_head->LRU_link_next == NULL) {
                    ssd->dram->buffer->buffer_head = NULL;
                    ssd->dram->buffer->buffer_tail = NULL;
                } else {
                    ssd->dram->buffer->buffer_tail = ssd->dram->buffer->buffer_tail->LRU_link_pre;
                    ssd->dram->buffer->buffer_tail->LRU_link_next = NULL;
                }
                pt->LRU_link_next = NULL;
                pt->LRU_link_pre = NULL;
                AVL_TREENODE_FREE(ssd->dram->buffer, (TREE_NODE *) pt);
                pt = NULL;

                write_back_count =
//                        write_back_count - sub_req->size;                            /*因为产生了实时写回操作，需要将主动写回操作区域增加*///lxcprogram_gc original wrong
                        write_back_count - sub_req_size;                            /*因为产生了实时写回操作，需要将主动写回操作区域增加*/
#ifdef DEBUG
                printf("once write insert into buffer but writeback first,  all need write into sectore is %d, but this time writeback into sector is %d \n", write_back_count, sub_req_size);
#endif
            }
        }

        /******************************************************************************
		*生成一个buffer node，根据这个页的情况分别赋值个各个成员，添加到队首和二叉树中
		*******************************************************************************/
#ifdef DEBUG
    printf("here make a buffer-node and insert into buffer tree. should printf here\n");
#endif
        new_node = NULL;
        new_node = (struct buffer_group *) malloc(sizeof(struct buffer_group));
        alloc_assert(new_node, "buffer_group_node");
        memset(new_node, 0, sizeof(struct buffer_group));

        new_node->group = lpn;
        new_node->stored = state;
        new_node->dirty_clean = state;
        new_node->LRU_link_pre = NULL;
        new_node->LRU_link_next = ssd->dram->buffer->buffer_head;
        if (ssd->dram->buffer->buffer_head != NULL) {
            ssd->dram->buffer->buffer_head->LRU_link_pre = new_node;
        } else {
            ssd->dram->buffer->buffer_tail = new_node;
        }
        ssd->dram->buffer->buffer_head = new_node;
        new_node->LRU_link_pre = NULL;
        avlTreeAdd(ssd->dram->buffer, (TREE_NODE *) new_node);
        ssd->dram->buffer->buffer_sector_count += sector_count;
    }
        /****************************************************************************************
	*在buffer中命中的情况
	*算然命中了，但是命中的只是lpn，有可能新来的写请求，只是需要写lpn这一page的某几个sub_page
	*这时有需要进一步的判断
	*****************************************************************************************/
    else {
#ifdef DEBUG
        printf("buffer_node hit\n");
#endif

        for (i = 0; i < ssd->parameter->subpage_page; i++) {
            /*************************************************************
			*判断state第i位是不是1
			*并且判断第i个sector是否存在buffer中，1表示存在，0表示不存在。
			**************************************************************/
            if ((state >> i) % 2 != 0) {
                lsn = lpn * ssd->parameter->subpage_page + i;
                hit_flag = 0;
                hit_flag = (buffer_node->stored) & (0x00000001 << i);

                if (hit_flag !=
                    0)                                                          /*命中了，需要将该节点移到buffer的队首，并且将命中的lsn进行标记*/
                {
                    active_region_flag = 1;                                             /*用来记录在这个buffer node中的lsn是否被命中，用于后面对阈值的判定*/

                    if (req != NULL) {
                        if (ssd->dram->buffer->buffer_head != buffer_node) {
                            if (ssd->dram->buffer->buffer_tail == buffer_node) {
                                ssd->dram->buffer->buffer_tail = buffer_node->LRU_link_pre;
                                buffer_node->LRU_link_pre->LRU_link_next = NULL;
                            } else if (buffer_node != ssd->dram->buffer->buffer_head) {
                                buffer_node->LRU_link_pre->LRU_link_next = buffer_node->LRU_link_next;
                                buffer_node->LRU_link_next->LRU_link_pre = buffer_node->LRU_link_pre;
                            }
                            buffer_node->LRU_link_next = ssd->dram->buffer->buffer_head;
                            ssd->dram->buffer->buffer_head->LRU_link_pre = buffer_node;
                            buffer_node->LRU_link_pre = NULL;
                            ssd->dram->buffer->buffer_head = buffer_node;
                        }
                        ssd->dram->buffer->write_hit++;
                        req->complete_lsn_count++;                                        /*关键 当在buffer中命中时 就用req->complete_lsn_count++表示往buffer中写了数据。*/
                    } else {
                    }
                } else {
                    /************************************************************************************************************
					*该lsn没有命中，但是节点在buffer中，需要将这个lsn加到buffer的对应节点中
					*从buffer的末端找一个节点，将一个已经写回的lsn从节点中删除(如果找到的话)，更改这个节点的状态，同时将这个新的
					*lsn加到相应的buffer节点中，该节点可能在buffer头，不在的话，将其移到头部。如果没有找到已经写回的lsn，在buffer
					*节点找一个group整体写回，将这个子请求挂在这个请求上。可以提前挂在一个channel上。
					*第一步:将buffer队尾的已经写回的节点删除一个，为新的lsn腾出空间，这里需要修改队尾某节点的stored状态这里还需要
					*       增加，当没有可以之间删除的lsn时，需要产生新的写子请求，写回LRU最后的节点。
					*第二步:将新的lsn加到所述的buffer节点中。
					*************************************************************************************************************/
                    ssd->dram->buffer->write_miss_hit++;

                    if (ssd->dram->buffer->buffer_sector_count >= ssd->dram->buffer->max_buffer_sector) {
                        if (buffer_node ==
                            ssd->dram->buffer->buffer_tail)                  /*如果命中的节点是buffer中最后一个节点，交换最后两个节点*/
                        {
                            pt = ssd->dram->buffer->buffer_tail->LRU_link_pre;
                            ssd->dram->buffer->buffer_tail->LRU_link_pre = pt->LRU_link_pre;
                            ssd->dram->buffer->buffer_tail->LRU_link_pre->LRU_link_next = ssd->dram->buffer->buffer_tail;
                            ssd->dram->buffer->buffer_tail->LRU_link_next = pt;
                            pt->LRU_link_next = NULL;
                            pt->LRU_link_pre = ssd->dram->buffer->buffer_tail;
                            ssd->dram->buffer->buffer_tail = pt;

                        }
                        sub_req = NULL;
                        sub_req_state = ssd->dram->buffer->buffer_tail->stored;
                        sub_req_size = size(ssd->dram->buffer->buffer_tail->stored);
                        sub_req_lpn = ssd->dram->buffer->buffer_tail->group;
                        sub_req = creat_sub_request(ssd, sub_req_lpn, sub_req_size, sub_req_state, req, WRITE);

                        if (req != NULL) {

                        } else if (req == NULL) {
                            sub_req->next_subs = sub->next_subs;
                            sub->next_subs = sub_req;
                        }

                        ssd->dram->buffer->buffer_sector_count = ssd->dram->buffer->buffer_sector_count - sub_req->size;
                        pt = ssd->dram->buffer->buffer_tail;
                        avlTreeDel(ssd->dram->buffer, (TREE_NODE *) pt);

                        /************************************************************************/
                        /* 改:  挂在了子请求，buffer的节点不应立即删除，						*/
                        /*			需等到写回了之后才能删除									*/
                        /************************************************************************/
                        if (ssd->dram->buffer->buffer_head->LRU_link_next == NULL) {
                            ssd->dram->buffer->buffer_head = NULL;
                            ssd->dram->buffer->buffer_tail = NULL;
                        } else {
                            ssd->dram->buffer->buffer_tail = ssd->dram->buffer->buffer_tail->LRU_link_pre;
                            ssd->dram->buffer->buffer_tail->LRU_link_next = NULL;
                        }
                        pt->LRU_link_next = NULL;
                        pt->LRU_link_pre = NULL;
                        AVL_TREENODE_FREE(ssd->dram->buffer, (TREE_NODE *) pt);
                        pt = NULL;
                    }

                    /*第二步:将新的lsn加到所述的buffer节点中*/
                    add_flag = 0x00000001 << (lsn % ssd->parameter->subpage_page);

                    if (ssd->dram->buffer->buffer_head !=
                        buffer_node)                      /*如果该buffer节点不在buffer的队首，需要将这个节点提到队首*/
                    {
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
                    buffer_node->stored = buffer_node->stored | add_flag;
                    buffer_node->dirty_clean = buffer_node->dirty_clean | add_flag;
                    ssd->dram->buffer->buffer_sector_count++;
                }

            }
        }
    }

    return ssd;
}

/**************************************************************************************
*函数的功能是寻找活跃快，应为每个plane中都只有一个活跃块，只有这个活跃块中才能进行操作
***************************************************************************************/
Status
find_active_block(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die, unsigned int plane) {
    unsigned int active_block;
    unsigned int free_page_num = 0;
    unsigned int count = 0;

    active_block = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block;
    free_page_num = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_page_num;
    //last_write_page=ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_page_num;
    while ((free_page_num == 0) && (count < ssd->parameter->block_plane)) {
        active_block = (active_block + 1) % ssd->parameter->block_plane;
        free_page_num = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_page_num;
        count++;
    }
    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].active_block = active_block;
    if (count < ssd->parameter->block_plane) {
        return SUCCESS;
    } else {
        return FAILURE;
    }
}

/*************************************************
*这个函数的功能就是一个模拟一个实实在在的写操作
*就是更改这个page的相关参数，以及整个ssd的统计参数
**************************************************/
Status write_page(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die, unsigned int plane,
                  unsigned int active_block, unsigned int *ppn) {
    int last_write_page = 0;
    last_write_page = ++(ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_page);
    if (last_write_page >= (int) (ssd->parameter->page_block)) {
        ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].last_write_page = 0;
        printf("error! the last write page larger than 64!!\n");
        return ERROR;
    }

    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].free_page_num--;
    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].free_page--;
    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[active_block].page_head[last_write_page].written_count++;
    ssd->write_flash_count++;
    *ppn = find_ppn(ssd, channel, chip, die, plane, active_block, last_write_page);

    return SUCCESS;
}

/**********************************************
*这个函数的功能是根据lpn，size，state创建子请求
**********************************************/
struct sub_request *
creat_sub_request(struct ssd_info *ssd, unsigned int lpn, int size, unsigned int state, struct request *req,
                  unsigned int operation) {
    struct sub_request *sub = NULL, *sub_r = NULL;
    struct channel_info *p_ch = NULL;
    struct local *loc = NULL;
    unsigned int flag = 0;

    sub = (struct sub_request *) malloc(sizeof(struct sub_request));                        /*申请一个子请求的结构*/
    alloc_assert(sub, "sub_request");
    memset(sub, 0, sizeof(struct sub_request));

    if (sub == NULL) {
        return NULL;
    }
    sub->location = NULL;
    sub->next_node = NULL;
    sub->next_subs = NULL;
    sub->update = NULL;

    if (req != NULL) {
        sub->next_subs = req->subs;
        req->subs = sub;
    }
#ifdef DEBUG
printf("enter creat_sub_request.!!!!!!!once one buffer->tail element.so the req and size is right req。not the ssd->request_tail's request!!!!!! operation is %d, req is %x(no 0 is no NULL),lpn is %d, size is %d, state is %d\n",operation, req, lpn, size, state);
#endif
    /*************************************************************************************
	*在读操作的情况下，有一点非常重要就是要预先判断读子请求队列中是否有与这个子请求相同的，
	*有的话，新子请求就不必再执行了，将新的子请求直接赋为完成
	**************************************************************************************/
    if (operation == READ) {
        loc = find_location(ssd, ssd->dram->map->map_entry[lpn].pn);

//        printf("added read sub %lld, its location is %lld\n",sub, loc);
       if(loc == 0x0){

           printf("lxclxclxc,lpn %d, size %d, request %lld, operation %d has wrong now\n", lpn, size, req, operation);
       }
       if(lpn > 16777216){
           printf("lxclxclxc_lpnwrong in now\n");
       }
        sub->location = loc;
        sub->begin_time = ssd->current_time;
        sub->current_state = SR_WAIT;
        sub->current_time = MAX_INT64;
        sub->next_state = SR_R_C_A_TRANSFER;
        sub->next_state_predict_time = MAX_INT64;
        sub->lpn = lpn;
        sub->size = size;                                                               /*需要计算出该子请求的请求大小*/


        p_ch = &ssd->channel_head[loc->channel];
        sub->ppn = ssd->dram->map->map_entry[lpn].pn;
        sub->operation = READ;
        sub->state = (ssd->dram->map->map_entry[lpn].state & 0x7fffffff);
        sub_r = p_ch->subs_r_head;                                                      /*一下几行包括flag用于判断该读子请求队列中是否有与这个子请求相同的，有的话，将新的子请求直接赋为完成*/
        flag = 0;
        while (sub_r != NULL) {
            if (sub_r->ppn == sub->ppn) {
                flag = 1;
                break;
            }
            sub_r = sub_r->next_node;
        }
#ifdef DEBUG
        printf("flag is %d *** 1 means there is same subrequest in queue 0 is opposite\n",flag);
#endif
        if (flag == 0) {
#ifdef DEBUG
            printf("subs queue insert read,flag=0 then insert subs into pch %d\n",loc->channel);
#endif



#ifdef DEBUGSUSPEND
printf("only here add chip's read length,new creat read location is channel %d, chip %d, die %d, plane %d, ppn %d\n",loc->channel, loc->chip, loc->die, loc->plane, sub->ppn);
#endif

ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].chip_read_queue_length ++;

//                                //lxcprogram_de-prioritize.
                                //ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].chip_read_queue_length ++;
                                unsigned int temp_length = ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].chip_read_queue_length;
//这个地方就错在此处是chip的read队列长度，所以每次增加或者减少是专门特定的chip，而serviced_time也要根据chip计算。不过确实也是chip的自增加。也没有错误好像。
//                                sub->serviced_time = temp_length * (ssd->parameter->time_characteristics.tR + 7 * ssd->parameter->time_characteristics.tWC + (sub->size * ssd->parameter->subpage_capacity) * ssd->parameter->time_characteristics.tWC) + ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].chip_status_time;// length * read cost + chip stalling time.
#ifdef DEBUGSUSPEND
                                printf("chip_read_queue_length add one now is %d,[channel %d, chip %d] serviced_time is %lld\n", temp_length,sub->location->channel, sub->location->chip, sub->serviced_time);
#endif



//lxcprogram_gc original wrong
            if( ( p_ch->subs_r_head != NULL) && (p_ch->subs_r_tail != NULL)){
//                printf("length is not 1\n");
                p_ch->subs_r_tail->next_node = sub;
                p_ch->subs_r_tail = sub;
            } else if((p_ch->subs_r_head == NULL) && (p_ch->subs_r_head == NULL)){

                p_ch->subs_r_head = sub;
                p_ch->subs_r_tail = sub;
            }else{
                printf("lxclxclxc, read added wrong now\n");
            }
                
//                if(ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].chip_read_queue_length != 1){
//                printf("lxclxclxc, wrong\n");
//}

//printf("now channel %d, chip %d length is %d,  head address %lld, tail address %lld , sub %lld\n", sub->location->channel, sub->location->chip, ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].chip_read_queue_length, ssd->channel_head[sub->location->channel].subs_r_head,ssd->channel_head[sub->location->channel].subs_r_tail, sub);
  if(sub->location == NULL){
                    printf("lxclxclxc_readlocation wrong\n");
                }

//printf_every_chip_read_subrequest(ssd);
        }else {
#ifdef DEBUG
printf("subs queue status change state from SR_R_DATA_RANSFER to next_state complete and sub next time=+1000,flag=1 pch is  %d\n",p_ch);
#endif

printf("read dont have to be inserted 1000 now\n");
            sub->current_state = SR_R_DATA_TRANSFER;
            sub->current_time = ssd->current_time;
#ifdef DEBUG_CURR_TIME  //lxcv1
            printf("sub_current_time is %lld , ssd_current_time is %lld in creat_sub_request function\n", sub->current_time,ssd->current_time);
#endif

            sub->next_state = SR_COMPLETE;
            sub->next_state_predict_time = ssd->current_time + 1000;
            sub->complete_time = ssd->current_time + 1000;
        }
    }
        /*************************************************************************************
	*写请求的情况下，就需要利用到函数allocate_location(ssd ,sub)来处理静态分配和动态分配了
	**************************************************************************************/
    else if (operation == WRITE) {
        sub->ppn = 0;
        sub->operation = WRITE;
        sub->location = (struct local *) malloc(sizeof(struct local));
        alloc_assert(sub->location, "sub->location");
        memset(sub->location, 0, sizeof(struct local));

        sub->current_state = SR_WAIT;
        sub->current_time = ssd->current_time;
        sub->lpn = lpn;
        sub->size = size;
        sub->state = state;
        sub->begin_time = ssd->current_time;

        if (allocate_location(ssd, sub) == ERROR) {
            printf("lxc, must mark here, create write request fail!!\n");
            free(sub->location);
            sub->location = NULL;
            free(sub);
            sub = NULL;
            return NULL;
        }

    } else {
        printf("opps!, never happen\n");
        free(sub->location);
        sub->location = NULL;
        free(sub);
        sub = NULL;
        printf("\nERROR ! Unexpected command.\n");
        return NULL;
    }

    return sub;
}

/******************************************************
*函数的功能是在给出的channel，chip，die上面寻找读子请求
*这个子请求的ppn要与相应的plane的寄存器里面的ppn相符
*******************************************************/
struct sub_request *
find_read_sub_request(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die) {
    unsigned int plane = 0;
    unsigned int address_ppn = 0;
    struct sub_request *sub = NULL, *p = NULL;

    for (plane = 0; plane < ssd->parameter->plane_die; plane++) {
        address_ppn = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].add_reg_ppn;
#ifdef DEBUG
        //printf("find channel %d, chip %d, die %d, now the existing register_ppn is %lld, if ppn is -1, that means no useful\n", address_ppn);
#endif

        if (address_ppn != -1) {
            sub = ssd->channel_head[channel].subs_r_head;
            if (sub->ppn == address_ppn) {
                //lxcprogram_gc!! original wrong.
                return sub;


                if (sub->next_node == NULL) {
                    ssd->channel_head[channel].subs_r_head = NULL;
                    ssd->channel_head[channel].subs_r_tail = NULL;
                }
                ssd->channel_head[channel].subs_r_head = sub->next_node;
            }
            while ((sub->ppn != address_ppn) && (sub->next_node != NULL)) {
                if (sub->next_node->ppn == address_ppn) {
                    p = sub->next_node;
                    //lxcprogram_gc original wrong.
                    return p;


                    if (p->next_node == NULL) {
                        sub->next_node = NULL;
                        ssd->channel_head[channel].subs_r_tail = sub;
                    } else {
                        sub->next_node = p->next_node;
                    }
                    sub = p;
                    break;
                }
                sub = sub->next_node;
            }
            if (sub->ppn == address_ppn) {
                sub->next_node = NULL;
                printf("find read sub , sub ppn is %d\n",sub->ppn);
                return sub;
            } else {
                printf("Error! Can't find the sub request.");
            }
        }
    }
    return NULL;
}

/*******************************************************************************
*函数的功能是寻找写子请求。
*分两种情况1，要是是完全动态分配就在ssd->subs_w_head队列上找
*2，要是不是完全动态分配那么就在ssd->channel_head[channel].subs_w_head队列上查找
********************************************************************************/
struct sub_request *find_write_sub_request(struct ssd_info *ssd, unsigned int channel) {
    struct sub_request *sub = NULL, *p = NULL;
    if ((ssd->parameter->allocation_scheme == 0) && (ssd->parameter->dynamic_allocation == 0))    /*是完全的动态分配*/
    {
        sub = ssd->subs_w_head;
        while (sub != NULL) {
            if (sub->current_state == SR_WAIT) {
                if (sub->update != NULL)                                                      /*如果有需要提前读出的页*/
                {
                    if ((sub->update->current_state == SR_COMPLETE) || ((sub->update->next_state == SR_COMPLETE) &&
                                                                        (sub->update->next_state_predict_time <=
                                                                         ssd->current_time)))   //被更新的页已经被读出
                    {
                        break;
                    }
                } else {
                    break;
                }
            }
            p = sub;
            sub = sub->next_node;
        }

        if (sub ==
            NULL)                                                                      /*如果没有找到可以服务的子请求，跳出这个for循环*/
        {
            return NULL;
        }

        if (sub != ssd->subs_w_head) {
            if (sub != ssd->subs_w_tail) {
                p->next_node = sub->next_node;
            } else {
                ssd->subs_w_tail = p;
                ssd->subs_w_tail->next_node = NULL;
            }
        } else {
            if (sub->next_node != NULL) {
                ssd->subs_w_head = sub->next_node;
            } else {
                ssd->subs_w_head = NULL;
                ssd->subs_w_tail = NULL;
            }
        }
        sub->next_node = NULL;
        if (ssd->channel_head[channel].subs_w_tail != NULL) {
            ssd->channel_head[channel].subs_w_tail->next_node = sub;
            ssd->channel_head[channel].subs_w_tail = sub;
        } else {
            ssd->channel_head[channel].subs_w_tail = sub;
            ssd->channel_head[channel].subs_w_head = sub;
        }
    }
        /**********************************************************
	*除了全动态分配方式，其他方式的请求已经分配到特定的channel，
	*就只需要在channel上找出准备服务的子请求
	***********************************************************/
    else {
        sub = ssd->channel_head[channel].subs_w_head;
        while (sub != NULL) {
            if (sub->current_state == SR_WAIT) {
                if (sub->update != NULL) {
                    if ((sub->update->current_state == SR_COMPLETE) || ((sub->update->next_state == SR_COMPLETE) &&
                                                                        (sub->update->next_state_predict_time <=
                                                                         ssd->current_time)))   //被更新的页已经被读出
                    {
                        break;
                    }
                } else {
                    break;
                }
            }
            p = sub;
            sub = sub->next_node;
        }

        if (sub == NULL) {
            return NULL;
        }
    }

    return sub;
}
//lxcprogram for the process of the write operation

int services_2_write_busy_in_chip(struct ssd_info *ssd)

{
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int Has_read_request = 0;
    struct sub_request *sub = NULL, *p = NULL;
    struct sub_request *sub_for_read = NULL;
    struct sub_request *test_sub = NULL;

#ifdef DEBUGSUSPEND
    printf("services_2_write_busy_in_chip  function go into.\n");
#endif


    for (i = 0; i <
            ssd->parameter->channel_number; i++)                                       /*for the preemptive judgment and service of write operation*/
    {
        // although we need to know the serviced sub, but we can jump out according to the chip situation!! such as 1) 2) as followed.
        //1) if chip in write_preemptive_sign status, then jump out.
        //2) if chip is not in write process status, then jump out.
        //3)every time,  the finish of one process,then try to suspend the write.here use write_try_to_suspend_function. 
        //4)the finish of all write deal issue.





        //1) if chip in write_preemptive_sign status, then jump out.

        for(j = 0; j < ssd->channel_head[i].chip; j++){ 
#ifdef DEBUG
            printf("every time process function normal monister newest num_w_cycle value here the %d channel , %d chip, num_w_write is %d.\n",i ,j, ssd->channel_head[i].chip_head[j].num_w_cycle );
#endif
            if(ssd->channel_head[i].chip_head[j].write_preemptive_sign == 1) {
#ifdef DEBUGSUSPEND
                printf("here is write  preemptive ! jump out, and now the %d channel , %d chip, num_w_write is %d.\n",i ,j, ssd->channel_head[i].chip_head[j].num_w_cycle );
#endif

                continue;
            }

            //2) if chip is not in write process status, then jump out.
            if (ssd->channel_head[i].chip_head[j].current_state !=  CHIP_WRITE_DATA_CMD_BUSY){

                if (ssd->channel_head[i].chip_head[j].current_state !=  CHIP_WRITE_BUSY)  {
#ifdef DEBUGSUSPEND
                    printf("here chip is not write  status ! jump out.\n");
#endif

                    continue;

                }
            }


            //3) every time,  the finish of one process,then try to suspend the write.here use write_try_to_suspend_function.  note this:  only the condition ( current_time > chip_next_state_predict_time) is true, then begin the next process.


            if(ssd->channel_head[i].chip_head[j].next_state_predict_time <= ssd->current_time){         

                //3.1)make sure the sned_write_cmd process finish.and then judge whether there is read now.
                if(ssd->channel_head[i].chip_head[j].current_state == CHIP_WRITE_DATA_CMD_BUSY){         

                    sub = ssd->channel_head[i].subs_w_head;
                    while (sub != NULL){
                        if( (sub->current_state == SR_W_C_A_DATA_TRANSFER) && (sub->location->chip == j) ){
                            break;
                        }
                        sub = sub->next_node;
                    }
                    if(sub == NULL){
                        printf("error happen, lxclxclxc\n"); 
                    }



                    test_sub = ssd->channel_head[i].subs_w_head;
                    while (test_sub != NULL){
                        if( test_sub->location->chip == j ){
                            break;
                        }
                        test_sub = test_sub->next_node;
                    }
                    if(sub != test_sub){
                        printf("lxclxclxc1.seriously wrong, first_write_sub current_state is %d, next_state is %d\n", test_sub->current_state, test_sub->next_state);
                    }else {
                        printf("lxclxclxc2.right\n");   
                    }

#ifdef DEBUGSUSPEND
                    printf("now is judging  finish send_write_cmd process,and the sub's channel %d, chip %d , block %d, page %d(block,page has not initialed here)\n",sub->location->channel, sub->location->chip,  sub->location->block,  sub->location->page);
#endif

                    if ( (sub->current_state == SR_W_C_A_DATA_TRANSFER) && (sub->next_state == SR_W_DATA_TRANSFER) ) {

#ifdef DEBUGSUSPEND
                        printf("now will go_one_step for finishing send_write_cmd process,and the sub's channel %d, chip %d , block %d, page %d(block,page has not initialed here)\n",sub->location->channel, sub->location->chip,  sub->location->block,  sub->location->page);
#endif

                        go_one_step(ssd, sub, NULL, SR_W_DATA_TRANSFER_ONE_PROG_OF_ITERATIONS, NORMAL);                      /*状态跳变处理函数*/

#ifdef DEBUGSUSPEND
                        printf("befor num_w_cycle add 1  is %d\n",ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].num_w_cycle);
#endif
                        ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].num_w_cycle += 1;

#ifdef DEBUGSUSPEND
                        printf("after num_w_cycle add 1  is %d\n",ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].num_w_cycle);
#endif
                        /*******************judge for suspend **************************/
                        //judge for whether need suspend. if so, here store the next num_w_cycle value here is 1.
                        //but the most important thing here is !!!!!! whether the read and serviced write are  in the same chip. 

#ifdef DEBUGSUSPEND
                        printf("begin to judge for suspend the write,now.\n");
#endif

                        Has_read_request = 0;
                        sub_for_read = ssd->channel_head[i].subs_r_head;
                        while(sub_for_read != NULL)  ////here is the question!!very very very important.  whether it should happen when this chip has no read subrequest??no , must this chip's read request. other chips need another judgement. 

                        {
                            if(sub_for_read->location->chip == j)
                            {
                                break;
                            }
                            sub_for_read = sub_for_read->next_node ;
                        }
                        if(sub_for_read != NULL){ 
                            Has_read_request = 1;
#ifdef DEBUGSUSPEND
                            printf("suspend!!!!! now after send_cmd do suspend for read\n");
#endif

                            printf("write suspend position is in just finish send_read_cmd . num_w_cylce is  %d\n",ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].num_w_cycle);

                        }else{
#ifdef DEBUGSUSPEND
                            printf("don't  need to suspend.\n");
#endif


                        }
                        // mask the write suspend
                        //lxcprogram_for_adjust_write_suspension_number
                        //
                        //                        if(ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].chip_read_stop == 1){
                        //
                        //                            Has_read_request = 0;
                        //                        }
                        //lxcprogram_for_adjust_write_suspension_number

                        if ( ssd->channel_head[sub->location->channel].gc_command != NULL ){ // this is just one sign for  GC process                                 /*有gc操作，需要进行一定的判断*/
                            Has_read_request = 0;
                        }
                        Has_read_request = 0;
                        if(Has_read_request == 1) {
                            ssd->write_suspend_times ++;

                            printf("write suspend times is %d\n", ssd->write_suspend_times);
                            ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].write_preemptive_sign = 1;
                            //   ssd->current_time += (7+2) * ssd->parameter->time_characteristics.tWC;
                            //   ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].current_state = CHIP_IDLE;
                            ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].next_state = CHIP_IDLE;
                            ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].current_state = UNKNOWN;
                            ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].current_time = ssd->current_time;
                            ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].next_state_predict_time = ssd->current_time + (7+2) * ssd->parameter->time_characteristics.tWC;
                            ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].storedsub = sub;

                            printf("statistic_suspendsion_write, 0\n");


                            printf("storesuboperation channel %d,chip: %d, lpn: %d, ppn: %d  operation: %d,size: %d,   ////sub_request status:: current_state: %d, current_time: %llu, next_state: %d, next_state_predict_time: %llu, state(only for write): %d\n",sub->location->channel, sub->location->chip, sub->lpn, sub->ppn,sub->operation, sub->size, sub->current_state, sub->current_time,sub->next_state, sub->next_state_predict_time, sub->state) ;


                            printf("write suspend position is iteration. num_w_cylce is  %d, stored sub is %lld, sub is %lld\n",ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].num_w_cycle, ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].storedsub, sub);


// print next read status, 

            printf("begin show next read here, sub current_state %d, sub current_time %lld, sub next_state %d,sub next predict time %lld, sub begin_time %lld, sub channel %d, sub chip %d, sub die %d, sub plane %d, sub block %d, sub page %d, sub->lpn %d\n",sub_for_read->current_state, sub_for_read->current_time, sub_for_read->next_state, sub_for_read->next_state_predict_time, sub_for_read->begin_time, sub_for_read->location->channel, sub_for_read->location->chip, sub_for_read->location->die, sub_for_read->location->plane, sub_for_read->location->block, sub_for_read->location->page, sub_for_read->lpn);


                        }                


                        /*******************judge for suspend **************************/

                        continue;
                    }           
                }//end of 3.1

                //3.2)  now chip only has the status: chip_write_busy. and nmu_w_cycle value >1
                //and sub status always is SR_W_DATA_TRANSFER. because 3.1 go_one_step


                if( (ssd->channel_head[i].chip_head[j].num_w_cycle <= 10) && (ssd->channel_head[i].chip_head[j].num_w_cycle >= 1) ){ 




                    sub = ssd->channel_head[i].subs_w_head;
                    while (sub != NULL){
                        if( ((sub->current_state == SR_W_DATA_TRANSFER) && (sub->location->chip == j)) ) {// because chip.num_w_cycle control the sr_complete is only in sub_write!!!!
                            break;
                        }
                        sub = sub->next_node;
                    }
                    if(sub == NULL){
                        printf("error happen, lxclxclxc\n"); 
                    }

                    //3.2.1) here, the judge of sub->next_state_predict_time is a little redundance. here just run when  the write finish
                    // sub->location->xxx  equal i , j.
                    if ((sub->next_state_predict_time <= ssd->current_time) && (ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].num_w_cycle == 10) ) {

#ifdef DEBUGSUSPEND
                        printf("now will try to do last verify now sub's channel %d, chip %d , block %d, page %d(block,page has not initialed here)\n",sub->location->channel, sub->location->chip,  sub->location->block,  sub->location->page);
#endif
#ifdef DEBUGSUSPEND
                        printf("sub current_satate is (%d)=SR_COMPLETE or sub->next_state is (%d)=SR_COMPLETE,\n",sub->current_state, sub->next_state);
#endif

                        //sub, channel, chip status don't have to be updated 
                        if(ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].current_state == CHIP_WRITE_BUSY){
                            ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].current_state = CHIP_IDLE;

                        }

                        //        sub->current_state = SR_COMPLETE;
#ifdef DEBUGSUSPEND
                        printf("begin get_ppn operation now,\n");
#endif
                        //lxc_programgc orignal wrong
                        sub->complete_time = sub->next_state_predict_time;
                        sub->next_state = SR_COMPLETE;


                        get_ppn(ssd, sub->location->channel, sub->location->chip, sub->location->die, sub->location->plane, sub);

#ifdef DEBUGSUSPEND
                        printf("begin delete shown above sub_write now\n");
#endif

                        printf("delete write sub request is: %d, lpn: %d, ppn: %d  operation: %d,size: %d,   ////sub_request status:: current_state: %d, current_time: %llu, next_state: %d, next_state_predict_time: %llu, state(only for write): %d, chip's num_w_cycle is %d, chip's write_preemtive_sign is %d\n", sub->location->chip, sub->lpn, sub->ppn,sub->operation, sub->size, sub->current_state, sub->current_time, sub->next_state, sub->next_state_predict_time, sub->state, ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].num_w_cycle,ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].write_preemptive_sign ) ;
                        delete_w_sub_request(ssd, sub->location->channel, sub);
                        ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].num_w_cycle = 0;

                        //            if (sub !=
                        //                    ssd->channel_head[i].subs_r_head)                             /*if the request is completed, we delete it from read queue */
                        //            {
                        //                p->next_node = sub->next_node;
                        //            } else {
                        //                if (ssd->channel_head[i].subs_r_head != ssd->channel_head[i].subs_r_tail) {
                        //                    ssd->channel_head[i].subs_r_head = sub->next_node;
                        //                } else {
                        //                    ssd->channel_head[i].subs_r_head = NULL;
                        //                    ssd->channel_head[i].subs_r_tail = NULL;
                        //                }
                        //            }



                        //lxcprogram_for_adjust_write_suspension_number

                        //                        if(ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].chip_read_stop == 1){
                        //
                        //                            ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].chip_read_stop = 0;
                        //                        }
                        //
                        //                        if(ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].chip_read_real_stop == 1){
                        //
                        //                            ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].chip_read_real_stop = 0;
                        //
                        //                        }
                        //lxcprogram_for_adjust_write_suspension_number




                        continue;
                    }// end of 3.2.1



                    // 3.2.2) normal iteration for program now.
                    // for the iterations process of program.
                    else if( (sub->next_state_predict_time <= ssd->current_time) && (ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].num_w_cycle < 10) && (ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].num_w_cycle > 0) ) {
#ifdef DEBUGSUSPEND
                        printf("now will  do normal iteration now sub's channel %d, chip %d , block %d, page %d(block,page has not initialed here)\n",sub->location->channel, sub->location->chip,  sub->location->block,  sub->location->page);
#endif

#ifdef DEBUGSUSPEND
                        printf("now go into write 3.2.2!! %d channel, %d chip, num_w_cycle = %d\n",sub->location->channel, sub->location->chip,ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].num_w_cycle );
#endif

                        if( ((ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].num_w_cycle) % 2) == 1 ){

                            go_one_step( ssd, sub, NULL,SR_W_DATA_TRANSFER_ONE_VERIFY_OF_ITERATIONS, NORMAL);

#ifdef DEBUGSUSPEND
                            printf("befor num_w_cycle add 1  is %d\n",ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].num_w_cycle);
#endif
                            ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].num_w_cycle += 1;

#ifdef DEBUGSUSPEND
                            printf("after num_w_cycle add 1  is %d\n",ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].num_w_cycle);
#endif
                        }else{

#ifdef DEBUGSUSPEND
                            printf("befor num_w_cycle add 1  is %d\n",ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].num_w_cycle);
#endif
                            go_one_step( ssd, sub, NULL, SR_W_DATA_TRANSFER_ONE_PROG_OF_ITERATIONS, NORMAL);
                            ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].num_w_cycle += 1;

#ifdef DEBUGSUSPEND
                            printf("after num_w_cycle add 1  is %d\n",ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].num_w_cycle);
#endif
                        }
                        if( (ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].num_w_cycle) == 10){
#ifdef DEBUGSUSPEND
                            printf("num_w_cycle == 10\n");
#endif
                            //here lxcprogram_gc original channged 
                            //  sub->next_state = SR_COMPLETE;
                        }


                        //                        /*******************added the realistic mode for write **************************/
                        //if( ((ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].num_w_cycle) == 6) && (((sub->location->plane) % 2) == 1) ){
                        //
                        //
                        //                      //      printf("now sub in channel %d, chip %d, die %d, plane %d, block %d, page %d, and divide is %d just do 6 iterations\n",sub->location->channel, sub->location->chip, sub->location->die, sub->location->plane, sub->location->block,  sub->location->page, sub->location->page % 2);
                        //#ifdef DEBUGSUSPEND
                        //                            printf("now will try to do last verify now sub's channel %d, chip %d , block %d, page %d(block,page has not initialed here)\n",sub->location->channel, sub->location->chip,  sub->location->block,  sub->location->page);
                        //                            printf("now will try to do last verify now sub's channel %d, chip %d , block %d, page %d(block,page has not initialed here)\n",sub->location->channel, sub->location->chip,  sub->location->block,  sub->location->page);
                        //#endif
                        //#ifdef DEBUGSUSPEND
                        //                            printf("sub current_satate is (%d)=SR_COMPLETE or sub->next_state is (%d)=SR_COMPLETE,\n",sub->current_state, sub->next_state);
                        //#endif
                        //
                        //                            //sub, channel, chip status don't have to be updated 
                        //                            if(ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].current_state == CHIP_WRITE_BUSY){
                        //                                ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].current_state = CHIP_IDLE;
                        //
                        //                            }
                        //
                        //                            //        sub->current_state = SR_COMPLETE;
                        //#ifdef DEBUGSUSPEND
                        //                            printf("begin get_ppn operation now,\n");
                        //#endif
                        //                            //lxc_programgc orignal wrong
                        //                            sub->complete_time = sub->next_state_predict_time;
                        //                            sub->next_state = SR_COMPLETE;
                        //
                        //
                        //                            get_ppn(ssd, sub->location->channel, sub->location->chip, sub->location->die, sub->location->plane, sub);
                        //
                        //#ifdef DEBUGSUSPEND
                        //                            printf("begin delete shown above sub_write now\n");
                        //#endif
                        //
                        //                            delete_w_sub_request(ssd, sub->location->channel, sub);
                        //                            ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].num_w_cycle = 0;
                        //                            continue;
                        //                        }
                        //
                        //
                        //                        /*******************added the realistic mode for write **************************/
                        //



                        /*******************judge for suspend **************************/
                        //judge for whether need suspend. if so, here store the next num_w_cycle value here is 1.
                        //but the most important thing here is !!!!!! whether the read and serviced write are  in the same chip. 
#ifdef DEBUGSUSPEND
                        printf("begin to judge for suspend the write,now.\n");
#endif

                        Has_read_request = 0;
                        sub_for_read = ssd->channel_head[i].subs_r_head;
                        while(sub_for_read != NULL)  ////here is the question!!very very very important.  whether it should happen when this chip has no read subrequest??no , must this chip's read request. other chips need another judgement. 

                        {
                            if(sub_for_read->location->chip == j)
                            {
                                break;
                            }
                            sub_for_read = sub_for_read->next_node ;
                        }
                        if(sub_for_read != NULL){ 
                            Has_read_request = 1;
#ifdef DEBUGSUSPEND
                            printf("suspend!!!!! now during program  do suspend for read, num_w_cycle= %d\n",ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].num_w_cycle);
#endif

                        }else{
#ifdef DEBUGSUSPEND
                            printf("don't need to suspend now \n");
#endif
                        } 
                        // mask the write suspend
                        // Has_read_request = 0;

                        if ( ssd->channel_head[sub->location->channel].gc_command != NULL ){ // this is just one sign for  GC process                                 /*有gc操作，需要进行一定的判断*/
                            Has_read_request = 0;
                        }



                        //lxcprogram_for_adjust_write_suspension_number

                        //
                        //                            if(NUMBER_OF_WRITE_ITERATION +(ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].chip_write_record_begin) > 10 ){
                        //                            //remend the NUMBER_OF_WRITE_ITERATION to least number
                        //                                    ssd->channel_head[sub->location->channel].chip_head[sub->location->channel].chip_write_record_begin = 10 - NUMBER_OF_WRITE_ITERATION;
                        //
                        //                            }
                        //
                        //                        if(ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].num_w_cycle == (NUMBER_OF_WRITE_ITERATION + ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].chip_write_record_begin) ){
                        //
                        //                            ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].chip_read_stop = 0;
                        //                            ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].chip_read_real_stop = 0;
                        //
                        //                            ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].chip_write_record_begin = 0;
                        //
                        //
                        //                        }
                        //
                        //
                        //
                        //
                        //
                        //                        if(ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].chip_read_stop == 1){
                        //
                        //
                        //                                Has_read_request = 0;
                        //
                        //                        }// end of chip_read_stop;
                        //
                        //
                        //lxcprogram_for_adjust_write_suspension_number








                        if(Has_read_request == 1) {

                            ssd->write_suspend_times ++;
                            printf("write suspend times is %d\n", ssd->write_suspend_times);
                            ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].write_preemptive_sign = 1;
                            //                            ssd->current_time += (7+2) * ssd->parameter->time_characteristics.tWC;
                            //                            ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].current_state = CHIP_IDLE;
                            ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].next_state = CHIP_IDLE;
                            ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].current_state = UNKNOWN;
                            ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].current_time = ssd->current_time;
                            ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].next_state_predict_time = ssd->current_time + (7+2) * ssd->parameter->time_characteristics.tWC;

                            ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].storedsub = sub;

                            printf("statistic_suspendsion_write, 1\n");


                            printf("storesuboperation channel %d,chip: %d, lpn: %d, ppn: %d  operation: %d,size: %d,   ////sub_request status:: current_state: %d, current_time: %llu, next_state: %d, next_state_predict_time: %llu, state(only for write): %d\n",sub->location->channel, sub->location->chip, sub->lpn, sub->ppn,sub->operation, sub->size, sub->current_state, sub->current_time,sub->next_state, sub->next_state_predict_time, sub->state) ;


                            printf("write suspend position is iteration. num_w_cylce is  %d, stored sub is %lld, sub is %lld\n",ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].num_w_cycle, ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].storedsub, sub);


                            printf("statistic_suspension_write, %d, %lld\n",ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].num_w_cycle, ssd->current_time);

                            printf("write suspend position is iteration. num_w_cylce is  %d, stored sub is %lld\n",ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].num_w_cycle, ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].storedsub);


// print next read status, 

            printf("begin show next read here, sub current_state %d, sub current_time %lld, sub next_state %d,sub next predict time %lld, sub begin_time %lld, sub channel %d, sub chip %d, sub die %d, sub plane %d, sub block %d, sub page %d, sub->lpn %d\n",sub_for_read->current_state, sub_for_read->current_time, sub_for_read->next_state, sub_for_read->next_state_predict_time, sub_for_read->begin_time, sub_for_read->location->channel, sub_for_read->location->chip, sub_for_read->location->die, sub_for_read->location->plane, sub_for_read->location->block, sub_for_read->location->page, sub_for_read->lpn);



                        }        
                        /*******************judge for suspend **************************/

                        continue;



                    }// end of 3.2.2

                }// end of 3.2

            }//end of time is enough to finish one step.
            else{
#ifdef DEBUG
                printf("ssd->channel_head[%d].chip_head[%d].next_state_predict_time[%lld] is not <= ssd->current_time[%lld],so give up\n", i, j, ssd->channel_head[i].chip_head[j].next_state_predict_time,ssd->current_time);
#endif
            }

        }//end of chip loop.

    }//end of channel loop.

    return SUCCESS;

}



//lxcprogram_gc
//for restore from gc preemptive.
int try_to_restore_gc_function(struct ssd_info *ssd, struct gc_operation *gc_node, unsigned int channel, unsigned int chip)
{
    unsigned int j = -1;
    //before the next executing, judge the chip's read queue
    j = judging_read_in_gc_chip(ssd, channel);
    if(j == 1){//恢复使用，处理了中断标志和恢复了chip的时间。和下一个状态的时间。
        gc_node->sign_for_preemptive = 0;
        ssd->channel_head[channel].chip_head[chip].current_state = gc_node->chip_status;
        ssd->channel_head[channel].chip_head[chip].next_state_predict_time = ssd->current_time + gc_node->chip_next_predict_time_diff;
        
        ssd->channel_head[channel].chip_head[chip].next_state = gc_node->chip_next_status;
        printf("now gc can be restored here,channel is %d \n", channel);
        printf_gc_node_information(gc_node);
#ifdef DEBUGSUSPEND
        printf("now gc can be restored here\n");
#endif
        return 1;
    }
#ifdef DEBUG
    printf("now gc can't restore here, for there are reads here.\n");
#endif
    return 0;
}


//lxcprogram_gc for suspend gc


int gc_try_to_suspend_function(struct ssd_info *ssd,  struct gc_operation *gc_node, unsigned int channel, unsigned int chip, unsigned int current_state, unsigned int next_state, int time)
{
    unsigned int j = -1;
    //before the next executing, judge the chip's read queue
    j = judging_read_in_gc_chip(ssd, channel);
    // mask the gc preemptive
    j = 1;
    if(j == 0){// 0 means there are read requests existing.此时是已经存在read 但是chip繁忙。所以需要suspend，current时间并不是最新的read时间，因为自己选择前一阶段完成以后，所以此处就是完全正确的时间可用。只用给current_time加上7个命令时间即可。恢复的时候，current_time不用改变，只用将chip状态恢复以及，断点位置恢复即可，断点位置不用恢复，本来是什么，就没有改变。只用恢复chip忙的状态不让read随便占用即可。但是还有一点，恢复的时候，要恢复next的时间！！！这样才能连续。
        gc_node->chip_status = current_state;
        gc_node->chip_next_status = next_state;
        gc_node->sign_for_preemptive = 1;
        gc_node->chip_next_predict_time_diff = time;
//        ssd->current_time += (7 * ssd->parameter->time_characteristics.tWC);//可以往后加，因为最少，目前这个read的next要比current时间靠后，下一次同样可以运行起来
        ssd->channel_head[channel].chip_head[chip].current_state = UNKNOWN;

//add lxcprogram_gc
        ssd->channel_head[channel].chip_head[chip].current_time = ssd->current_time;
        ssd->channel_head[channel].chip_head[chip].next_state = CHIP_IDLE; 
        ssd->channel_head[channel].next_state_predict_time = ssd->current_time +(7 * ssd->parameter->time_characteristics.tWC) ;


        //ssd->debug_sign = 1;// begin to printf the gc_suspend
        ssd->gc_suspend_times ++;
printf("statistic_suspension_gc, victim_block_number,read, write, write_inner,  erase,  channel, chip, die, plane, current_time %d,%d,%d,%d,%d, %d,%d,%d,%d,%lld\n", gc_node->victim_block_number, gc_node->read, gc_node->write, gc_node->gc_write_inner, gc_node->erase, channel, gc_node->chip, gc_node->die, gc_node->plane, ssd->current_time);
        printf("gc suspend times is %d\n", ssd->gc_suspend_times);
        printf("now in preemptive state here\n");
        printf_gc_node_information(gc_node);
#ifdef DEBUG
        printf("now in preemptive state here\n");
#endif
        return 1;
    }
#ifdef DEBUG
    printf("dont have to suspend here\n");
#endif
    return 0;
} 







//lxcprogram for the process of the erase operation
int services_2_gc_busy_in_chip(struct ssd_info *ssd)
{
    unsigned int i = 0;
    unsigned int j = 0;
    struct gc_operation *gc_node = NULL;
    unsigned int current_state = 0, next_state = 0;
    long long next_state_predict_time = 0;

    struct local *location = NULL;
    unsigned int channel, chip, die, plane, block, transfer_size;
#ifdef DEBUG
    printf("go into gc busy process here\n");
#endif


    if(ssd->debug_sign == 1){
//printf("every process can debug now\n");
//#define DEBUG 
//#define DEBUGQUEUE 
//#define DEBUGSUSPEND
//#define DEBUGTRACE_OUT 
//#define DEBUG_CURR_TIME
        }



    for (i = 0; i < ssd->parameter->channel_number; i++) {

        gc_node = ssd->channel_head[i].gc_command;

     //   printf("here is  channel %d in gc_busy judging now.\n", i);
        if(gc_node == NULL){
#ifdef DEBUGONLYGC
            printf("here channel %d has no gc ! jump out.\n", i);
#endif
        continue;
        }
        channel = i;
        chip = gc_node->chip;
        die = gc_node->die;
        plane = gc_node->plane;
        block = gc_node->block;// actually gc_node->block = gc_node->victim_block_number

        if (gc_node->sign_for_preemptive == 1){
#ifdef DEBUGONLYGC
            printf("here is gc preemptive ! jump out.\n");
#endif
            continue;

        }

#ifdef DEBUGONLYGC
        
        printf("here is channel %d begin gc_busy_deal now.\n", i);
        printf_gc_node_information (gc_node);
#endif


        if (gc_node != NULL) {
#ifdef DEBUG
            printf("gc must be unNULL since this is the channel gc.\n");
#endif

            current_state = ssd->channel_head[channel].chip_head[chip].current_state;
            next_state = ssd->channel_head[channel].chip_head[chip].next_state;
            next_state_predict_time = ssd->channel_head[channel].chip_head[chip].next_state_predict_time;
        }

// here, if chip is not the GC related status, then jump out.
if( (current_state != CHIP_GC_RW_BUSY) && (current_state != CHIP_ERASE_BUSY) )  {

#ifdef DEBUG
            printf("here channel's chip is not gc status ! jump out.\n");
#endif

continue;

}






        //1、 for read_transfer_write  intermediate process
        if ((gc_node->read_write_end == -1) && (next_state_predict_time <= ssd->current_time)){// this position only for channel IDLE and chip IDLE for the begining of (the read and write in GC)
#ifdef DEBUG
            printf("in read_transfer_write part\n");
#endif

#ifdef DEBUG
            printf_ssd_gc_node(ssd);
#endif
       



            //1.1、 for read_transfer_write's  read part intermediate process(first, make sure the finish of send_read_cmd.second, then go into move data from chip to register. )
            if( (gc_node->read == 1) && (gc_node->write == 0) && (gc_node->gc_read_inner == 0)) {
#ifdef DEBUG
                printf("gc read phase, read iterations is %d,and inner sequence is %d \n", gc_node->read, gc_node->gc_read_inner);
#endif



                //for read read_data_to_register phase of once read_transfer_write process.
                ssd->channel_head[channel].chip_head[chip].current_state = CHIP_GC_RW_BUSY;
                ssd->channel_head[channel].chip_head[chip].current_time = ssd->current_time;
                ssd->channel_head[channel].chip_head[chip].next_state = CHIP_IDLE; 

                ssd->channel_head[channel].chip_head[chip].next_state_predict_time = ssd->current_time + ssd->parameter->time_characteristics.tR;


                gc_node->gc_read_inner = 1;// the sign is the begin of the (move data to register) process.


//suspension judgement.
                if( gc_try_to_suspend_function(ssd,  gc_node, channel, chip,  CHIP_GC_RW_BUSY, CHIP_IDLE, ssd->parameter->time_characteristics.tR) == 1 ){
               //return 1;
                    continue;
           }



                continue;
                //return 1;

            }//end of 1.1


            //1.2、 for read_transfer_write's read part intermediate process (make sure the completion of read and then turn into the begining of gc_write )
            if( (gc_node->read == 1) && (gc_node->write == 0) && (gc_node->gc_read_inner == 1)) {
#ifdef DEBUG
                printf("gc read phase, read iterations is %d,and inner sequence is %d \n", gc_node->read, gc_node->gc_read_inner);

#endif
                //already finish all the read, because the time current_time > tR time.then begin to go to write.所以下一个状态是进入write，然后将时间段定为：write的命令阶段。 

                gc_node->gc_read_inner = -1; //restore this value for next gc_read.
                gc_node->read = 0;
                gc_node->write = 1; //prepare for gc_write.
               //此处不用专门给chip 是 idle，前面的next_state就够了。且此处不用判断中断，本来就可以执行read，因为chip空闲了。 
                //return 1;

                continue;
            }//end of 1.2

            //1.3、 for read_transfer_write's write part intermediate process (make sure the completion of send_write_cmd and then begin to write the data in register into the flash )
            if( (gc_node->read == 0) && (gc_node->write == 1) && (gc_node->gc_write_inner >= 0) && (gc_node->gc_write_inner <= 9)) {
#ifdef DEBUG
                printf("channel %d, gc write phase, write iterations is %d,and inner sequence is %d \n",channel,  gc_node->write, gc_node->gc_write_inner);

#endif



                //for program of once write process write_data_from_register phase of once read_transfer_write process.
                ssd->channel_head[channel].chip_head[chip].current_state = CHIP_GC_RW_BUSY;
                ssd->channel_head[channel].chip_head[chip].current_time = ssd->current_time;
                ssd->channel_head[channel].chip_head[chip].next_state = CHIP_GC_RW_BUSY; 

                ssd->channel_head[channel].chip_head[chip].next_state_predict_time = ssd->current_time + 20000;
          //      ssd->channel_head[channel].chip_head[chip].next_state_predict_time = ssd->current_time + ssd->parameter->time_characteristics.tPROG_w_progam;
                gc_node->gc_write_inner ++; //this is the sign of the begining of move data from flash to flash.



//suspension judgement.
                if ( gc_try_to_suspend_function(ssd,  gc_node, channel, chip,  CHIP_GC_RW_BUSY, CHIP_GC_RW_BUSY, 20000) == 1){
                    //return 1;

                    continue;
                }



                continue;
                //return 1;

            }//end of 1.3



            //1.4、 for read_transfer_write's write part intermediate process (make sure the completion of writing register data to flash.then judge another read_transfer_write or erase begin )
            if( (gc_node->read == 0) && (gc_node->write == 1) && (gc_node->gc_write_inner == 10)) {
#ifdef DEBUG
                printf("gc write phase, write iterations is %d,and inner sequence is %d \n", gc_node->write, gc_node->gc_write_inner);

#endif
                //for program of once write process write_data_from_register phase of once read_transfer_write process.


                ssd->channel_head[channel].chip_head[chip].next_state = CHIP_IDLE; 

                if(gc_node->last_page_in_victim_deal == 0)
                {
                    gc_node->read_write_end = 0;// because, this means the last page.

                    location = (struct local *) malloc(sizeof(struct local));
                    alloc_assert(location, "location");
                    memset(location, 0, sizeof(struct local));

                    location->channel = channel;
                    location->chip = chip;
                    location->die = die;
                    location->plane = plane;
                    location->block = block;
                    location->page = gc_node->copy_page_number;
                    move_page(ssd, location,
                            &transfer_size);                       
                    //return 1;
                    continue;
                }else{

                    gc_node->gc_write_inner = -1; //this is the sign of the begining of next iteration of read_transfer_write.
                    gc_node->read = 1; 
                    gc_node->write = 0; //prepare for gc_write.




//for real operation move_page. since here is the end of one page copyback.
                    location = (struct local *) malloc(sizeof(struct local));
                    alloc_assert(location, "location");
                    memset(location, 0, sizeof(struct local));

                    location->channel = channel;
                    location->chip = chip;
                    location->die = die;
                    location->plane = plane;
                    location->block = block;
                    location->page = gc_node->copy_page_number;
                    move_page(ssd, location,
                            &transfer_size);                                                   /*真实的move_page操作*/
                    //page_move_count++;
                    //lxcprogram_gc
                    free(location);
                    location = NULL;



                    continue;
                    //return 1;
                }
            }//end of 1.4
#ifdef DEBUG
            printf_ssd_gc_node(ssd);
#endif

        }//end of read_transfer_write process.





        //2.0 for erase middle process 
        if ((gc_node->read_write_end == 0) && (next_state_predict_time <= ssd->current_time)){

#ifdef DEBUG
            printf("in erase  part\n");
#endif
#ifdef DEBUG
            printf_ssd_gc_node(ssd);
#endif


            //2.1、 for erase intermediate process (make sure the completion of send_erase_cmd  and then go into the erase pulse process)
            if(gc_node->erase < (ssd->erase_iterations - 1)){

//            if (gc_node->erase == 0)  {
#ifdef DEBUG
                printf("gc erase phase, erase iterations is %d\n", gc_node->erase);

#endif
                //for erase phase of once erase process. 
                ssd->channel_head[channel].chip_head[chip].current_state = CHIP_ERASE_BUSY;
                ssd->channel_head[channel].chip_head[chip].current_time = ssd->current_time;
                ssd->channel_head[channel].chip_head[chip].next_state = CHIP_ERASE_BUSY; 

                ssd->channel_head[channel].chip_head[chip].next_state_predict_time = ssd->current_time + ssd->parameter->time_characteristics.tBERS/ssd->erase_iterations;

                gc_node->erase += 1; //this means now in erase pulse process.

//suspension judgement.
                if( gc_try_to_suspend_function(ssd,  gc_node, channel, chip,  CHIP_ERASE_BUSY, CHIP_ERASE_BUSY, ssd->parameter->time_characteristics.tBERS/ ssd->erase_iterations) == 1){// 这个地方只是说，send_cmd完成了，即将开始erase pulse。所以，这个暂停是停在send_erase_cmd之前的。
                  //return 1;

                  continue;
              }


                continue;
              // return 1;
//            }//end of 2.1

}//end of among of erase iteration

            //2.2、 for erase intermediate process (make sure the completion of erase pulse process and then go into verify process )
            if (gc_node->erase == (ssd->erase_iterations - 1))  {
#ifdef DEBUG
                printf("gc erase phase, erase iterations is %d\n", gc_node->erase);

#endif

                //for erase phase of once erase process. 
                ssd->channel_head[channel].chip_head[chip].current_state = CHIP_ERASE_BUSY;
                ssd->channel_head[channel].chip_head[chip].current_time = ssd->current_time;
                ssd->channel_head[channel].chip_head[chip].next_state = CHIP_IDLE; 

                ssd->channel_head[channel].chip_head[chip].next_state_predict_time = ssd->current_time + ssd->parameter->time_characteristics.tBERS/ ssd->erase_iterations;
                gc_node->erase = ssd->erase_iterations; //this means now in erase verigy process.

//suspension judgement.
                if( gc_try_to_suspend_function(ssd,  gc_node, channel, chip,  CHIP_ERASE_BUSY, CHIP_IDLE, ssd->parameter->time_characteristics.tBERS/ssd->erase_iterations) == 1){
                  //return 1;

                  continue;
              }


 
               // return 1;

                continue;
            }//end of 2.2


            //2.3、 for erase intermediate process (make sure the completion of erase verify process then deal the real operation after once GC )
            if (gc_node->erase == ssd->erase_iterations)  {
#ifdef DEBUG
                printf("gc erase phase, erase iterations is %d\n", gc_node->erase);

#endif

                //for erase phase of once erase process. 
           //     ssd->channel_head[channel].chip_head[chip].current_state = CHIP_ERASE_BUSY;
           //     ssd->channel_head[channel].chip_head[chip].current_time = ssd->current_time;
           //     ssd->channel_head[channel].chip_head[chip].next_state = CHIP_IDLE; 

           //     ssd->channel_head[channel].chip_head[chip].next_state_predict_time = ssd->current_time + ssd->parameter->time_characteristics.tBERS/2;


                //real deal after the GC process

                erase_operation(ssd, channel, chip, die, plane, block);       

                printf("now gc finished in channel %d\n", i);
                delete_gc_node(ssd, channel, gc_node); 

               // ssd->gc_request--;
                continue;

                //return 1;



            }//end of 2.3




        } //end of erase intermediate process


    }// end of channel for loop.
return 1;
}//end of services_2_gc_busy_in_chip. 






/*********************************************************************************************
*专门为读子请求服务的函数
*1，只有当读子请求的当前状态是SR_R_C_A_TRANSFER
*2，读子请求的当前状态是SR_COMPLETE或者下一状态是SR_COMPLETE并且下一状态到达的时间比当前时间小
**********************************************************************************************/
int  services_2_r_cmd_trans_and_complete(struct ssd_info *ssd) {
    unsigned int i = 0;
    struct sub_request *sub = NULL, *p = NULL;
    //lxcprogram_gc
    int temp_chip_number = 0 ; 
    int tempchannel_read_length = 0;

    struct gc_operation *gc_node = NULL;
    unsigned int chip ;
#ifdef DEBUGSUSPEND
                    printf("go into services_2_r_cmd_trans_and_complete\n");
#endif


    for (i = 0; i <
            ssd->parameter->channel_number; i++)                                       /*这个循环处理不需要channel的时间(读命令已经到达chip，chip由ready变为busy)，当读请求完成时，将其从channel的队列中取出*/
    {
        sub = ssd->channel_head[i].subs_r_head;

        while (sub != NULL) {
            
            //printf("lxc,now is wrong read here.channel is %d, \n", i);
            if(sub->location == NULL){
                printf("lxc,now is wrong read here.channel is %d, \n", i);
            }

     //       printf("wrong sub is %lld \n", sub);
            if(i != sub->location->channel){
                printf("here what kind of wrong???\n");
            }
            chip = sub->location->chip;
#ifdef DEBUGSUSPEND


            printf(":services_2_r_cmd_trans_and_complete sub information: channel %d, chip %d, die %d, plane %d, ppn %d,sub->current_time %lld, sub->next_time %lld,ssd-> current_time %lld, sub->current_state %d, sub->next_state %d\n",sub->location->channel, sub->location->chip, sub->location->die, sub->location->plane, sub->ppn, sub->current_time, sub->next_state_predict_time, ssd->current_time,sub->current_state, sub->next_state );

#endif
            if (sub->current_state ==
                    SR_R_C_A_TRANSFER)                                  /*读命令发送完毕，将对应的die置为busy，同时修改sub的状态; 这个部分专门处理读请求由当前状态为传命令变为die开始busy，die开始busy不需要channel为空，所以单独列出*/
            {
#ifdef DEBUGSUSPEND
                printf("sub current_state is SR_R_C_A_TRANSFER,sub is %d\n",sub);
#endif

                if (sub->next_state_predict_time <= ssd->current_time) {
                    go_one_step(ssd, sub, NULL, SR_R_READ, NORMAL);                      /*状态跳变处理函数*/

                }
            }
            sub = sub->next_node;
        }//end of send_cmd judgement.

    }//end of for channel loop.

        //for complete.
     for (i = 0; i <
            ssd->parameter->channel_number; i++)                                       /*这个循环处理不需要channel的时间(读命令已经到达chip，chip由ready变为busy)，当读请求完成时，将其从channel的队列中取出*/
    {
        sub = ssd->channel_head[i].subs_r_head;
        p = sub;
        while (sub != NULL) {
            
            //printf("lxc,now is wrong read here.channel is %d, \n", i);
            if(sub->location == NULL){
                printf("lxc,now is wrong read here.channel is %d, \n", i);
            }

     //       printf("wrong sub is %lld \n", sub);
            if(i != sub->location->channel){
                printf("here what kind of wrong???\n");
            }
            chip = sub->location->chip;
           
         if ((sub->current_state == SR_COMPLETE) ||
                    ((sub->next_state == SR_COMPLETE) && (sub->next_state_predict_time <= ssd->current_time))) {


//lxcprogram_for_adjust_write_suspension_number

// if the chip has chip_read_stop sign value and the same chip just finished one read sub_request, then should give the priority to write sub_request in the same chip, only when gc is not happening. since if gc is happening,there is no right for corresponding channel's write sub_request. 
//               unsigned int temp_num_write = 0; 
//                    temp_num_write = statistic_write_request_number(ssd);
//                    if(temp_num_write > 0){
//                        if(ssd->channel_head[sub->location->channel].gc_command == NULL ){
//                            ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].chip_read_real_stop = 1;
//                        }
//                    }
//
//
//


//lxcprogram_for_adjust_write_suspension_number





#ifdef DEBUGSUSPEND
                printf("now go into read complete function.sub->current_satate is (%d)=SR_COMPLETE or sub->next_state is (%d)=COMPLETE,\n",sub->current_state, sub->next_state);
#endif
                //lxcprogram_de-prioritize
//                if(ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].chip_read_queue_length > 0){
//
                //1. reduce length in this chip
                    ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].chip_read_queue_length --;
//#ifdef DEBUGSUSPEND
//                    printf("channel [%d], chip [%d], chip_read_queue_length minus one now is %d\n", sub->location->channel, sub->location->chip, ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].chip_read_queue_length);
//#endif
//
//                }
//2. calculate all length in channel i
//                    printf("now channel %d, chip is %d,  head address %lld, tail address %lld, moved sub is %lld\n", i, sub->location->chip, ssd->channel_head[i].subs_r_head,ssd->channel_head[i].subs_r_tail, sub);
                    for(temp_chip_number = 0, tempchannel_read_length = 0; temp_chip_number < (ssd->parameter->chip_num / ssd->parameter->channel_number); temp_chip_number ++){
                        printf("channel %d, chip %d, read_length is %d\n", i, temp_chip_number, ssd->channel_head[i].chip_head[temp_chip_number].chip_read_queue_length);
                        tempchannel_read_length += ssd->channel_head[i].chip_head[temp_chip_number].chip_read_queue_length;
                    } 

                if (sub !=
                        ssd->channel_head[i].subs_r_head)                             /*if the request is completed, we delete it from read queue */
                {
                    printf("not head\n");
                  //  p->next_node = sub->next_node;
                    //lxcprogram_gc so important!!! original wrong!!!
                    if(sub == ssd->channel_head[i].subs_r_tail){

                        p->next_node = sub->next_node;
                        ssd->channel_head[i].subs_r_tail = p;
                        //ssd->channel_head[i].subs_r_tail->next_node = NULL;
                        break;
                    }else {
                        p->next_node = sub->next_node;
                   //     p = sub; // now p should stay here!!!
                        sub = sub->next_node;
                    }
                    if(ssd->channel_head[i].subs_r_head->location == 0){   

                        printf("lxclxclxc2_deletereadwrongin channelhead\n");
                    }
                    //printf("now channel %d, length is %d,  head address %lld, tail address %lld, moved sub is %lld\n", i, tempchannel_read_length, ssd->channel_head[i].subs_r_head,ssd->channel_head[i].subs_r_tail, sub);
                   // ssd->channel_head[i].chip_head[chip].chip_read_queue_length --;
                } else {

                    if(tempchannel_read_length != 0){
                        ssd->channel_head[i].subs_r_head = ssd->channel_head[i].subs_r_head->next_node;
                        sub = sub->next_node;
                        p = sub;
                       // ssd->channel_head[i].chip_head[chip].chip_read_queue_length --;
                    } else {
                       // printf("here tail = NULL");
                        ssd->channel_head[i].subs_r_head = NULL;
                        ssd->channel_head[i].subs_r_tail = NULL;
                        break;
                        //   ssd->channel_head[i].chip_head[chip].chip_read_queue_length --;
                    }

                    //printf("now channel %d, length is %d,  head address %lld, tail address %lld, \n", i, tempchannel_read_length, ssd->channel_head[i].subs_r_head,ssd->channel_head[i].subs_r_tail);
                    if(ssd->channel_head[i].subs_r_head->location == 0){   

                        printf("lxclxclxc1_deletereadwrongin channelhead\n");
                    }

                }

//                    printf("after moved now channel %d, length is %d,  head address %lld,0x %x,  tail address %lld, 0x%x \n", i, tempchannel_read_length, ssd->channel_head[i].subs_r_head,ssd->channel_head[i].subs_r_head,ssd->channel_head[i].subs_r_tail,ssd->channel_head[i].subs_r_tail );
                   if((tempchannel_read_length != 0) && ( ssd->channel_head[i].subs_r_head == 0)){
                   printf("stop here\n");
                   }
                    //printf_every_chip_read_subrequest(ssd);
            }else {// end of complete or c_a_cmd  state
            p = sub;
            sub = sub->next_node;

            
            }
         if(ssd->channel_head[i].subs_r_head->location == 0){   

             printf("lxclxclxc_deletereadwrongin channelhead\n");
         }
        }// end of complement judgement.
    }//end of for channel loop.


        //judge for gc restore after every one read finished. //gc is channel gc
        //lxcprogram_gc 
//    for (i = 0; i <
//            ssd->parameter->channel_number; i++) { 
//        gc_node = ssd->channel_head[i].gc_command;
//        if(gc_node != NULL){
//
//            chip = gc_node->chip;
//        }
//
//        if((gc_node != NULL) && (gc_node->sign_for_preemptive == 1) ){
//            try_to_restore_gc_function(ssd, gc_node, i, chip);
//        }
//
//    }
//

    //judge for suspended write after every one read finished. because this function contains channel loop and chip loop independently.  
    try_to_restore_write_function(ssd);


    return SUCCESS;
}

/**************************************************************************
*这个函数也是只处理读子请求，处理chip当前状态是CHIP_WAIT，
*或者下一个状态是CHIP_DATA_TRANSFER并且下一状态的预计时间小于当前时间的chip
***************************************************************************/
Status services_2_r_data_trans(struct ssd_info *ssd, unsigned int channel, unsigned int *channel_busy_flag,
                               unsigned int *change_current_time_flag) {
    int chip = 0;
    unsigned int die = 0, plane = 0, address_ppn = 0, die1 = 0;
    struct sub_request *sub = NULL, *p = NULL, *sub1 = NULL;
    struct sub_request *sub_twoplane_one = NULL, *sub_twoplane_two = NULL;
    struct sub_request *sub_interleave_one = NULL, *sub_interleave_two = NULL;
    for (chip = 0; chip < ssd->channel_head[channel].chip; chip++) {
        if ((ssd->channel_head[channel].chip_head[chip].current_state == CHIP_WAIT) ||
            ((ssd->channel_head[channel].chip_head[chip].next_state == CHIP_DATA_TRANSFER) &&
             (ssd->channel_head[channel].chip_head[chip].next_state_predict_time <= ssd->current_time))) {
            for (die = 0; die < ssd->parameter->die_chip; die++) {
                sub = find_read_sub_request(ssd, channel, chip, die);                   /*在channel,chip,die中找到读子请求*/
                if (sub != NULL) {
                    break;
                }
            }

            if (sub == NULL) {
                continue;
            }

#ifdef DEBUGSUSPEND
            printf("SERVICES_2_r_data_trans, sub from find_read_sub_request,sub information: channel %d, chip %d, die %d, plane %d, ppn %d\n",sub->location->channel, sub->location->chip, sub->location->die, sub->location->plane, sub->ppn);
#endif



            /**************************************************************************************
				*如果ssd支持高级命令，那没我们可以一起处理支持AD_TWOPLANE_READ，AD_INTERLEAVE的读子请求
				*1，有可能产生了two plane操作，在这种情况下，将同一个die上的两个plane的数据依次传出
				*2，有可能产生了interleave操作，在这种情况下，将不同die上的两个plane的数据依次传出
				***************************************************************************************/
            if (((ssd->parameter->advanced_commands & AD_TWOPLANE_READ) == AD_TWOPLANE_READ) ||
                ((ssd->parameter->advanced_commands & AD_INTERLEAVE) == AD_INTERLEAVE)) {
                if ((ssd->parameter->advanced_commands & AD_TWOPLANE_READ) ==
                    AD_TWOPLANE_READ)     /*有可能产生了two plane操作，在这种情况下，将同一个die上的两个plane的数据依次传出*/
                {
#ifdef DEBUG
                    printf("only has ad_twoplane_read\n");
#endif
                    sub_twoplane_one = sub;
                    sub_twoplane_two = NULL;
                    /*为了保证找到的sub_twoplane_two与sub_twoplane_one不同，令add_reg_ppn=-1*/
                    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[sub->location->plane].add_reg_ppn = -1;
                    sub_twoplane_two = find_read_sub_request(ssd, channel, chip,
                                                             die);               /*在相同的channel,chip,die中寻找另外一个读子请求*/

                    /******************************************************
						*如果找到了那么就执行TWO_PLANE的状态转换函数go_one_step
						*如果没找到那么就执行普通命令的状态转换函数go_one_step
						******************************************************/
                    if (sub_twoplane_two == NULL) {
                        go_one_step(ssd, sub_twoplane_one, NULL, SR_R_DATA_TRANSFER, NORMAL);
                        *change_current_time_flag = 0;
                        *channel_busy_flag = 1;

                    } else {
                        go_one_step(ssd, sub_twoplane_one, sub_twoplane_two, SR_R_DATA_TRANSFER, TWO_PLANE);
                        *change_current_time_flag = 0;
                        *channel_busy_flag = 1;

                    }
                } else if ((ssd->parameter->advanced_commands & AD_INTERLEAVE) ==
                           AD_INTERLEAVE)      /*有可能产生了interleave操作，在这种情况下，将不同die上的两个plane的数据依次传出*/
                {
#ifdef DEBUG
                    printf("opps, never here for ad_interleave\n");
#endif
                    sub_interleave_one = sub;
                    sub_interleave_two = NULL;
                    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[sub->location->plane].add_reg_ppn = -1;

                    for (die1 = 0; die1 < ssd->parameter->die_chip; die1++) {
                        if (die1 != die) {
                            sub_interleave_two = find_read_sub_request(ssd, channel, chip,
                                                                       die1);    /*在相同的channel，chhip不同的die上面找另外一个读子请求*/
                            if (sub_interleave_two != NULL) {
                                break;
                            }
                        }
                    }
                    if (sub_interleave_two == NULL) {
                        go_one_step(ssd, sub_interleave_one, NULL, SR_R_DATA_TRANSFER, NORMAL);

                        *change_current_time_flag = 0;
                        *channel_busy_flag = 1;

                    } else {
                        go_one_step(ssd, sub_twoplane_one, sub_interleave_two, SR_R_DATA_TRANSFER, INTERLEAVE);

                        *change_current_time_flag = 0;
                        *channel_busy_flag = 1;

                    }
                }
            } else                                                                                 /*如果ssd不支持高级命令那么就执行一个一个的执行读子请求*/
            {
#ifdef DEBUGSUSPEND
                printf("SERVICES_2_r_data_trans, in chip  enter go_one_step(SR_R_DATA_TRANSFER) now\n");
#endif

                go_one_step(ssd, sub, NULL, SR_R_DATA_TRANSFER, NORMAL);
                *change_current_time_flag = 0;
                *channel_busy_flag = 1;

            }
            break;
        }

        if (*channel_busy_flag == 1) {
            break;
        }
    }
    return SUCCESS;
}


/******************************************************
*这个函数也是只服务读子请求，并且处于等待状态的读子请求
*******************************************************/
int services_2_r_wait(struct ssd_info *ssd, unsigned int channel, unsigned int *channel_busy_flag,
        unsigned int *change_current_time_flag) {
    unsigned int plane = 0, address_ppn = 0;
    struct sub_request *sub = NULL, *p = NULL;
    struct sub_request *sub_twoplane_one = NULL, *sub_twoplane_two = NULL;
    struct sub_request *sub_interleave_one = NULL, *sub_interleave_two = NULL;

    struct sub_request *temp_for_writesub = ssd->channel_head[channel].subs_w_head;
    struct gc_operation *temp_gc_node = NULL;
    //if(){
    //
    //temp_gc_node
    //}
    sub = ssd->channel_head[channel].subs_r_head;

    printf("enter services_2_r_wait, and now channel is %d\n", channel);
#ifdef DEBUGSUSPEND
    printf("enter services_2_r_wait\n");
#endif

    if ((ssd->parameter->advanced_commands & AD_TWOPLANE_READ) ==
            AD_TWOPLANE_READ)         /*to find whether there are two sub request can be served by two plane operation*/
    {
#ifdef DEBUGSUSPEND
        printf("enter services_2_r_wait AD_TWOPLANE_READ\n");
#endif


        sub_twoplane_one = NULL;
        sub_twoplane_two = NULL;
        /*寻找能执行two_plane的两个读子请求*/
        find_interleave_twoplane_sub_request(ssd, channel, sub_twoplane_one, sub_twoplane_two, TWO_PLANE);

        if (sub_twoplane_two != NULL)                                                     /*可以执行two plane read 操作*/
        {
#ifdef DEBUG
            printf("opps, never happen,enter services_2_r_wait\n");
#endif

            go_one_step(ssd, sub_twoplane_one, sub_twoplane_two, SR_R_C_A_TRANSFER, TWO_PLANE);

            *change_current_time_flag = 0;
            *channel_busy_flag = 1;                                                       /*已经占用了这个周期的总线，不用执行die中数据的回传*/
        } else if ((ssd->parameter->advanced_commands & AD_INTERLEAVE) !=
                AD_INTERLEAVE)       /*没有满足条件的两个page，，并且没有interleave read命令时，只能执行单个page的读*/
        {
#ifdef DEBUG
            printf("opps1, never happen also here, services_2_r_wait\n");
#endif

            while (sub !=
                    NULL)                                                            /*if there are read requests in queue, send one of them to target die*/
            {
                if (sub->current_state == SR_WAIT) {                                                                        /*注意下个这个判断条件与services_2_r_data_trans中判断条件的不同
                                                                                                                             */
                    if ((ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].current_state ==
                                CHIP_IDLE) ||
                            ((ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].next_state ==
                              CHIP_IDLE) &&
                             (ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].next_state_predict_time <=
                              ssd->current_time))) {
                        go_one_step(ssd, sub, NULL, SR_R_C_A_TRANSFER, NORMAL);

                        *change_current_time_flag = 0;
                        *channel_busy_flag = 1;                                           /*已经占用了这个周期的总线，不用执行die中数据的回传*/
                        break;
                    } else {
                        /*因为die的busy导致的阻塞*/
                    }
                }
                sub = sub->next_node;
            }
        }
    }
    if ((ssd->parameter->advanced_commands & AD_INTERLEAVE) ==
            AD_INTERLEAVE)               /*to find whether there are two sub request can be served by INTERLEAVE operation*/
    {
#ifdef DEBUGSUSPEND
        printf("enter services_2_r_wait INTERLEAVE\n");
#endif


        sub_interleave_one = NULL;
        sub_interleave_two = NULL;
        find_interleave_twoplane_sub_request(ssd, channel, sub_interleave_one, sub_interleave_two, INTERLEAVE);

        if (sub_interleave_two != NULL)                                                  /*可以执行interleave read 操作*/
        {
#ifdef DEBUG
            printf("opps2, never happen services_2_r_wait\n");
#endif


            go_one_step(ssd, sub_interleave_one, sub_interleave_two, SR_R_C_A_TRANSFER, INTERLEAVE);

            *change_current_time_flag = 0;
            *channel_busy_flag = 1;                                                      /*已经占用了这个周期的总线，不用执行die中数据的回传*/
        } else                                                                           /*没有满足条件的两个page，只能执行单个page的读*/
        {
#ifdef DEBUG
            printf("in services_2_r_wait, only here\n");
#endif

            while (sub !=
                    NULL)                                                           /*if there are read requests in queue, send one of them to target die*/
            {
                if (sub->current_state == SR_WAIT) {
                    if ((ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].current_state ==
                                CHIP_IDLE) ||
                            ((ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].next_state ==
                              CHIP_IDLE) &&
                             (ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].next_state_predict_time <=
                              ssd->current_time))) {

                        go_one_step(ssd, sub, NULL, SR_R_C_A_TRANSFER, NORMAL);

                        *change_current_time_flag = 0;
                        *channel_busy_flag = 1;                                          /*已经占用了这个周期的总线，不用执行die中数据的回传*/
                        break;
                    }

                    else{
                        /*因为die的busy导致的阻塞*/
                    }
                }
                sub = sub->next_node;
            }
        }
    }

    /*******************************
     *ssd不能执行执行高级命令的情况下
     *******************************/
    if (((ssd->parameter->advanced_commands & AD_INTERLEAVE) != AD_INTERLEAVE) &&
            ((ssd->parameter->advanced_commands & AD_TWOPLANE_READ) != AD_TWOPLANE_READ)) {
        while (sub !=
                NULL)                                                               /*if there are read requests in queue, send one of them to target chip*/

        {
            if(sub->location->channel == 5){
                printf("begin enter services_2_r_wait NO advance commands,here is sub information, but, some of them are not useful, now channel %d, chip %d,sub current_state %d, sub current_time %lld, sub next_state %d,sub next predict time %lld, sub begin_time %lld, sub channel %d, sub chip %d, sub die %d, sub plane %d, sub block %d, sub page %d, sub->lpn %d\n",sub->location->channel, sub->location->chip, sub->current_state, sub->current_time, sub->next_state, sub->next_state_predict_time, sub->begin_time, sub->location->channel, sub->location->chip, sub->location->die, sub->location->plane, sub->location->block, sub->location->page, sub->lpn);
            }
#ifdef DEBUGSUSPEND
            printf("begin enter services_2_r_wait NO advance commands,here is sub information, but, some of them are not useful, sub current_state %d, sub current_time %lld, sub next_state %d,sub next predict time %lld, sub begin_time %lld, sub channel %d, sub chip %d, sub die %d, sub plane %d, sub block %d, sub page %d, sub->lpn %d\n",sub->current_state, sub->current_time, sub->next_state, sub->next_state_predict_time, sub->begin_time, sub->location->channel, sub->location->chip, sub->location->die, sub->location->plane, sub->location->block, sub->location->page, sub->lpn);
#endif

            if (sub->current_state == SR_WAIT) {

                if ((ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].current_state ==
                            CHIP_IDLE) ||
                        ((ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].next_state ==
                          CHIP_IDLE) &&
                         (ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].next_state_predict_time <=
                          ssd->current_time))) {


                    printf("begin enter services_2_r_wait NO advance commands,here is sub information, but, some of them are not useful, now channel %d, chip %d,sub current_state %d, sub current_time %lld, sub next_state %d,sub next predict time %lld, sub begin_time %lld, sub channel %d, sub chip %d, sub die %d, sub plane %d, sub block %d, sub page %d, sub->lpn %d\n",sub->location->channel, sub->location->chip, sub->current_state, sub->current_time, sub->next_state, sub->next_state_predict_time, sub->begin_time, sub->location->channel, sub->location->chip, sub->location->die, sub->location->plane, sub->location->block, sub->location->page, sub->lpn);

                    // FCFS:    jump out if read is later than write sub in this destination chip.
                    //                        while(temp_for_writesub != NULL ){
                    //                            if (temp_for_writesub->location->chip == sub->location->chip){
                    //
                    //                                break;
                    //                            }else{
                    //                                temp_for_writesub = temp_for_writesub->next_node;
                    //                            }
                    //                        }
                    //                        printf("temp_for_writesub %lld\n", temp_for_writesub);
                    //                        if (temp_for_writesub != NULL){
                    //                            if(temp_for_writesub->begin_time < sub->begin_time){
                    //
                    //                                printf("read should give up to write, time is %lld, write time is %lld \n", temp_for_writesub->begin_time, sub->begin_time );
                    //
                    //                                ssd->diff_between_FCFS_RF++;
                    //                                if(sub->location->chip != temp_for_writesub->location->chip){
                    //                                printf("FCFS seriously wrong\n");
                    //                                }
                    //                                break;
                    //                            }
                    //                        }
                    //


                    // FCFS:    jump out if read is later than write sub in this destination chip.
                    if(ssd->channel_head[channel].gc_command != NULL){
                        if(sub->location->chip != ssd->channel_head[channel].gc_command->chip){





                            go_one_step(ssd, sub, NULL, SR_R_C_A_TRANSFER, NORMAL);

                            *change_current_time_flag = 0;
                            *channel_busy_flag = 1;                                              /*已经占用了这个周期的总线，不用执行die中数据的回传*/
                            break;
                        } else {
                            /*因为die的busy导致的阻塞*/
                        }

                    }else{
                        go_one_step(ssd, sub, NULL, SR_R_C_A_TRANSFER, NORMAL);

                        *change_current_time_flag = 0;
                        *channel_busy_flag = 1;                                              /*已经占用了这个周期的总线，不用执行die中数据的回传*/
                        break;


                    }
                }
            }
            sub = sub->next_node;
        }
    }

    return SUCCESS;
}

//judge whether write sub_request exist in pointed chip

int judge_write_existing_in_chip(struct ssd_info *ssd, int channel, int chip)// return 1 means write exit in this chip
{


    struct sub_request *temp_sub = NULL;
    temp_sub = ssd->channel_head[channel].subs_r_head;
    while(temp_sub != NULL){

        if(temp_sub->location->chip == chip){
            if(temp_sub->operation == 0){

                return 1;
            }
            temp_sub = temp_sub->next_node;
        }else{
            temp_sub = temp_sub->next_node;
        }

    }
return 0;

}



/*********************************************************************
*当一个写子请求处理完后，要从请求队列上删除，这个函数就是执行这个功能。
**********************************************************************/
int delete_w_sub_request(struct ssd_info *ssd, unsigned int channel, struct sub_request *sub) {
    struct sub_request *p = NULL;



    //lxcprogram_gc
ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].chip_write_queue_length --;
#ifdef DEBUGSUSPEND
    printf("channel [%d], chip [%d],die [%d], block[%d], page[%d], write queue length minus one %d\n",sub->location->channel, sub->location->chip, sub->location->die,  sub->location->block,  sub->location->page, ssd->channel_head[sub->location->channel].chip_head[sub->location->chip].chip_write_queue_length);
#endif

    if (sub == ssd->channel_head[channel].subs_w_head)                                   /*将这个子请求从channel队列中删除*/
    {
        if (ssd->channel_head[channel].subs_w_head != ssd->channel_head[channel].subs_w_tail) {
            ssd->channel_head[channel].subs_w_head = sub->next_node;
        } else {
            ssd->channel_head[channel].subs_w_head = NULL;
            ssd->channel_head[channel].subs_w_tail = NULL;
        }
    } else {
        p = ssd->channel_head[channel].subs_w_head;
        while (p->next_node != sub) {
            p = p->next_node;
        }

        if (sub->next_node != NULL) {
            p->next_node = sub->next_node;
        } else {
            p->next_node = NULL;
            ssd->channel_head[channel].subs_w_tail = p;
        }
    }

    return SUCCESS;
}

/*
*函数的功能就是执行copyback命令的功能，
*/
Status
copy_back(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die, struct sub_request *sub) {
    int old_ppn = -1, new_ppn = -1;
    long long time = 0;
    if (ssd->parameter->greed_CB_ad == 1)                                               /*允许贪婪使用copyback高级命令*/
    {
        old_ppn = -1;
        if (ssd->dram->map->map_entry[sub->lpn].state !=
            0)                             /*说明这个逻辑页之前有写过，需要使用copyback+random input命令，否则直接写下去即可*/
        {
            if ((sub->state & ssd->dram->map->map_entry[sub->lpn].state) == ssd->dram->map->map_entry[sub->lpn].state) {
                sub->next_state_predict_time = ssd->current_time + 7 * ssd->parameter->time_characteristics.tWC +
                                               (sub->size * ssd->parameter->subpage_capacity) *
                                               ssd->parameter->time_characteristics.tWC;
            } else {
                sub->next_state_predict_time = ssd->current_time + 19 * ssd->parameter->time_characteristics.tWC +
                                               ssd->parameter->time_characteristics.tR +
                                               (sub->size * ssd->parameter->subpage_capacity) *
                                               ssd->parameter->time_characteristics.tWC;
                ssd->copy_back_count++;
                ssd->read_count++;
                ssd->update_read_count++;
                old_ppn = ssd->dram->map->map_entry[sub->lpn].pn;                       /*记录原来的物理页，用于在copyback时，判断是否满足同为奇地址或者偶地址*/
            }
        } else {
            sub->next_state_predict_time = ssd->current_time + 7 * ssd->parameter->time_characteristics.tWC +
                                           (sub->size * ssd->parameter->subpage_capacity) *
                                           ssd->parameter->time_characteristics.tWC;
        }
        sub->complete_time = sub->next_state_predict_time;
        time = sub->complete_time;

        get_ppn(ssd, sub->location->channel, sub->location->chip, sub->location->die, sub->location->plane, sub);

        if (old_ppn !=
            -1)                                                              /*采用了copyback操作，需要判断是否满足了奇偶地址的限制*/
        {
            new_ppn = ssd->dram->map->map_entry[sub->lpn].pn;
            while (old_ppn % 2 != new_ppn % 2)                                              /*没有满足奇偶地址限制，需要再往下找一页*/
            {
                get_ppn(ssd, sub->location->channel, sub->location->chip, sub->location->die, sub->location->plane,
                        sub);
                ssd->program_count--;
                ssd->write_flash_count--;
                ssd->waste_page_count++;
                new_ppn = ssd->dram->map->map_entry[sub->lpn].pn;
            }
        }
    } else                                                                              /*不能贪婪的使用copyback高级命令*/
    {
        if (ssd->dram->map->map_entry[sub->lpn].state != 0) {
            if ((sub->state & ssd->dram->map->map_entry[sub->lpn].state) == ssd->dram->map->map_entry[sub->lpn].state) {
                sub->next_state_predict_time = ssd->current_time + 7 * ssd->parameter->time_characteristics.tWC +
                                               (sub->size * ssd->parameter->subpage_capacity) *
                                               ssd->parameter->time_characteristics.tWC;
                get_ppn(ssd, sub->location->channel, sub->location->chip, sub->location->die, sub->location->plane,
                        sub);
            } else {
                old_ppn = ssd->dram->map->map_entry[sub->lpn].pn;                       /*记录原来的物理页，用于在copyback时，判断是否满足同为奇地址或者偶地址*/
                get_ppn(ssd, sub->location->channel, sub->location->chip, sub->location->die, sub->location->plane,
                        sub);
                new_ppn = ssd->dram->map->map_entry[sub->lpn].pn;
                if (old_ppn % 2 == new_ppn % 2) {
                    ssd->copy_back_count++;
                    sub->next_state_predict_time = ssd->current_time + 19 * ssd->parameter->time_characteristics.tWC +
                                                   ssd->parameter->time_characteristics.tR +
                                                   (sub->size * ssd->parameter->subpage_capacity) *
                                                   ssd->parameter->time_characteristics.tWC;
                } else {
                    sub->next_state_predict_time = ssd->current_time + 7 * ssd->parameter->time_characteristics.tWC +
                                                   ssd->parameter->time_characteristics.tR +
                                                   (size(ssd->dram->map->map_entry[sub->lpn].state)) *
                                                   ssd->parameter->time_characteristics.tRC +
                                                   (sub->size * ssd->parameter->subpage_capacity) *
                                                   ssd->parameter->time_characteristics.tWC;
                }
                ssd->read_count++;
                ssd->update_read_count++;
            }
        } else {
            sub->next_state_predict_time = ssd->current_time + 7 * ssd->parameter->time_characteristics.tWC +
                                           (sub->size * ssd->parameter->subpage_capacity) *
                                           ssd->parameter->time_characteristics.tWC;
            get_ppn(ssd, sub->location->channel, sub->location->chip, sub->location->die, sub->location->plane, sub);
        }
        sub->complete_time = sub->next_state_predict_time;
        time = sub->complete_time;
    }

    /****************************************************************
	*执行copyback高级命令时，需要修改channel，chip的状态，以及时间等
	*****************************************************************/
    ssd->channel_head[channel].current_state = CHANNEL_TRANSFER;
    ssd->channel_head[channel].current_time = ssd->current_time;
    ssd->channel_head[channel].next_state = CHANNEL_IDLE;
    ssd->channel_head[channel].next_state_predict_time = time;

    ssd->channel_head[channel].chip_head[chip].current_state = CHIP_WRITE_BUSY;
    ssd->channel_head[channel].chip_head[chip].current_time = ssd->current_time;
    ssd->channel_head[channel].chip_head[chip].next_state = CHIP_IDLE;
    ssd->channel_head[channel].chip_head[chip].next_state_predict_time =
            time + ssd->parameter->time_characteristics.tPROG;

    return SUCCESS;
}

/*****************
*静态写操作的实现
******************/
Status
static_write(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die, struct sub_request *sub) {
    long long time = 0;
    if (ssd->dram->map->map_entry[sub->lpn].state !=
        0)                                    /*说明这个逻辑页之前有写过，需要使用先读出来，再写下去，否则直接写下去即可*/
    {
        if ((sub->state & ssd->dram->map->map_entry[sub->lpn].state) ==
            ssd->dram->map->map_entry[sub->lpn].state)   /*可以覆盖*/
        {
            sub->next_state_predict_time = ssd->current_time + 7 * ssd->parameter->time_characteristics.tWC +
                                           (sub->size * ssd->parameter->subpage_capacity) *
                                           ssd->parameter->time_characteristics.tWC;
        } else {
            sub->next_state_predict_time = ssd->current_time + 7 * ssd->parameter->time_characteristics.tWC +
                                           ssd->parameter->time_characteristics.tR +
                                           (size((ssd->dram->map->map_entry[sub->lpn].state ^ sub->state))) *
                                           ssd->parameter->time_characteristics.tRC +
                                           (sub->size * ssd->parameter->subpage_capacity) *
                                           ssd->parameter->time_characteristics.tWC;
            ssd->read_count++;
            ssd->update_read_count++;
        }
    } else {
        sub->next_state_predict_time = ssd->current_time + 7 * ssd->parameter->time_characteristics.tWC +
                                       (sub->size * ssd->parameter->subpage_capacity) *
                                       ssd->parameter->time_characteristics.tWC;
    }
    sub->complete_time = sub->next_state_predict_time;
    time = sub->complete_time;

    get_ppn(ssd, sub->location->channel, sub->location->chip, sub->location->die, sub->location->plane, sub);

    /****************************************************************
	*执行copyback高级命令时，需要修改channel，chip的状态，以及时间等
	*****************************************************************/
    ssd->channel_head[channel].current_state = CHANNEL_TRANSFER;
    ssd->channel_head[channel].current_time = ssd->current_time;
    ssd->channel_head[channel].next_state = CHANNEL_IDLE;
    ssd->channel_head[channel].next_state_predict_time = time;

    ssd->channel_head[channel].chip_head[chip].current_state = CHIP_WRITE_BUSY;
    ssd->channel_head[channel].chip_head[chip].current_time = ssd->current_time;
    ssd->channel_head[channel].chip_head[chip].next_state = CHIP_IDLE;
    ssd->channel_head[channel].chip_head[chip].next_state_predict_time =
            time + ssd->parameter->time_characteristics.tPROG;

    return SUCCESS;
}
//lxcprogram just for the begin step which occupys the channel and chip simultaneously.
/********************
新写子请求的处理函数
*********************/
int  new_services_2_write(struct ssd_info *ssd, unsigned int channel, unsigned int *channel_busy_flag,
                        unsigned int *change_current_time_flag) {
    int j = 0, chip = 0;
    unsigned int k = 0;
    unsigned int old_ppn = 0, new_ppn = 0;
    unsigned int chip_token = 0, die_token = 0, plane_token = 0, address_ppn = 0;
    unsigned int die = 0, plane = 0;
    long long time = 0;
    struct sub_request *sub = NULL, *p = NULL;
    struct sub_request *sub_twoplane_one = NULL, *sub_twoplane_two = NULL;
    struct sub_request *sub_interleave_one = NULL, *sub_interleave_two = NULL;

    /************************************************************************************************************************
	*写子请求挂在两个地方一个是channel_head[channel].subs_w_head，另外一个是ssd->subs_w_head，所以要保证至少有一个队列不为空
	*同时子请求的处理还分为动态分配和静态分配。
	*************************************************************************************************************************/
#ifdef DEBUG
    printf("enter services_2_write, channel is %d, channel_busy_flag is %d, change_current_time_flag is %d\n",channel, *channel_busy_flag, *change_current_time_flag);
#endif
    if ((ssd->channel_head[channel].subs_w_head != NULL) || (ssd->subs_w_head != NULL)) {
        if (ssd->parameter->allocation_scheme == 0)                                       /*动态分配*/
        {
            for (j = 0; j < ssd->channel_head[channel].chip; j++) {
                if ((ssd->channel_head[channel].subs_w_head == NULL) && (ssd->subs_w_head == NULL)) {
                    break;
                }
                

                chip_token = ssd->channel_head[channel].token;                            /*令牌*/
#ifdef DEBUG
    printf("channel %d's chip token is %d\n", channel, chip_token);
#endif


                if (*channel_busy_flag == 0) {
                    if ((ssd->channel_head[channel].chip_head[chip_token].current_state == CHIP_IDLE) ||
                        ((ssd->channel_head[channel].chip_head[chip_token].next_state == CHIP_IDLE) &&
                         (ssd->channel_head[channel].chip_head[chip_token].next_state_predict_time <=
                          ssd->current_time))) {
                        if ((ssd->channel_head[channel].subs_w_head == NULL) && (ssd->subs_w_head == NULL)) {
                            break;
                        }
                        die_token = ssd->channel_head[channel].chip_head[chip_token].token;
#ifdef DEBUG
                        printf("channel %d's chip token is %d,die token is %d\n", channel, chip_token, die_token);
#endif


                        if (((ssd->parameter->advanced_commands & AD_INTERLEAVE) != AD_INTERLEAVE) &&
                            ((ssd->parameter->advanced_commands & AD_TWOPLANE) !=
                             AD_TWOPLANE))       //can't use advanced commands
                        {
#ifdef DEBUG
                            printf("if you have not set advanced command,then only do normal read and write\n");
#endif

                            sub = find_write_sub_request(ssd, channel);
                            if (sub == NULL) {
                                break;
                            }

                            if (sub->current_state == SR_WAIT) {
                                plane_token = ssd->channel_head[channel].chip_head[chip_token].die_head[die_token].token;

                                get_ppn(ssd, channel, chip_token, die_token, plane_token, sub);

                                ssd->channel_head[channel].chip_head[chip_token].die_head[die_token].token =
                                        (ssd->channel_head[channel].chip_head[chip_token].die_head[die_token].token +
                                         1) % ssd->parameter->plane_die;

                                *change_current_time_flag = 0;

                                if (ssd->parameter->ad_priority2 == 0) {
                                    ssd->real_time_subreq--;
                                }
                                go_one_step(ssd, sub, NULL, SR_W_TRANSFER, NORMAL);       /*执行普通的状态的转变。*/
                                delete_w_sub_request(ssd, channel, sub);                /*删掉处理完后的写子请求*/

                                *channel_busy_flag = 1;
                                /**************************************************************************
									*跳出for循环前，修改令牌
									*这里的token的变化完全取决于在这个channel chip die plane下写是否成功
									*成功了就break 没成功token就要变化直到找到能写成功的channel chip die plane
									***************************************************************************/
                                ssd->channel_head[channel].chip_head[chip_token].token =
                                        (ssd->channel_head[channel].chip_head[chip_token].token + 1) %
                                        ssd->parameter->die_chip;
                                ssd->channel_head[channel].token =
                                        (ssd->channel_head[channel].token + 1) % ssd->parameter->chip_channel[channel];
                                break;
                            }
                        } else                                                          /*use advanced commands*/
                        {
                            if (dynamic_advanced_process(ssd, channel, chip_token) == NULL) {
                                *channel_busy_flag = 0;
#ifdef DEBUG
    printf("channel %d's chip token is %d, there is not suit operation in this channel\n", channel, chip_token);
#endif

                            } else {
#ifdef DEBUG
    printf("channel %d's chip token is %d, here get a dynamic_advance_process\n", channel, chip_token);
#endif

                                *channel_busy_flag = 1;                                 /*执行了一个请求，传输了数据，占用了总线，需要跳出到下一个channel*/
                                ssd->channel_head[channel].chip_head[chip_token].token =
                                        (ssd->channel_head[channel].chip_head[chip_token].token + 1) %
                                        ssd->parameter->die_chip;
                                ssd->channel_head[channel].token =
                                        (ssd->channel_head[channel].token + 1) % ssd->parameter->chip_channel[channel];
#ifdef DEBUG
                                printf("channel---token update (only move ahead) channel %d's chip token is %d\n", channel, ssd->channel_head[channel].token);
#endif

                                break;
                            }
                        }

                        ssd->channel_head[channel].chip_head[chip_token].token =
                                (ssd->channel_head[channel].chip_head[chip_token].token + 1) % ssd->parameter->die_chip;
#ifdef DEBUG
                        printf("die---token update (only move head), then new token is channel %d's chip token is %d, die_token is %d\n", channel, chip_token, ssd->channel_head[channel].chip_head[chip_token].token);
#endif

                    }
                }

                ssd->channel_head[channel].token =
                        (ssd->channel_head[channel].token + 1) % ssd->parameter->chip_channel[channel];
            }
        } else if (ssd->parameter->allocation_scheme == 1)                                     /*静态分配*/
        {
#ifdef DEBUGSUSPEND
    printf("static allocation.....now, and very important, channel here is %d.\n", channel);
#endif
            for (chip = 0; chip < ssd->channel_head[channel].chip; chip++) {

                if ((ssd->channel_head[channel].chip_head[chip].current_state == CHIP_IDLE) ||
                    ((ssd->channel_head[channel].chip_head[chip].next_state == CHIP_IDLE) &&
                     (ssd->channel_head[channel].chip_head[chip].next_state_predict_time <= ssd->current_time))) {
                    if (ssd->channel_head[channel].subs_w_head == NULL) {
                        break;
                    }
                    if (*channel_busy_flag == 0) {

                        if (((ssd->parameter->advanced_commands & AD_INTERLEAVE) != AD_INTERLEAVE) &&
                            ((ssd->parameter->advanced_commands & AD_TWOPLANE) != AD_TWOPLANE))     /*不执行高级命令*/
                        {
#ifdef DEBUGSUSPEND
                            printf("new_services_2_write static operation. Here marks that there is no use of interleave and twoplane techonologies. \n");

#endif
                       //     for (die = 0; die < ssd->channel_head[channel].chip_head[chip].die_num; die++) {
                                if (ssd->channel_head[channel].subs_w_head == NULL) {
                                    break;
                                }
#ifdef DEBUGSUSPEND
                                printf("for loop,channel is %d, chip is %d, die is %d\n",channel, chip, die);

#endif

                                sub = ssd->channel_head[channel].subs_w_head;
                                while (sub != NULL) {
#ifdef DEBUGSUSPEND
                            printf("lxc!!, here is strange!!, every sub:  sub->location->channel %d, sub->location->chip %d, sub->location->die %d, sub->location->block %d, sub->location->page %d ,sub->current_state %d\n", sub->location->channel, sub->location->chip, sub->location->die, sub->location->block, sub->location->page, sub->current_state );

#endif
                                    if ((sub->current_state == SR_WAIT) && (sub->location->channel == channel) &&
                                        (sub->location->chip == chip) )//&&
                                  //      (sub->location->die == die))      /*该子请求就是当前die的请求*/
                                    {
                                        break;
                                    }
                                    sub = sub->next_node;
                                }
                                if (sub == NULL) {
#ifdef DEBUGSUSPEND
                                    printf("this chip %d, die %d has no available sub-requests,move to next die \n", chip,die);
#endif

                                    continue;
                                }

                                if (sub->current_state == SR_WAIT) {
                                    sub->current_time = ssd->current_time;
                                   //sub->current_state = SR_W_TRANSFER;
                                    //sub->next_state = SR_COMPLETE;
                                   // sub->current_state = SR_W_C_A_DATA_TRANSFER;
                                   // sub->next_state = SR_W_DATA_TRANSFER_ONE_PROG_OF_ITERATIONS;
                                    if ((ssd->parameter->advanced_commands & AD_COPYBACK) == AD_COPYBACK) {
                                        copy_back(ssd, channel, chip, die,
                                                  sub);      /*如果可以执行copyback高级命令，那么就用函数copy_back(ssd, channel,chip, die,sub)处理写子请求*/
#ifdef DEBUGSUSPEND
                                        printf("will never happen now, lxclxclxc\n");

#endif

                                        *change_current_time_flag = 0;
                                    } else {
                            //            static_write(ssd, channel, chip, die, sub);   /*不能执行copyback高级命令，那么就用static_write(ssd, channel,chip, die,sub)函数来处理写子请求*/
#ifdef DEBUGSUSPEND
                                        printf("write begin!!now begin run the go_one_step, sr_w_c_a_data_transfer, now %d channel, %d chip\n",channel, chip);
#endif
                                        go_one_step(ssd, sub, NULL, SR_W_C_A_DATA_TRANSFER, NORMAL);

                                        *change_current_time_flag = 0;
                                    }

                                 //   delete_w_sub_request(ssd, channel, sub);
                                    *channel_busy_flag = 1;
                                    break;
                                }
                      //      }//end of die loop.
                        } else                                                        /*能处理高级命令*/
                        {
                            if (dynamic_advanced_process(ssd, channel, chip) == NULL) {
                                *channel_busy_flag = 0;
                            } else {
                                *channel_busy_flag = 1;                               /*执行了一个请求，传输了数据，占用了总线，需要跳出到下一个channel*/
                                break;
                            }
                        }

                    }
                }
                else{

#ifdef DEBUGSUSPEND
                    printf("in new services_2_write channel %d, chip %d is busy\n", channel, chip);

#endif
                }//end of chip loop.
            }
        }
    }
    return SUCCESS;
}//end of new_services_2_write.

//try to recover by themself here lxcprogram very important!!here.


int try_to_restore_write_function_by_themself(struct ssd_info *ssd, int channel)
{

    unsigned int chip;
    //  unsigned int  Has_read_request = 0;
    struct sub_request *sub_for_read = NULL;
    struct sub_request *sub_for_write = NULL;
    struct sub_request *temp_sub_for_store = NULL;

    struct gc_operation *gc_node = NULL;


//        printf("now begin judge channel %d whether has gc\n", channel);
    if ( ssd->channel_head[channel].gc_command != NULL ){ // this is just one sign for  GC process                                 /*有gc操作，需要进行一定的判断*/

        printf("here channel %d has gc1111\n", channel);



        for (chip = 0; chip < ssd->channel_head[channel].chip; chip ++) {


            sub_for_read = ssd->channel_head[channel].subs_r_head;
            while(sub_for_read != NULL)
            {
                if(sub_for_read->location->chip == chip) //由于是该chip才有的preeptive标志，因此这个标志正确，但是会导致可能好多个write都在这个chip上，那么不就有问题了。
                {
                    break;
                }
                sub_for_read = sub_for_read->next_node ;
            }
            //                if(sub_for_read != NULL){
            //                    Has_read_request = 1;
            //#ifdef DEBUG
            //                    printf("so important, here will never happen... lxclxclxc\n");
            //#endif
            //
            //                }

            if(sub_for_read != NULL){






                //                if( (ssd->channel_head[channel].chip_head[chip].current_state ==  CHIP_WRITE_DATA_CMD_BUSY) || (ssd->channel_head[channel].chip_head[chip].current_state ==  CHIP_WRITE_BUSY) ) {

                if(ssd->channel_head[channel].chip_head[chip].current_state == UNKNOWN){

                    if(ssd->channel_head[channel].chip_head[chip].write_preemptive_sign == 1){

                        printf("try to restore already write suspended channel [%d], chip [%d] .\n", channel, chip);
#ifdef DEBUGSUSPEND
                        printf("try to restore already write suspended channel [%d], chip [%d] .\n", channel, chip);
#endif
                        // actually, here dont have to judge the 
                        sub_for_write = ssd->channel_head[channel].subs_w_head;
                        while(sub_for_write != NULL)
                        {
                            if(sub_for_write->location->chip == chip) //由于是该chip才有的preeptive标志，因此这个标志正确，但是会导致可能好多个write都在这个chip上，那么不就有问题了。
                            {
                                break;
                            }
                            sub_for_write = sub_for_write->next_node ;
                        }



                        printf("stored sub is %lld\n",  ssd->channel_head[channel].chip_head[chip].storedsub);
                        temp_sub_for_store = ssd->channel_head[channel].chip_head[chip].storedsub;
                        printf("restored write chip status: %d, lpn: %d, ppn: %d  operation: %d,size: %d,   ////sub_request status:: current_state: %d, current_time: %llu, next_state: %d, next_state_predict_time: %llu, state(only for write): %d\n", temp_sub_for_store->location->chip, temp_sub_for_store->lpn, temp_sub_for_store->ppn,temp_sub_for_store->operation, temp_sub_for_store->size, temp_sub_for_store->current_state,temp_sub_for_store->current_time,temp_sub_for_store->next_state, temp_sub_for_store->next_state_predict_time, temp_sub_for_store->state) ;
                        if(sub_for_write != temp_sub_for_store){
                            printf("lxclxclxc,so serious wrong!!!, chip %d first write %lld\n", chip, sub_for_write);
                        }
                        //#ifdef DEBUGSUSPEND
                        printf("this chip has no read now, so begin to restore,but have to judgment channel is IDLE and the num_w_cycle is %d.\n",ssd->channel_head[channel].chip_head[chip].num_w_cycle);
                        //#endif
                        //write 恢复的时候，按照原来存储的下个状态，将下一阶段需要消耗的时间存储在chip中，还有要存储的是：下个要开始的阶段。以及下个开始阶段的用时。由于write比较固定，因此只用存下次要开始的次数就行。而gc部分，由于很复杂，所以要存储下一阶段标记以及下阶段的用时。但是你不能设定chip的当前时间！！！！并且write不用存储当前状态，因为write中间都设定成同一个状态了。
                        //so important here!!!restoring without any permission of channel IDLE
                        //if ((ssd->channel_head[channel].current_state == CHANNEL_IDLE) || (ssd->channel_head[channel].next_state == CHANNEL_IDLE && ssd->channel_head[channel].next_state_predict_time <= ssd->current_time)) {

                        //gc_node = ssd->channel_head[channel].gc_command;
                        if(ssd->channel_head[channel].chip_head[chip].num_w_cycle == 0){
#ifdef DEBUGSUSPEND
                            printf("restore num_w_cycle is 0 , wrong happen!! lxclxclxc.\n");
#endif
                        }
#ifdef DEBUGSUSPEND
                        printf("now really can restore .\n");
#endif


                        //here do not have to set sub state since chip state can lead to next write_busy time!!!!
                        ssd->channel_head[channel].chip_head[chip].write_preemptive_sign = 0;
                        ssd->channel_head[channel].chip_head[chip].current_state = CHIP_WRITE_BUSY;// this can avoid the begin of  a new write .
                        ssd->channel_head[channel].chip_head[chip].current_time = ssd->current_time;// this can avoid the begin of  a new write .

                        if( (ssd->channel_head[channel].chip_head[chip].num_w_cycle) % 2 == 1 ){

                            ssd->channel_head[channel].chip_head[chip].next_state_predict_time = ssd->current_time + ssd->parameter->time_characteristics.tPROG_w_progam + 5*ssd->parameter->time_characteristics.tWC ;// this can avoid a new write begin.
                        }else {
                            ssd->channel_head[channel].chip_head[chip].next_state_predict_time = ssd->current_time + ssd->parameter->time_characteristics.tPROG_w_verify + 5*ssd->parameter->time_characteristics.tWC;// this can avoid a new write begin.
                        }
                        // ssd->channel_head[channel].chip_head[chip].current_time = ssd->current_time;// 
                        //                    }else{
                        //#ifdef DEBUGSUSPEND
                        //                        printf("only because channel is not IDLE give up restore this time.\n");
                        //#endif
                        //                    }
                    }
                }
            }
            //}
        }//end of chip loop
    }//end of gc judgement
}

//lxc_program
//shown as the name ,try to restore the write at every one read completed for the write suspension and restore are chip level.
int try_to_restore_write_function(struct ssd_info *ssd){

    unsigned int channel , chip;
    //  unsigned int  Has_read_request = 0;
    struct sub_request *sub_for_read = NULL;
    struct sub_request *sub_for_write = NULL;
    struct sub_request *temp_sub_for_store = NULL;

    struct gc_operation *gc_node = NULL;

    for (channel = 0; channel < ssd->parameter->channel_number; channel++)    {

    //    if ( ssd->channel_head[channel].gc_command == NULL ){ // this is just one sign for  GC process                                 /*有gc操作，需要进行一定的判断*/
        for (chip = 0; chip < ssd->channel_head[channel].chip; chip ++) {

            if(ssd->channel_head[channel].chip_head[chip].write_preemptive_sign == 1){
#ifdef DEBUGSUSPEND
                printf("try to restore already write suspended channel [%d], chip [%d] .\n", channel, chip);
#endif
                // actually, here dont have to judge the 

                sub_for_read = ssd->channel_head[channel].subs_r_head;
                while(sub_for_read != NULL)
                {
                    if(sub_for_read->location->chip == chip) //由于是该chip才有的preeptive标志，因此这个标志正确，但是会导致可能好多个write都在这个chip上，那么不就有问题了。
                    {
                        break;
                    }
                    sub_for_read = sub_for_read->next_node ;
                }
                //                if(sub_for_read != NULL){
                //                    Has_read_request = 1;
                //#ifdef DEBUG
                //                    printf("so important, here will never happen... lxclxclxc\n");
                //#endif
                //
                //                }

                if(sub_for_read == NULL){

                sub_for_write = ssd->channel_head[channel].subs_w_head;
                while(sub_for_write != NULL)
                {
                    if(sub_for_write->location->chip == chip) //由于是该chip才有的preeptive标志，因此这个标志正确，但是会导致可能好多个write都在这个chip上，那么不就有问题了。
                    {
                        break;
                    }
                    sub_for_write = sub_for_write->next_node ;
                }



                    printf("stored sub is %lld\n",  ssd->channel_head[channel].chip_head[chip].storedsub);
                    temp_sub_for_store = ssd->channel_head[channel].chip_head[chip].storedsub;
            printf("restored write chip status: %d, lpn: %d, ppn: %d  operation: %d,size: %d,   ////sub_request status:: current_state: %d, current_time: %llu, next_state: %d, next_state_predict_time: %llu, state(only for write): %d\n", temp_sub_for_store->location->chip, temp_sub_for_store->lpn, temp_sub_for_store->ppn,temp_sub_for_store->operation, temp_sub_for_store->size, temp_sub_for_store->current_state,temp_sub_for_store->current_time,temp_sub_for_store->next_state, temp_sub_for_store->next_state_predict_time, temp_sub_for_store->state) ;
                    if(sub_for_write != temp_sub_for_store){
                        printf("lxclxclxc,so serious wrong!!!, chip %d first write %lld\n", chip, sub_for_write);
                    }
//#ifdef DEBUGSUSPEND
                    printf("this chip has no read now, so begin to restore,but have to judgment channel is IDLE and the num_w_cycle is %d.\n",ssd->channel_head[channel].chip_head[chip].num_w_cycle);
//#endif
                    //write 恢复的时候，按照原来存储的下个状态，将下一阶段需要消耗的时间存储在chip中，还有要存储的是：下个要开始的阶段。以及下个开始阶段的用时。由于write比较固定，因此只用存下次要开始的次数就行。而gc部分，由于很复杂，所以要存储下一阶段标记以及下阶段的用时。但是你不能设定chip的当前时间！！！！并且write不用存储当前状态，因为write中间都设定成同一个状态了。
//so important here!!!restoring without any permission of channel IDLE
                    //if ((ssd->channel_head[channel].current_state == CHANNEL_IDLE) || (ssd->channel_head[channel].next_state == CHANNEL_IDLE && ssd->channel_head[channel].next_state_predict_time <= ssd->current_time)) {

                        //gc_node = ssd->channel_head[channel].gc_command;
                        if(ssd->channel_head[channel].chip_head[chip].num_w_cycle == 0){
#ifdef DEBUGSUSPEND
                            printf("restore num_w_cycle is 0 , wrong happen!! lxclxclxc.\n");
#endif
                        }
#ifdef DEBUGSUSPEND
                        printf("now really can restore .\n");
#endif


//here do not have to set sub state since chip state can lead to next write_busy time!!!!
                        ssd->channel_head[channel].chip_head[chip].write_preemptive_sign = 0;
                        ssd->channel_head[channel].chip_head[chip].current_state = CHIP_WRITE_BUSY;// this can avoid the begin of  a new write .
                        ssd->channel_head[channel].chip_head[chip].current_time = ssd->current_time;// this can avoid the begin of  a new write .

                        if( (ssd->channel_head[channel].chip_head[chip].num_w_cycle) % 2 == 1 ){

                            ssd->channel_head[channel].chip_head[chip].next_state_predict_time = ssd->current_time + ssd->parameter->time_characteristics.tPROG_w_progam + 5*ssd->parameter->time_characteristics.tWC ;// this can avoid a new write begin.
                        }else {
                            ssd->channel_head[channel].chip_head[chip].next_state_predict_time = ssd->current_time + ssd->parameter->time_characteristics.tPROG_w_verify + 5*ssd->parameter->time_characteristics.tWC;// this can avoid a new write begin.
                        }
                        // ssd->channel_head[channel].chip_head[chip].current_time = ssd->current_time;// 
//                    }else{
//#ifdef DEBUGSUSPEND
//                        printf("only because channel is not IDLE give up restore this time.\n");
//#endif
//                    }
                }
            }//end of if, preemptive chip.

        }//end of chip loop
        //}//end of gc judgement
    }//end of channel loop
}






/********************
写子请求的处理函数
*********************/
Status services_2_write(struct ssd_info *ssd, unsigned int channel, unsigned int *channel_busy_flag,
                        unsigned int *change_current_time_flag) {
    int j = 0, chip = 0;
    unsigned int k = 0;
    unsigned int old_ppn = 0, new_ppn = 0;
    unsigned int chip_token = 0, die_token = 0, plane_token = 0, address_ppn = 0;
    unsigned int die = 0, plane = 0;
    long long time = 0;
    struct sub_request *sub = NULL, *p = NULL;
    struct sub_request *sub_twoplane_one = NULL, *sub_twoplane_two = NULL;
    struct sub_request *sub_interleave_one = NULL, *sub_interleave_two = NULL;

    /************************************************************************************************************************
	*写子请求挂在两个地方一个是channel_head[channel].subs_w_head，另外一个是ssd->subs_w_head，所以要保证至少有一个队列不为空
	*同时子请求的处理还分为动态分配和静态分配。
	*************************************************************************************************************************/
#ifdef DEBUG
    printf("enter services_2_write, channel is %d, channel_busy_flag is %d, change_current_time_flag is %d\n",channel, channel_busy_flag, change_current_time_flag);
#endif
    if ((ssd->channel_head[channel].subs_w_head != NULL) || (ssd->subs_w_head != NULL)) {
        if (ssd->parameter->allocation_scheme == 0)                                       /*动态分配*/
        {
            for (j = 0; j < ssd->channel_head[channel].chip; j++) {
                if ((ssd->channel_head[channel].subs_w_head == NULL) && (ssd->subs_w_head == NULL)) {
                    break;
                }

                chip_token = ssd->channel_head[channel].token;                            /*令牌*/
#ifdef DEBUG
    printf("channel %d's chip token is %d\n", channel, chip_token);
#endif


                if (*channel_busy_flag == 0) {
                    if ((ssd->channel_head[channel].chip_head[chip_token].current_state == CHIP_IDLE) ||
                        ((ssd->channel_head[channel].chip_head[chip_token].next_state == CHIP_IDLE) &&
                         (ssd->channel_head[channel].chip_head[chip_token].next_state_predict_time <=
                          ssd->current_time))) {
                        if ((ssd->channel_head[channel].subs_w_head == NULL) && (ssd->subs_w_head == NULL)) {
                            break;
                        }
                        die_token = ssd->channel_head[channel].chip_head[chip_token].token;
#ifdef DEBUG
                        printf("channel %d's chip token is %d,die token is %d\n", channel, chip_token, die_token);
#endif


                        if (((ssd->parameter->advanced_commands & AD_INTERLEAVE) != AD_INTERLEAVE) &&
                            ((ssd->parameter->advanced_commands & AD_TWOPLANE) !=
                             AD_TWOPLANE))       //can't use advanced commands
                        {
#ifdef DEBUG
                            printf("if you have not set advanced command,then only do normal read and write\n");
#endif

                            sub = find_write_sub_request(ssd, channel);
                            if (sub == NULL) {
                                break;
                            }

                            if (sub->current_state == SR_WAIT) {
                                plane_token = ssd->channel_head[channel].chip_head[chip_token].die_head[die_token].token;

                                get_ppn(ssd, channel, chip_token, die_token, plane_token, sub);

                                ssd->channel_head[channel].chip_head[chip_token].die_head[die_token].token =
                                        (ssd->channel_head[channel].chip_head[chip_token].die_head[die_token].token +
                                         1) % ssd->parameter->plane_die;

                                *change_current_time_flag = 0;

                                if (ssd->parameter->ad_priority2 == 0) {
                                    ssd->real_time_subreq--;
                                }
                                go_one_step(ssd, sub, NULL, SR_W_TRANSFER, NORMAL);       /*执行普通的状态的转变。*/
                                delete_w_sub_request(ssd, channel, sub);                /*删掉处理完后的写子请求*/

                                *channel_busy_flag = 1;
                                /**************************************************************************
									*跳出for循环前，修改令牌
									*这里的token的变化完全取决于在这个channel chip die plane下写是否成功
									*成功了就break 没成功token就要变化直到找到能写成功的channel chip die plane
									***************************************************************************/
                                ssd->channel_head[channel].chip_head[chip_token].token =
                                        (ssd->channel_head[channel].chip_head[chip_token].token + 1) %
                                        ssd->parameter->die_chip;
                                ssd->channel_head[channel].token =
                                        (ssd->channel_head[channel].token + 1) % ssd->parameter->chip_channel[channel];
                                break;
                            }
                        } else                                                          /*use advanced commands*/
                        {
                            if (dynamic_advanced_process(ssd, channel, chip_token) == NULL) {
                                *channel_busy_flag = 0;
#ifdef DEBUG
    printf("channel %d's chip token is %d, there is not suit operation in this channel\n", channel, chip_token);
#endif

                            } else {
#ifdef DEBUG
    printf("channel %d's chip token is %d, here get a dynamic_advance_process\n", channel, chip_token);
#endif

                                *channel_busy_flag = 1;                                 /*执行了一个请求，传输了数据，占用了总线，需要跳出到下一个channel*/
                                ssd->channel_head[channel].chip_head[chip_token].token =
                                        (ssd->channel_head[channel].chip_head[chip_token].token + 1) %
                                        ssd->parameter->die_chip;
                                ssd->channel_head[channel].token =
                                        (ssd->channel_head[channel].token + 1) % ssd->parameter->chip_channel[channel];
#ifdef DEBUG
                                printf("channel---token update (only move ahead) channel %d's chip token is %d\n", channel, ssd->channel_head[channel].token);
#endif

                                break;
                            }
                        }

                        ssd->channel_head[channel].chip_head[chip_token].token =
                                (ssd->channel_head[channel].chip_head[chip_token].token + 1) % ssd->parameter->die_chip;
#ifdef DEBUG
                        printf("die---token update (only move head), then new token is channel %d's chip token is %d, die_token is %d\n", channel, chip_token, ssd->channel_head[channel].chip_head[chip_token].token);
#endif

                    }
                }

                ssd->channel_head[channel].token =
                        (ssd->channel_head[channel].token + 1) % ssd->parameter->chip_channel[channel];
            }
        } else if (ssd->parameter->allocation_scheme == 1)                                     /*静态分配*/
        {
#ifdef DEBUG
    printf("static allocation.....now.\n");
#endif

            for (chip = 0; chip < ssd->channel_head[channel].chip; chip++) {
                if ((ssd->channel_head[channel].chip_head[chip].current_state == CHIP_IDLE) ||
                    ((ssd->channel_head[channel].chip_head[chip].next_state == CHIP_IDLE) &&
                     (ssd->channel_head[channel].chip_head[chip].next_state_predict_time <= ssd->current_time))) {
                    if (ssd->channel_head[channel].subs_w_head == NULL) {
                        break;
                    }
                    if (*channel_busy_flag == 0) {

                        if (((ssd->parameter->advanced_commands & AD_INTERLEAVE) != AD_INTERLEAVE) &&
                            ((ssd->parameter->advanced_commands & AD_TWOPLANE) != AD_TWOPLANE))     /*不执行高级命令*/
                        {
#ifdef DEBUG
                            printf("services_2_write tatic operation. Here should be printed for write\n");

#endif
                           // for (die = 0; die < ssd->channel_head[channel].chip_head[chip].die_num; die++) {
                                if (ssd->channel_head[channel].subs_w_head == NULL) {
                                    break;
                                }
                                sub = ssd->channel_head[channel].subs_w_head;
                                while (sub != NULL) {
                                    if ((sub->current_state == SR_WAIT) && (sub->location->channel == channel) &&
                                        (sub->location->chip == chip) &&
                                        (sub->location->die == die))      /*该子请求就是当前die的请求*/
                                    {
                                        break;
                                    }
                                    sub = sub->next_node;
                                }
                                if (sub == NULL) {
                                    continue;
                                }

                                if (sub->current_state == SR_WAIT) {
                                    sub->current_time = ssd->current_time;
                                    sub->current_state = SR_W_TRANSFER;
                                    sub->next_state = SR_COMPLETE;

                                    if ((ssd->parameter->advanced_commands & AD_COPYBACK) == AD_COPYBACK) {
                                        copy_back(ssd, channel, chip, die,
                                                  sub);      /*如果可以执行copyback高级命令，那么就用函数copy_back(ssd, channel,chip, die,sub)处理写子请求*/
                                        *change_current_time_flag = 0;
                                    } else {
                                        static_write(ssd, channel, chip, die,
                                                     sub);   /*不能执行copyback高级命令，那么就用static_write(ssd, channel,chip, die,sub)函数来处理写子请求*/
                                        *change_current_time_flag = 0;
                                    }

                                    delete_w_sub_request(ssd, channel, sub);
                                    *channel_busy_flag = 1;
                                    break;
                                }
                     //       }//end of die loop
                        } else                                                        /*不能处理高级命令*/
                        {
                            if (dynamic_advanced_process(ssd, channel, chip) == NULL) {
                                *channel_busy_flag = 0;
                            } else {
                                *channel_busy_flag = 1;                               /*执行了一个请求，传输了数据，占用了总线，需要跳出到下一个channel*/
                                break;
                            }
                        }

                    }
                }
            }
        }
    }
    return SUCCESS;
}//end of services_2_write


/********************************************************
*这个函数的主要功能是主控读子请求和写子请求的状态变化处理
*********************************************************/

struct ssd_info *process(struct ssd_info *ssd) {

    /*********************************************************************************************************
     *flag_die表示是否因为die的busy，阻塞了时间前进，-1表示没有，非-1表示有阻塞，
     *flag_die的值表示die号,old ppn记录在copyback之前的物理页号，用于判断copyback是否遵守了奇偶地址的限制；
     *two_plane_bit[8],two_plane_place[8]数组成员表示同一个channel上每个die的请求分配情况；
     *chg_cur_time_flag作为是否需要调整当前时间的标志位，当因为channel处于busy导致请求阻塞时，需要调整当前时间；
     *初始认为需要调整，置为1，当任何一个channel处理了传送命令或者数据时，这个值置为0，表示不需要调整；
     **********************************************************************************************************/
    int old_ppn = -1, flag_die = -1;
    unsigned int i, chan, random_num;
    unsigned int flag = 0, new_write = 0, chg_cur_time_flag = 1, flag2 = 0, flag_gc = 0;
    int64_t time, channel_time = MAX_INT64;
    struct sub_request *sub;
    //lxcprogram_gc   several gc signs.
    unsigned int whether_begin_run_gc = 0; // 0 means don't go into gc()function.
    unsigned int whether_stop_write =0;// 0 means don't stop, otherwise 1 stop.



    if(ssd->debug_sign == 1){
//printf("every process can debug now\n");
//#define DEBUG 
//#define DEBUGQUEUE 
//#define DEBUGSUSPEND
//#define DEBUGTRACE_OUT 
//#define DEBUG_CURR_TIME
        }



#ifdef DEBUG_CURR_TIME
    printf("enter process,  current time:%lld\n",ssd->current_time);
#endif

    /*********************************************************
     *判断是否有读写子请求，如果有那么flag令为0，没有flag就为1
     *当flag为1时，若ssd中有gc操作这时就可以执行gc操作
     **********************************************************/
    for (i = 0; i < ssd->parameter->channel_number; i++) {
        if ((ssd->channel_head[i].subs_r_head == NULL) && (ssd->channel_head[i].subs_w_head == NULL) &&
                (ssd->subs_w_head == NULL)) {
            //lxcprogram_gc
            // flag = 1;
        } else {
            flag = 0;
            break;
        }
    }
#ifdef DEBUG
    printf("flag is %d, *** 1 has no subs, now can do background GC,,0 has subs\n",flag);
#endif
    if (flag == 1) {
        ssd->flag = 1;
        if (ssd->gc_request > 0)                                                            /*SSD中有gc操作的请求*/
        {
            gc(ssd, 0, 1);                                                                  /*这个gc要求所有channel都必须遍历到*/
        }
        return ssd;
    } else {
        ssd->flag = 0;
    }

    time = ssd->current_time;
#ifdef DEBUG 
    printf("begin go into services_2_r_cmd_trans_and_complete\n");
    printfsub_request_status(ssd);
    printf_every_chip_read_subrequest(ssd);
#endif

    services_2_r_cmd_trans_and_complete(
            ssd);                                            /*处理当前状态是SR_R_C_A_TRANSFER或者当前状态是SR_COMPLETE，或者下一状态是SR_COMPLETE并且下一状态预计时间小于当前状态时间*/
#ifdef DEBUG 
    printf("finish services_2_r_cmd_trans_and_complete\n");
    printfsub_request_status(ssd);

    printf_every_chip_read_subrequest(ssd);
#endif
    //lxcprogram 
    //for write intermediate process not the begining of write.since they dont have to occupy the channel
#ifdef DEBUG 
    printf("begin go into services_2_write_busy_in_chip\n");
    printfsub_request_status(ssd);
    printf_every_chip_read_subrequest(ssd);
#endif

    services_2_write_busy_in_chip(ssd);
    unsigned int temp_for_judge;

//    printf("begin go into for loop \n");
//    for (temp_for_judge = 0;temp_for_judge < ssd->parameter->channel_number; temp_for_judge++){                                       /*for the preemptive judgment and service of write operation*/
//        try_to_restore_write_function_by_themself(ssd, temp_for_judge);
//    }


#ifdef DEBUG 
    printf("finish services_2_write_busy_in_chip\n");
    printfsub_request_status(ssd);

    printf_every_chip_read_subrequest(ssd);
#endif


    //for GC intermediate process not the begining of GC.since they don't have to occupy the channel.
#ifdef DEBUG 
    printf("begin go into services_2_gc_busy_in_chip\n");
    printfsub_request_status(ssd);
    printf_every_chip_read_subrequest(ssd);
#endif


    services_2_gc_busy_in_chip(ssd);


#ifdef DEBUG 
    printf("finish services_2_gc_busy_in_chip\n");
    printfsub_request_status(ssd);

    printf_every_chip_read_subrequest(ssd);
#endif



    random_num =
        ssd->program_count % ssd->parameter->channel_number;                        /*产生一个随机数，保证每次从不同的channel开始查询*/

    /*****************************************
     *循环处理所有channel上的读写子请求
     *发读请求命令，传读写数据，都需要占用总线，
     ******************************************/
    for (chan = 0; chan < ssd->parameter->channel_number; chan++) {
        i = (random_num + chan) % ssd->parameter->channel_number;
        flag = 0;
        flag_gc = 0;                                                                       /*每次进入channel时，将gc的标志位置为0，默认认为没有进行gc操作*/
//printf("loop for every channel is %d\n", i);
//if(ssd->channel_head[i].current_time > 1875915859632){
//if(ssd->channel_head[i].current_time > 1895915859632){
 //       printf("ssd currenttime is %lld,  channel value:  ssd->channel_head[%d].current_state is %d,or ssd->channel_head[%d].next_state is %d and next_state_predict_time %lld, channel current_time %lld \n",ssd->current_time, i,ssd->channel_head[i].current_state, i, ssd->channel_head[i].next_state,ssd->channel_head[i].current_time, ssd->channel_head[i].next_state_predict_time );
//    unsigned int j;

//if(i == 5){
//    printf("ssd current_time is %lld\n", ssd->current_time);
//        printf("channel[%d].next_state is %d,ssd->channel_head[%d].next_state_predict_time is %lld, channel[%d].current_state is %d,ssd->channel_head[%d].current_time is %lld\n",i,ssd->channel_head[i].next_state,i,ssd->channel_head[i].next_state_predict_time  ,i,ssd->channel_head[i].current_state,i,ssd->channel_head[i].current_time  );
//
//        for (j = 0; j < ssd->parameter->chip_channel[i]; j++) {
//            printf("ssd->channel_head[%d].chip_head[%d].next_state_predict_time is %lld,  ssd->channel_head[%d].chip_head[%d].next_state is %d  ssd->channel_head[%d].chip_head[%d].current_time is %lld,  ssd->channel_head[%d].chip_head[%d].current_state is %d\n", i,j,ssd->channel_head[i].chip_head[j].next_state_predict_time ,i,j,ssd->channel_head[i].chip_head[j].next_state , i,j,ssd->channel_head[i].chip_head[j].current_time ,i,j,ssd->channel_head[i].chip_head[j].current_state  );
//        }
//
//
//printf("ssd->gc_request value is %d,  ssd->channel_head[i].gc_command %d\n ", ssd->gc_request, ssd->channel_head[i].gc_command);
//
//}
//}
#ifdef DEBUG
        printf("enter the process for channel %d circulation\n", i);
#endif
        if ((ssd->channel_head[i].current_state == CHANNEL_IDLE) || (ssd->channel_head[i].next_state == CHANNEL_IDLE &&
                    ssd->channel_head[i].next_state_predict_time <=
                    ssd->current_time)) {
#ifdef DEBUG
            printf("only deal the value 0!! here is the value:  ssd->channel_head[%d].current_state is %d,or ssd->channel_head[%d].next_state is %d and next_state_predict_time<ssd->current_time \n",i,ssd->channel_head[i].current_state, i, ssd->channel_head[i].next_state);
#endif
            //lxcprogram_gc   here should move gc part behind the read operation, for the read has the priority over the environment.
            //            if (ssd->gc_request > 0)                                                       /*有gc操作，需要进行一定的判断*/
            //            {
            //#ifdef DEBUG
            //                printf("has gc now\n");
            //#endif
            //                //lxcprogram_gc
            //
            //                if( (ssd->channel_head[i].gc_command != NULL) && (ssd->channel_head[i].erase_sign ==0) ) {
            //                    flag_gc = gc(ssd, i,
            //                            0);                                                 /*gc函数返回一个值，表示是否执行了gc操作，如果执行了gc操作，这个channel在这个时刻不能服务其他的请求*/
            //                }
            //                if (flag_gc == 1)                                                          /*执行过gc操作，需要跳出此次循环*/
            //                {
            //                    continue;
            //                }
            //            }
            //
            sub = ssd->channel_head[i].subs_r_head;    //这个好像没有用。。。 是用于每次通道变换的时候。                                   /*先处理读请求*/
            //lxcprogram_gc  just judge is there any read request queued in the on-going gc chip.
               
            if ( ssd->channel_head[i].gc_command != NULL ){ // this is just one sign for  GC process                                 /*有gc操作，需要进行一定的判断*/
            whether_begin_run_gc = 1;
            }

//            whether_begin_run_gc = judging_read_in_gc_chip(ssd, i);//0 means gcing chip has read requests,then don't go into gc.1 means there is no requests, need to go into gc.

//if(ssd->channel_head[i].gc_command == NULL){

            printf("begin go into services_2_r_wait, flag is %d, flag_gc is %d\n",flag,flag_gc);
#ifdef DEBUG 
            printf("begin go into services_2_r_wait, flag is %d, flag_gc is %d\n",flag,flag_gc);
            printfsub_request_status(ssd);
            printf_every_chip_read_subrequest(ssd);
#endif
            services_2_r_wait(ssd, i, &flag, &chg_cur_time_flag);                           /*处理处于等待状态的读子请求*/
#ifdef DEBUG 
            printf("finish services_2_r_wait, flag is %d, flag_gc is %d\n",flag,flag_gc);
            printfsub_request_status(ssd);
            printf_every_chip_read_subrequest(ssd);
#endif

//}else{//so important, if has write suspension,but cannot been save by read, then recover by write themself here.
//
//
//// try_to_restore_write_function_by_themself(ssd, i);
//}
//if(whether_begin_run_gc != 1){

            if ((flag == 0) && (ssd->channel_head[i].subs_r_head !=
                        NULL))                      /*if there are no new read request and data is ready in some dies, send these data to controller and response this request*/
            {
#ifdef DEBUG 
                printf("begin go into services_2_r_date_trans, flag is %d, flag_gc is %d\n",flag,flag_gc);
                printfsub_request_status(ssd);
                printf_every_chip_read_subrequest(ssd);
#endif

                services_2_r_data_trans(ssd, i, &flag, &chg_cur_time_flag);
#ifdef DEBUG   
                printf("finish services_2_r_data_trans, flag is %d, flag_gc is %d\n",flag,flag_gc);
                printfsub_request_status(ssd);
                printf_every_chip_read_subrequest(ssd);
#endif

            }

//}


//here is for gc

            //lxcprogram_gc  the gc is moved here. only when the GCing chip has incoming read requests again.

            if(whether_begin_run_gc == 1 ){
#ifdef DEBUG 
                printf("now has GC, flag is %d, flag_gc is %d\n",flag,flag_gc);
                printfsub_request_status(ssd);
                printf_every_chip_read_subrequest(ssd);
#endif

//                printf("now has GC, flag is %d, flag_gc is %d\n",flag,flag_gc);

                gc(ssd, i, 0);          
#ifdef DEBUG   
                printf("finish GC, flag is %d, flag_gc is %d\n",flag,flag_gc);
                printfsub_request_status(ssd);
                printf_every_chip_read_subrequest(ssd);
#endif

            }




//here is for write judgement.            
            if ( ssd->channel_head[i].gc_command != NULL ){ // this means every time should skip write during GC process                                 /*有gc操作，需要进行一定的判断*/
            
                whether_stop_write = 1;
            }else {

                whether_stop_write = 0;
            }
#ifdef DEBUG
            printf("whether_begin_run_gc is %d,  whether_stop_write is %d\n",whether_begin_run_gc, whether_stop_write );
#endif



//here is for write process.
            if(whether_stop_write == 0){ 
                //lxcprogram_gc
                if ( (flag == 0) && (ssd->channel_head[i].gc_command == NULL) ) //在这也不是说一定无read，而是判定当有空闲还是支持write见缝插针的，只是gc就没法见缝插针了。除了gc产生的时候，一定不执行，当仅仅被read suspend之后，就可以见缝插针了。这样效率高。 no read,并且无gc的时候(无gc是指当前通道无gc，不是整个无gc)，执行write.                                                                /*if there are no read request to take channel, we can serve write requests*/
                {
#ifdef DEBUG 
                    printf("begin go into services_2_write, flag=0,flag_gc is %d\n",flag_gc);
                    printfsub_request_status(ssd);
                    printf_every_chip_read_subrequest(ssd);
#endif
                    //lxcprogram
                    //if(program_preemptive == 1 ){
                    //read here
                    // } else{

                    //services_2_write(ssd, i, &flag, &chg_cur_time_flag);
#ifdef DEBUG 
                    printf("before go into services_2_write,channel is %d\n",i);
#endif 
                    new_services_2_write(ssd, i, &flag, &chg_cur_time_flag);

                    //services_2_write_busy_in_chip(ssd);
                    //}
#ifdef DEBUG 
                    printf("finish services_2_write, flag is %d, flag_gc is %d\n",flag, flag_gc);
                    printfsub_request_status(ssd);
                    printf_every_chip_read_subrequest(ssd);
#endif

                }
            }
        }
    }//end of channel loop

    return ssd;
}

/****************************************************************************************************************************
*当ssd支持高级命令时，这个函数的作用就是处理高级命令的写子请求
*根据请求的个数，决定选择哪种高级命令（这个函数只处理写请求，读请求已经分配到每个channel，所以在执行时之间进行选取相应的命令）
*****************************************************************************************************************************/
struct ssd_info *dynamic_advanced_process(struct ssd_info *ssd, unsigned int channel, unsigned int chip) {
    unsigned int die = 0, plane = 0;
    unsigned int subs_count = 0;
    int flag;
    unsigned int gate;                                                                    /*record the max subrequest that can be executed in the same channel. it will be used when channel-level priority order is highest and allocation scheme is full dynamic allocation*/
    unsigned int plane_place;                                                             /*record which plane has sub request in static allocation*/
    struct sub_request *sub = NULL, *p = NULL, *sub0 = NULL, *sub1 = NULL, *sub2 = NULL, *sub3 = NULL, *sub0_rw = NULL, *sub1_rw = NULL, *sub2_rw = NULL, *sub3_rw = NULL;
    struct sub_request **subs = NULL;
    unsigned int max_sub_num = 0;
    unsigned int die_token = 0, plane_token = 0;
    unsigned int *plane_bits = NULL;
    unsigned int interleaver_count = 0;

    unsigned int mask = 0x00000001;
    unsigned int i = 0, j = 0;
#ifdef DEBUG
    printf("enter dynamic_advanced_process\n");
#endif

    max_sub_num = (ssd->parameter->die_chip) * (ssd->parameter->plane_die);
    gate = max_sub_num;
    subs = (struct sub_request **) malloc(max_sub_num * sizeof(struct sub_request *));
    alloc_assert(subs, "sub_request");

    for (i = 0; i < max_sub_num; i++) {
        subs[i] = NULL;
    }

    if ((ssd->parameter->allocation_scheme == 0) && (ssd->parameter->dynamic_allocation == 0) &&
        (ssd->parameter->ad_priority2 == 0)) {
        gate = ssd->real_time_subreq / ssd->parameter->channel_number;

        if (gate == 0) {
            gate = 1;
        } else {
            if (ssd->real_time_subreq % ssd->parameter->channel_number != 0) {
                gate++;
            }
        }
    }

    if ((ssd->parameter->allocation_scheme ==
         0))                                           /*全动态分配，需要从ssd->subs_w_head上选取等待服务的子请求*/
    {
        if (ssd->parameter->dynamic_allocation == 0) {
            sub = ssd->subs_w_head;
        } else {
            sub = ssd->channel_head[channel].subs_w_head;
        }

        subs_count = 0;

        while ((sub != NULL) && (subs_count < max_sub_num) && (subs_count < gate)) {
            if (sub->current_state == SR_WAIT) {
                if ((sub->update == NULL) || ((sub->update != NULL) && ((sub->update->current_state == SR_COMPLETE) ||
                                                                        ((sub->update->next_state == SR_COMPLETE) &&
                                                                         (sub->update->next_state_predict_time <=
                                                                          ssd->current_time)))))    //没有需要提前读出的页
                {
                    subs[subs_count] = sub;
                    subs_count++;
                }
            }

            p = sub;
            sub = sub->next_node;
        }

        if (subs_count == 0)                                                               /*没有请求可以服务，返回NULL*/
        {
            for (i = 0; i < max_sub_num; i++) {
                subs[i] = NULL;
            }
            free(subs);

            subs = NULL;
            free(plane_bits);
            return NULL;
        }
        if (subs_count >= 2) {
            /*********************************************
			*two plane,interleave都可以使用
			*在这个channel上，选用interleave_two_plane执行
			**********************************************/
            if (((ssd->parameter->advanced_commands & AD_TWOPLANE) == AD_TWOPLANE) &&
                ((ssd->parameter->advanced_commands & AD_INTERLEAVE) == AD_INTERLEAVE)) {
                get_ppn_for_advanced_commands(ssd, channel, chip, subs, subs_count, INTERLEAVE_TWO_PLANE);
            } else if (((ssd->parameter->advanced_commands & AD_TWOPLANE) == AD_TWOPLANE) &&
                       ((ssd->parameter->advanced_commands & AD_INTERLEAVE) != AD_INTERLEAVE)) {
                if (subs_count > ssd->parameter->plane_die) {
                    for (i = ssd->parameter->plane_die; i < subs_count; i++) {
                        subs[i] = NULL;
                    }
                    subs_count = ssd->parameter->plane_die;
                }
                get_ppn_for_advanced_commands(ssd, channel, chip, subs, subs_count, TWO_PLANE);
            } else if (((ssd->parameter->advanced_commands & AD_TWOPLANE) != AD_TWOPLANE) &&
                       ((ssd->parameter->advanced_commands & AD_INTERLEAVE) == AD_INTERLEAVE)) {

                if (subs_count > ssd->parameter->die_chip) {
                    for (i = ssd->parameter->die_chip; i < subs_count; i++) {
                        subs[i] = NULL;
                    }
                    subs_count = ssd->parameter->die_chip;
                }
                get_ppn_for_advanced_commands(ssd, channel, chip, subs, subs_count, INTERLEAVE);
            } else {
                for (i = 1; i < subs_count; i++) {
                    subs[i] = NULL;
                }
                subs_count = 1;
                get_ppn_for_normal_command(ssd, channel, chip, subs[0]);
            }

        }//if(subs_count>=2)
        else if (subs_count == 1)     //only one request
        {
            get_ppn_for_normal_command(ssd, channel, chip, subs[0]);
        }

    }//if ((ssd->parameter->allocation_scheme==0))
    else                                                                                  /*静态分配方式，只需从这个特定的channel上选取等待服务的子请求*/
    {
        /*在静态分配方式中，根据channel上的请求落在同一个die上的那些plane来确定使用什么命令*/

        sub = ssd->channel_head[channel].subs_w_head;
        plane_bits = (unsigned int *) malloc((ssd->parameter->die_chip) * sizeof(unsigned int));
        alloc_assert(plane_bits, "plane_bits");
        memset(plane_bits, 0, (ssd->parameter->die_chip) * sizeof(unsigned int));

        for (i = 0; i < ssd->parameter->die_chip; i++) {
            plane_bits[i] = 0x00000000;
        }
        subs_count = 0;

        while ((sub != NULL) && (subs_count < max_sub_num)) {
            if (sub->current_state == SR_WAIT) {
                if ((sub->update == NULL) || ((sub->update != NULL) && ((sub->update->current_state == SR_COMPLETE) ||
                                                                        ((sub->update->next_state == SR_COMPLETE) &&
                                                                         (sub->update->next_state_predict_time <=
                                                                          ssd->current_time))))) {
                    if (sub->location->chip == chip) {
                        plane_place = 0x00000001 << (sub->location->plane);

                        if ((plane_bits[sub->location->die] & plane_place) !=
                            plane_place)      //we have not add sub request to this plane
                        {
                            subs[sub->location->die * ssd->parameter->plane_die + sub->location->plane] = sub;
                            subs_count++;
                            plane_bits[sub->location->die] = (plane_bits[sub->location->die] | plane_place);
                        }
                    }
                }
            }
            sub = sub->next_node;
        }//while ((sub!=NULL)&&(subs_count<max_sub_num))

        if (subs_count == 0)                                                            /*没有请求可以服务，返回NULL*/
        {
            for (i = 0; i < max_sub_num; i++) {
                subs[i] = NULL;
            }
            free(subs);
            subs = NULL;
            free(plane_bits);
            return NULL;
        }

        flag = 0;
        if (ssd->parameter->advanced_commands != 0) {
            if ((ssd->parameter->advanced_commands & AD_COPYBACK) == AD_COPYBACK)        /*全部高级命令都可以使用*/
            {
                if (subs_count > 1)                                                    /*有1个以上可以直接服务的写请求*/
                {
                    get_ppn_for_advanced_commands(ssd, channel, chip, subs, subs_count, COPY_BACK);
                } else {
                    for (i = 0; i < max_sub_num; i++) {
                        if (subs[i] != NULL) {
                            break;
                        }
                    }
                    get_ppn_for_normal_command(ssd, channel, chip, subs[i]);
                }

            }// if ((ssd->parameter->advanced_commands&AD_COPYBACK)==AD_COPYBACK)
            else                                                                     /*不能执行copyback*/
            {
                if (subs_count > 1)                                                    /*有1个以上可以直接服务的写请求*/
                {
                    if (((ssd->parameter->advanced_commands & AD_INTERLEAVE) == AD_INTERLEAVE) &&
                        ((ssd->parameter->advanced_commands & AD_TWOPLANE) == AD_TWOPLANE)) {
                        get_ppn_for_advanced_commands(ssd, channel, chip, subs, subs_count, INTERLEAVE_TWO_PLANE);
                    } else if (((ssd->parameter->advanced_commands & AD_INTERLEAVE) == AD_INTERLEAVE) &&
                               ((ssd->parameter->advanced_commands & AD_TWOPLANE) != AD_TWOPLANE)) {
                        for (die = 0; die < ssd->parameter->die_chip; die++) {
                            if (plane_bits[die] != 0x00000000) {
                                for (i = 0; i < ssd->parameter->plane_die; i++) {
                                    plane_token = ssd->channel_head[channel].chip_head[chip].die_head[die].token;
                                    ssd->channel_head[channel].chip_head[chip].die_head[die].token =
                                            (plane_token + 1) % ssd->parameter->plane_die;
                                    mask = 0x00000001 << plane_token;
                                    if ((plane_bits[die] & mask) == mask) {
                                        plane_bits[die] = mask;
                                        break;
                                    }
                                }
                                for (i = i + 1; i < ssd->parameter->plane_die; i++) {
                                    plane = (plane_token + 1) % ssd->parameter->plane_die;
                                    subs[die * ssd->parameter->plane_die + plane] = NULL;
                                    subs_count--;
                                }
                                interleaver_count++;
                            }//if(plane_bits[die]!=0x00000000)
                        }//for(die=0;die<ssd->parameter->die_chip;die++)
                        if (interleaver_count >= 2) {
                            get_ppn_for_advanced_commands(ssd, channel, chip, subs, subs_count, INTERLEAVE);
                        } else {
                            for (i = 0; i < max_sub_num; i++) {
                                if (subs[i] != NULL) {
                                    break;
                                }
                            }
                            get_ppn_for_normal_command(ssd, channel, chip, subs[i]);
                        }
                    }//else if (((ssd->parameter->advanced_commands&AD_INTERLEAVE)==AD_INTERLEAVE)&&((ssd->parameter->advanced_commands&AD_TWOPLANE)!=AD_TWOPLANE))
                    else if (((ssd->parameter->advanced_commands & AD_INTERLEAVE) != AD_INTERLEAVE) &&
                             ((ssd->parameter->advanced_commands & AD_TWOPLANE) == AD_TWOPLANE)) {
                        for (i = 0; i < ssd->parameter->die_chip; i++) {
                            die_token = ssd->channel_head[channel].chip_head[chip].token;
                            ssd->channel_head[channel].chip_head[chip].token =
                                    (die_token + 1) % ssd->parameter->die_chip;
                            if (size(plane_bits[die_token]) > 1) {
                                break;
                            }

                        }

                        if (i < ssd->parameter->die_chip) {
                            for (die = 0; die < ssd->parameter->die_chip; die++) {
                                if (die != die_token) {
                                    for (plane = 0; plane < ssd->parameter->plane_die; plane++) {
                                        if (subs[die * ssd->parameter->plane_die + plane] != NULL) {
                                            subs[die * ssd->parameter->plane_die + plane] = NULL;
                                            subs_count--;
                                        }
                                    }
                                }
                            }
                            get_ppn_for_advanced_commands(ssd, channel, chip, subs, subs_count, TWO_PLANE);
                        }//if(i<ssd->parameter->die_chip)
                        else {
                            for (i = 0; i < ssd->parameter->die_chip; i++) {
                                die_token = ssd->channel_head[channel].chip_head[chip].token;
                                ssd->channel_head[channel].chip_head[chip].token =
                                        (die_token + 1) % ssd->parameter->die_chip;
                                if (plane_bits[die_token] != 0x00000000) {
                                    for (j = 0; j < ssd->parameter->plane_die; j++) {
                                        plane_token = ssd->channel_head[channel].chip_head[chip].die_head[die_token].token;
                                        ssd->channel_head[channel].chip_head[chip].die_head[die_token].token =
                                                (plane_token + 1) % ssd->parameter->plane_die;
                                        if (((plane_bits[die_token]) & (0x00000001 << plane_token)) != 0x00000000) {
                                            sub = subs[die_token * ssd->parameter->plane_die + plane_token];
                                            break;
                                        }
                                    }
                                }
                            }//for(i=0;i<ssd->parameter->die_chip;i++)
                            get_ppn_for_normal_command(ssd, channel, chip, sub);
                        }//else
                    }//else if (((ssd->parameter->advanced_commands&AD_INTERLEAVE)!=AD_INTERLEAVE)&&((ssd->parameter->advanced_commands&AD_TWOPLANE)==AD_TWOPLANE))
                }//if (subs_count>1)
                else {
                    for (i = 0; i < ssd->parameter->die_chip; i++) {
                        die_token = ssd->channel_head[channel].chip_head[chip].token;
                        ssd->channel_head[channel].chip_head[chip].token = (die_token + 1) % ssd->parameter->die_chip;
                        if (plane_bits[die_token] != 0x00000000) {
                            for (j = 0; j < ssd->parameter->plane_die; j++) {
                                plane_token = ssd->channel_head[channel].chip_head[chip].die_head[die_token].token;
                                ssd->channel_head[channel].chip_head[chip].die_head[die_token].token =
                                        (plane_token + 1) % ssd->parameter->plane_die;
                                if (((plane_bits[die_token]) & (0x00000001 << plane_token)) != 0x00000000) {
                                    sub = subs[die_token * ssd->parameter->plane_die + plane_token];
                                    break;
                                }
                            }
                            if (sub != NULL) {
                                break;
                            }
                        }
                    }//for(i=0;i<ssd->parameter->die_chip;i++)
                    get_ppn_for_normal_command(ssd, channel, chip, sub);
                }//else
            }
        }//if (ssd->parameter->advanced_commands!=0)
        else {
            for (i = 0; i < ssd->parameter->die_chip; i++) {
                die_token = ssd->channel_head[channel].chip_head[chip].token;
                ssd->channel_head[channel].chip_head[chip].token = (die_token + 1) % ssd->parameter->die_chip;
                if (plane_bits[die_token] != 0x00000000) {
                    for (j = 0; j < ssd->parameter->plane_die; j++) {
                        plane_token = ssd->channel_head[channel].chip_head[chip].die_head[die_token].token;
                        ssd->channel_head[channel].chip_head[chip].die_head[die].token =
                                (plane_token + 1) % ssd->parameter->plane_die;
                        if (((plane_bits[die_token]) & (0x00000001 << plane_token)) != 0x00000000) {
                            sub = subs[die_token * ssd->parameter->plane_die + plane_token];
                            break;
                        }
                    }
                    if (sub != NULL) {
                        break;
                    }
                }
            }//for(i=0;i<ssd->parameter->die_chip;i++)
            get_ppn_for_normal_command(ssd, channel, chip, sub);
        }//else

    }//else

    for (i = 0; i < max_sub_num; i++) {
        subs[i] = NULL;
    }
    free(subs);
    subs = NULL;
    free(plane_bits);
    return ssd;
}

/****************************************
*执行写子请求时，为普通的写子请求获取ppn
*****************************************/
Status
get_ppn_for_normal_command(struct ssd_info *ssd, unsigned int channel, unsigned int chip, struct sub_request *sub) {
    unsigned int die = 0;
    unsigned int plane = 0;
#ifdef DEBUG
    printf("enter into get_ppn_for_normal_command\n");
#endif

    if (sub == NULL) {
        return ERROR;
    }

    if (ssd->parameter->allocation_scheme == DYNAMIC_ALLOCATION) {
        die = ssd->channel_head[channel].chip_head[chip].token;
        plane = ssd->channel_head[channel].chip_head[chip].die_head[die].token;
        get_ppn(ssd, channel, chip, die, plane, sub);
        ssd->channel_head[channel].chip_head[chip].die_head[die].token = (plane + 1) % ssd->parameter->plane_die;
        ssd->channel_head[channel].chip_head[chip].token = (die + 1) % ssd->parameter->die_chip;

        compute_serve_time(ssd, channel, chip, die, &sub, 1, NORMAL);
        return SUCCESS;
    } else {
        die = sub->location->die;
        plane = sub->location->plane;
        get_ppn(ssd, channel, chip, die, plane, sub);
        compute_serve_time(ssd, channel, chip, die, &sub, 1, NORMAL);
        return SUCCESS;
    }

}


/************************************************************************************************
*为高级命令获取ppn
*根据不同的命令，遵从在同一个block中顺序写的要求，选取可以进行写操作的ppn，跳过的ppn全部置为失效。
*在使用two plane操作时，为了寻找相同水平位置的页，可能需要直接找到两个完全空白的块，这个时候原来
*的块没有用完，只能放在这，等待下次使用，同时修改查找空白page的方法，将以前首先寻找free块改为，只
*要invalid block!=64即可。
*except find aim page, we should modify token and decide gc operation
*************************************************************************************************/
Status
get_ppn_for_advanced_commands(struct ssd_info *ssd, unsigned int channel, unsigned int chip, struct sub_request **subs,
                              unsigned int subs_count, unsigned int command) {
    unsigned int die = 0, plane = 0;
    unsigned int die_token = 0, plane_token = 0;
    struct sub_request *sub = NULL;
    unsigned int i = 0, j = 0, k = 0;
    unsigned int unvalid_subs_count = 0;
    unsigned int valid_subs_count = 0;
    unsigned int interleave_flag = FALSE;
    unsigned int multi_plane_falg = FALSE;
    unsigned int max_subs_num = 0;
    struct sub_request *first_sub_in_chip = NULL;
    struct sub_request *first_sub_in_die = NULL;
    struct sub_request *second_sub_in_die = NULL;
    unsigned int state = SUCCESS;
    unsigned int multi_plane_flag = FALSE;

    max_subs_num = ssd->parameter->die_chip * ssd->parameter->plane_die;

#ifdef DEBUG
    printf("enter get_ppn_for_advanced_commands\n");
#endif

    if (ssd->parameter->allocation_scheme == DYNAMIC_ALLOCATION)                         /*动态分配操作*/
    {
        if ((command == INTERLEAVE_TWO_PLANE) ||
            (command == COPY_BACK))                      /*INTERLEAVE_TWO_PLANE以及COPY_BACK的情况*/
        {
            for (i = 0; i < subs_count; i++) {
                die = ssd->channel_head[channel].chip_head[chip].token;
                if (i <
                    ssd->parameter->die_chip)                                         /*为每个subs[i]获取ppn，i小于die_chip*/
                {
                    plane = ssd->channel_head[channel].chip_head[chip].die_head[die].token;
                    get_ppn(ssd, channel, chip, die, plane, subs[i]);
                    ssd->channel_head[channel].chip_head[chip].die_head[die].token =
                            (plane + 1) % ssd->parameter->plane_die;
                } else {
                    /*********************************************************************************************************************************
					*超过die_chip的i所指向的subs[i]与subs[i%ssd->parameter->die_chip]获取相同位置的ppn
					*如果成功的获取了则令multi_plane_flag=TRUE并执行compute_serve_time(ssd,channel,chip,0,subs,valid_subs_count,INTERLEAVE_TWO_PLANE);
					*否则执行compute_serve_time(ssd,channel,chip,0,subs,valid_subs_count,INTERLEAVE);
					***********************************************************************************************************************************/
                    state = make_level_page(ssd, subs[i % ssd->parameter->die_chip], subs[i]);
                    if (state != SUCCESS) {
                        subs[i] = NULL;
                        unvalid_subs_count++;
                    } else {
                        multi_plane_flag = TRUE;
                    }
                }
                ssd->channel_head[channel].chip_head[chip].token = (die + 1) % ssd->parameter->die_chip;
            }
            valid_subs_count = subs_count - unvalid_subs_count;
            ssd->interleave_count++;
            if (multi_plane_flag == TRUE) {
                ssd->inter_mplane_count++;
                compute_serve_time(ssd, channel, chip, 0, subs, valid_subs_count,
                                   INTERLEAVE_TWO_PLANE);/*计算写子请求的处理时间，以写子请求的状态转变*/
            } else {
                compute_serve_time(ssd, channel, chip, 0, subs, valid_subs_count, INTERLEAVE);
            }
            return SUCCESS;
        }//if((command==INTERLEAVE_TWO_PLANE)||(command==COPY_BACK))
        else if (command == INTERLEAVE) {
            /***********************************************************************************************
			*INTERLEAVE高级命令的处理，这个处理比TWO_PLANE高级命令的处理简单
			*因为two_plane的要求是同一个die里面不同plane的同一位置的page，而interleave要求则是不同die里面的。
			************************************************************************************************/
            for (i = 0; (i < subs_count) && (i < ssd->parameter->die_chip); i++) {
                die = ssd->channel_head[channel].chip_head[chip].token;
                plane = ssd->channel_head[channel].chip_head[chip].die_head[die].token;
                get_ppn(ssd, channel, chip, die, plane, subs[i]);
                ssd->channel_head[channel].chip_head[chip].die_head[die].token =
                        (plane + 1) % ssd->parameter->plane_die;
                ssd->channel_head[channel].chip_head[chip].token = (die + 1) % ssd->parameter->die_chip;
                valid_subs_count++;
            }
            ssd->interleave_count++;
            compute_serve_time(ssd, channel, chip, 0, subs, valid_subs_count, INTERLEAVE);
            return SUCCESS;
        }//else if(command==INTERLEAVE)
        else if (command == TWO_PLANE) {
            if (subs_count < 2) {
                return ERROR;
            }
            die = ssd->channel_head[channel].chip_head[chip].token;
            for (j = 0; j < subs_count; j++) {
                if (j == 1) {
                    state = find_level_page(ssd, channel, chip, die, subs[0],
                                            subs[1]);        /*寻找与subs[0]的ppn位置相同的subs[1]，执行TWO_PLANE高级命令*/
                    if (state != SUCCESS) {
                        get_ppn_for_normal_command(ssd, channel, chip, subs[0]);           /*没找到，那么就当普通命令来处理*/
                        return FAILURE;
                    } else {
                        valid_subs_count = 2;
                    }
                } else if (j > 1) {
                    state = make_level_page(ssd, subs[0],
                                            subs[j]);                         /*寻找与subs[0]的ppn位置相同的subs[j]，执行TWO_PLANE高级命令*/
                    if (state != SUCCESS) {
                        for (k = j; k < subs_count; k++) {
                            subs[k] = NULL;
                        }
                        subs_count = j;
                        break;
                    } else {
                        valid_subs_count++;
                    }
                }
            }//for(j=0;j<subs_count;j++)
            ssd->channel_head[channel].chip_head[chip].token = (die + 1) % ssd->parameter->die_chip;
            ssd->m_plane_prog_count++;
            compute_serve_time(ssd, channel, chip, die, subs, valid_subs_count, TWO_PLANE);
            return SUCCESS;
        }//else if(command==TWO_PLANE)
        else {
            return ERROR;
        }
    }//if (ssd->parameter->allocation_scheme==DYNAMIC_ALLOCATION)
    else                                                                              /*静态分配的情况*/
    {
        if ((command == INTERLEAVE_TWO_PLANE) || (command == COPY_BACK)) {
            for (die = 0; die < ssd->parameter->die_chip; die++) {
                first_sub_in_die = NULL;
                for (plane = 0; plane < ssd->parameter->plane_die; plane++) {
                    sub = subs[die * ssd->parameter->plane_die + plane];
                    if (sub != NULL) {
                        if (first_sub_in_die == NULL) {
                            first_sub_in_die = sub;
                            get_ppn(ssd, channel, chip, die, plane, sub);
                        } else {
                            state = make_level_page(ssd, first_sub_in_die, sub);
                            if (state != SUCCESS) {
                                subs[die * ssd->parameter->plane_die + plane] = NULL;
                                subs_count--;
                                sub = NULL;
                            } else {
                                multi_plane_flag = TRUE;
                            }
                        }
                    }
                }
            }
            if (multi_plane_flag == TRUE) {
                ssd->inter_mplane_count++;
                compute_serve_time(ssd, channel, chip, 0, subs, valid_subs_count, INTERLEAVE_TWO_PLANE);
                return SUCCESS;
            } else {
                compute_serve_time(ssd, channel, chip, 0, subs, valid_subs_count, INTERLEAVE);
                return SUCCESS;
            }
        }//if((command==INTERLEAVE_TWO_PLANE)||(command==COPY_BACK))
        else if (command == INTERLEAVE) {
            for (die = 0; die < ssd->parameter->die_chip; die++) {
                first_sub_in_die = NULL;
                for (plane = 0; plane < ssd->parameter->plane_die; plane++) {
                    sub = subs[die * ssd->parameter->plane_die + plane];
                    if (sub != NULL) {
                        if (first_sub_in_die == NULL) {
                            first_sub_in_die = sub;
                            get_ppn(ssd, channel, chip, die, plane, sub);
                            valid_subs_count++;
                        } else {
                            subs[die * ssd->parameter->plane_die + plane] = NULL;
                            subs_count--;
                            sub = NULL;
                        }
                    }
                }
            }
            if (valid_subs_count > 1) {
                ssd->interleave_count++;
            }
            compute_serve_time(ssd, channel, chip, 0, subs, valid_subs_count, INTERLEAVE);
        }//else if(command==INTERLEAVE)
        else if (command == TWO_PLANE) {
            for (die = 0; die < ssd->parameter->die_chip; die++) {
                first_sub_in_die = NULL;
                second_sub_in_die = NULL;
                for (plane = 0; plane < ssd->parameter->plane_die; plane++) {
                    sub = subs[die * ssd->parameter->plane_die + plane];
                    if (sub != NULL) {
                        if (first_sub_in_die == NULL) {
                            first_sub_in_die = sub;
                        } else if (second_sub_in_die == NULL) {
                            second_sub_in_die = sub;
                            state = find_level_page(ssd, channel, chip, die, first_sub_in_die, second_sub_in_die);
                            if (state != SUCCESS) {
                                subs[die * ssd->parameter->plane_die + plane] = NULL;
                                subs_count--;
                                second_sub_in_die = NULL;
                                sub = NULL;
                            } else {
                                valid_subs_count = 2;
                            }
                        } else {
                            state = make_level_page(ssd, first_sub_in_die, sub);
                            if (state != SUCCESS) {
                                subs[die * ssd->parameter->plane_die + plane] = NULL;
                                subs_count--;
                                sub = NULL;
                            } else {
                                valid_subs_count++;
                            }
                        }
                    }//if(sub!=NULL)
                }//for(plane=0;plane<ssd->parameter->plane_die;plane++)
                if (second_sub_in_die != NULL) {
                    multi_plane_flag = TRUE;
                    break;
                }
            }//for(die=0;die<ssd->parameter->die_chip;die++)
            if (multi_plane_flag == TRUE) {
                ssd->m_plane_prog_count++;
                compute_serve_time(ssd, channel, chip, die, subs, valid_subs_count, TWO_PLANE);
                return SUCCESS;
            }//if(multi_plane_flag==TRUE)
            else {
                i = 0;
                sub = NULL;
                while ((sub == NULL) && (i < max_subs_num)) {
                    sub = subs[i];
                    i++;
                }
                if (sub != NULL) {
                    get_ppn_for_normal_command(ssd, channel, chip, sub);
                    return FAILURE;
                } else {
                    return ERROR;
                }
            }//else
        }//else if(command==TWO_PLANE)
        else {
            return ERROR;
        }
    }//elseb 静态分配的情况
}


/***********************************************
*函数的作用是让sub0，sub1的ppn所在的page位置相同
************************************************/
Status make_level_page(struct ssd_info *ssd, struct sub_request *sub0, struct sub_request *sub1) {
    unsigned int i = 0, j = 0, k = 0;
    unsigned int channel = 0, chip = 0, die = 0, plane0 = 0, plane1 = 0, block0 = 0, block1 = 0, page0 = 0, page1 = 0;
    unsigned int active_block0 = 0, active_block1 = 0;
    unsigned int old_plane_token = 0;

    if ((sub0 == NULL) || (sub1 == NULL) || (sub0->location == NULL)) {
        return ERROR;
    }
    channel = sub0->location->channel;
    chip = sub0->location->chip;
    die = sub0->location->die;
    plane0 = sub0->location->plane;
    block0 = sub0->location->block;
    page0 = sub0->location->page;
    old_plane_token = ssd->channel_head[channel].chip_head[chip].die_head[die].token;

    /***********************************************************************************************
	*动态分配的情况下
	*sub1的plane是根据sub0的ssd->channel_head[channel].chip_head[chip].die_head[die].token令牌获取的
	*sub1的channel，chip，die，block，page都和sub0的相同
	************************************************************************************************/
    if (ssd->parameter->allocation_scheme == DYNAMIC_ALLOCATION) {
        old_plane_token = ssd->channel_head[channel].chip_head[chip].die_head[die].token;
        for (i = 0; i < ssd->parameter->plane_die; i++) {
            plane1 = ssd->channel_head[channel].chip_head[chip].die_head[die].token;
            if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane1].add_reg_ppn == -1) {
                find_active_block(ssd, channel, chip, die, plane1);                               /*在plane1中找到活跃块*/
                block1 = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane1].active_block;

                /*********************************************************************************************
				*只有找到的block1与block0相同，才能继续往下寻找相同的page
				*在寻找page时比较简单，直接用last_write_page（上一次写的page）+1就可以了。
				*如果找到的page不相同，那么如果ssd允许贪婪的使用高级命令，这样就可以让小的page 往大的page靠拢
				*********************************************************************************************/
                if (block1 == block0) {
                    page1 = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane1].blk_head[block1].last_write_page +
                            1;
                    if (page1 == page0) {
                        break;
                    } else if (page1 < page0) {
                        if (ssd->parameter->greed_MPW_ad == 1)                                  /*允许贪婪的使用高级命令*/
                        {
                            //make_same_level(ssd,channel,chip,die,plane1,active_block1,page0); /*小的page地址往大的page地址靠*/
                            make_same_level(ssd, channel, chip, die, plane1, block1, page0);
                            break;
                        }
                    }
                }//if(block1==block0)
            }
            ssd->channel_head[channel].chip_head[chip].die_head[die].token = (plane1 + 1) % ssd->parameter->plane_die;
        }//for(i=0;i<ssd->parameter->plane_die;i++)
        if (i < ssd->parameter->plane_die) {
            flash_page_state_modify(ssd, sub1, channel, chip, die, plane1, block1,
                                    page0);          /*这个函数的作用就是更新page1所对应的物理页以及location还有map表*/
            //flash_page_state_modify(ssd,sub1,channel,chip,die,plane1,block1,page1);
            ssd->channel_head[channel].chip_head[chip].die_head[die].token = (plane1 + 1) % ssd->parameter->plane_die;
            return SUCCESS;
        } else {
            ssd->channel_head[channel].chip_head[chip].die_head[die].token = old_plane_token;
            return FAILURE;
        }
    } else                                                                                      /*静态分配的情况*/
    {
        if ((sub1->location == NULL) || (sub1->location->channel != channel) || (sub1->location->chip != chip) ||
            (sub1->location->die != die)) {
            return ERROR;
        }
        plane1 = sub1->location->plane;
        if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane1].add_reg_ppn == -1) {
            find_active_block(ssd, channel, chip, die, plane1);
            block1 = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane1].active_block;
            if (block1 == block0) {
                page1 = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane1].blk_head[block1].last_write_page +
                        1;
                if (page1 > page0) {
                    return FAILURE;
                } else if (page1 < page0) {
                    if (ssd->parameter->greed_MPW_ad == 1) {
                        //make_same_level(ssd,channel,chip,die,plane1,active_block1,page0);    /*小的page地址往大的page地址靠*/
                        make_same_level(ssd, channel, chip, die, plane1, block1, page0);
                        flash_page_state_modify(ssd, sub1, channel, chip, die, plane1, block1, page0);
                        //flash_page_state_modify(ssd,sub1,channel,chip,die,plane1,block1,page1);
                        return SUCCESS;
                    } else {
                        return FAILURE;
                    }
                } else {
                    flash_page_state_modify(ssd, sub1, channel, chip, die, plane1, block1, page0);
                    //flash_page_state_modify(ssd,sub1,channel,chip,die,plane1,block1,page1);
                    return SUCCESS;
                }

            } else {
                return FAILURE;
            }

        } else {
            return ERROR;
        }
    }

}

/******************************************************************************************************
*函数的功能是为two plane命令寻找出两个相同水平位置的页，并且修改统计值，修改页的状态
*注意这个函数与上一个函数make_level_page函数的区别，make_level_page这个函数是让sub1与sub0的page位置相同
*而find_level_page函数的作用是在给定的channel，chip，die中找两个位置相同的subA和subB。
*******************************************************************************************************/
Status find_level_page(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die,
                       struct sub_request *subA, struct sub_request *subB) {
    unsigned int i, planeA, planeB, active_blockA, active_blockB, pageA, pageB, aim_page, old_plane;
    struct gc_operation *gc_node;

    old_plane = ssd->channel_head[channel].chip_head[chip].die_head[die].token;

    /************************************************************
	*在动态分配的情况下
	*planeA赋初值为die的令牌，如果planeA是偶数那么planeB=planeA+1
	*planeA是奇数，那么planeA+1变为偶数，再令planeB=planeA+1
	*************************************************************/
    if (ssd->parameter->allocation_scheme == 0) {
        planeA = ssd->channel_head[channel].chip_head[chip].die_head[die].token;
        if (planeA % 2 == 0) {
            planeB = planeA + 1;
            ssd->channel_head[channel].chip_head[chip].die_head[die].token =
                    (ssd->channel_head[channel].chip_head[chip].die_head[die].token + 2) % ssd->parameter->plane_die;
        } else {
            planeA = (planeA + 1) % ssd->parameter->plane_die;
            planeB = planeA + 1;
            ssd->channel_head[channel].chip_head[chip].die_head[die].token =
                    (ssd->channel_head[channel].chip_head[chip].die_head[die].token + 3) % ssd->parameter->plane_die;
        }
    } else                                                                                     /*静态分配的情况，就直接赋值给planeA和planeB*/
    {
        planeA = subA->location->plane;
        planeB = subB->location->plane;
    }
    find_active_block(ssd, channel, chip, die, planeA);                                          /*寻找active_block*/
    find_active_block(ssd, channel, chip, die, planeB);
    active_blockA = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeA].active_block;
    active_blockB = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeB].active_block;



    /*****************************************************
	*如果active_block相同，那么就在这两个块中找相同的page
	*或者使用贪婪的方法找到两个相同的page
	******************************************************/
    if (active_blockA == active_blockB) {
        pageA = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeA].blk_head[active_blockA].last_write_page +
                1;
        pageB = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeB].blk_head[active_blockB].last_write_page +
                1;
        if (pageA == pageB)                                                                    /*两个可用的页正好在同一个水平位置上*/
        {
            flash_page_state_modify(ssd, subA, channel, chip, die, planeA, active_blockA, pageA);
            flash_page_state_modify(ssd, subB, channel, chip, die, planeB, active_blockB, pageB);
        } else {
            if (ssd->parameter->greed_MPW_ad == 1)                                             /*贪婪地使用高级命令*/
            {
                if (pageA < pageB) {
                    aim_page = pageB;
                    make_same_level(ssd, channel, chip, die, planeA, active_blockA,
                                    aim_page);     /*小的page地址往大的page地址靠*/
                } else {
                    aim_page = pageA;
                    make_same_level(ssd, channel, chip, die, planeB, active_blockB, aim_page);
                }
                flash_page_state_modify(ssd, subA, channel, chip, die, planeA, active_blockA, aim_page);
                flash_page_state_modify(ssd, subB, channel, chip, die, planeB, active_blockB, aim_page);
            } else                                                                             /*不能贪婪的使用高级命令*/
            {
                subA = NULL;
                subB = NULL;
                ssd->channel_head[channel].chip_head[chip].die_head[die].token = old_plane;
                return FAILURE;
            }
        }
    }
        /*********************************
	*如果找到的两个active_block不相同
	**********************************/
    else {
        pageA = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeA].blk_head[active_blockA].last_write_page +
                1;
        pageB = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeB].blk_head[active_blockB].last_write_page +
                1;
        if (pageA < pageB) {
            if (ssd->parameter->greed_MPW_ad == 1)                                             /*贪婪地使用高级命令*/
            {
                /*******************************************************************************
				*在planeA中，与active_blockB相同位置的的block中，与pageB相同位置的page是可用的。
				*也就是palneA中的相应水平位置是可用的，将其最为与planeB中对应的页。
				*那么可也让planeA，active_blockB中的page往pageB靠拢
				********************************************************************************/
                if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeA].blk_head[active_blockB].page_head[pageB].free_state ==
                    PG_SUB) {
                    make_same_level(ssd, channel, chip, die, planeA, active_blockB, pageB);
                    flash_page_state_modify(ssd, subA, channel, chip, die, planeA, active_blockB, pageB);
                    flash_page_state_modify(ssd, subB, channel, chip, die, planeB, active_blockB, pageB);
                }
                    /********************************************************************************
				*在planeA中，与active_blockB相同位置的的block中，与pageB相同位置的page是可用的。
				*那么就要重新寻找block，需要重新找水平位置相同的一对页
				*********************************************************************************/
                else {
                    for (i = 0; i < ssd->parameter->block_plane; i++) {
                        pageA = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeA].blk_head[i].last_write_page +
                                1;
                        pageB = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeB].blk_head[i].last_write_page +
                                1;
                        if ((pageA < ssd->parameter->page_block) && (pageB < ssd->parameter->page_block)) {
                            if (pageA < pageB) {
                                if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeA].blk_head[i].page_head[pageB].free_state ==
                                    PG_SUB) {
                                    aim_page = pageB;
                                    make_same_level(ssd, channel, chip, die, planeA, i, aim_page);
                                    break;
                                }
                            } else {
                                if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeB].blk_head[i].page_head[pageA].free_state ==
                                    PG_SUB) {
                                    aim_page = pageA;
                                    make_same_level(ssd, channel, chip, die, planeB, i, aim_page);
                                    break;
                                }
                            }
                        }
                    }//for (i=0;i<ssd->parameter->block_plane;i++)
                    if (i < ssd->parameter->block_plane) {
                        flash_page_state_modify(ssd, subA, channel, chip, die, planeA, i, aim_page);
                        flash_page_state_modify(ssd, subB, channel, chip, die, planeB, i, aim_page);
                    } else {
                        subA = NULL;
                        subB = NULL;
                        ssd->channel_head[channel].chip_head[chip].die_head[die].token = old_plane;
                        return FAILURE;
                    }
                }
            }//if (ssd->parameter->greed_MPW_ad==1)
            else {
                subA = NULL;
                subB = NULL;
                ssd->channel_head[channel].chip_head[chip].die_head[die].token = old_plane;
                return FAILURE;
            }
        }//if (pageA<pageB)
        else {
            if (ssd->parameter->greed_MPW_ad == 1) {
                if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeB].blk_head[active_blockA].page_head[pageA].free_state ==
                    PG_SUB) {
                    make_same_level(ssd, channel, chip, die, planeB, active_blockA, pageA);
                    flash_page_state_modify(ssd, subA, channel, chip, die, planeA, active_blockA, pageA);
                    flash_page_state_modify(ssd, subB, channel, chip, die, planeB, active_blockA, pageA);
                } else {
                    for (i = 0; i < ssd->parameter->block_plane; i++) {
                        pageA = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeA].blk_head[i].last_write_page +
                                1;
                        pageB = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeB].blk_head[i].last_write_page +
                                1;
                        if ((pageA < ssd->parameter->page_block) && (pageB < ssd->parameter->page_block)) {
                            if (pageA < pageB) {
                                if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeA].blk_head[i].page_head[pageB].free_state ==
                                    PG_SUB) {
                                    aim_page = pageB;
                                    make_same_level(ssd, channel, chip, die, planeA, i, aim_page);
                                    break;
                                }
                            } else {
                                if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeB].blk_head[i].page_head[pageA].free_state ==
                                    PG_SUB) {
                                    aim_page = pageA;
                                    make_same_level(ssd, channel, chip, die, planeB, i, aim_page);
                                    break;
                                }
                            }
                        }
                    }//for (i=0;i<ssd->parameter->block_plane;i++)
                    if (i < ssd->parameter->block_plane) {
                        flash_page_state_modify(ssd, subA, channel, chip, die, planeA, i, aim_page);
                        flash_page_state_modify(ssd, subB, channel, chip, die, planeB, i, aim_page);
                    } else {
                        subA = NULL;
                        subB = NULL;
                        ssd->channel_head[channel].chip_head[chip].die_head[die].token = old_plane;
                        return FAILURE;
                    }
                }
            } //if (ssd->parameter->greed_MPW_ad==1)
            else {
                if ((pageA == pageB) && (pageA == 0)) {
                    /*******************************************************************************************
					*下面是两种情况
					*1，planeA，planeB中的active_blockA，pageA位置都可用，那么不同plane 的相同位置，以blockA为准
					*2，planeA，planeB中的active_blockB，pageA位置都可用，那么不同plane 的相同位置，以blockB为准
					********************************************************************************************/
                    if ((ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeA].blk_head[active_blockA].page_head[pageA].free_state ==
                         PG_SUB)
                        &&
                        (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeB].blk_head[active_blockA].page_head[pageA].free_state ==
                         PG_SUB)) {
                        flash_page_state_modify(ssd, subA, channel, chip, die, planeA, active_blockA, pageA);
                        flash_page_state_modify(ssd, subB, channel, chip, die, planeB, active_blockA, pageA);
                    } else if (
                            (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeA].blk_head[active_blockB].page_head[pageA].free_state ==
                             PG_SUB)
                            &&
                            (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeB].blk_head[active_blockB].page_head[pageA].free_state ==
                             PG_SUB)) {
                        flash_page_state_modify(ssd, subA, channel, chip, die, planeA, active_blockB, pageA);
                        flash_page_state_modify(ssd, subB, channel, chip, die, planeB, active_blockB, pageA);
                    } else {
                        subA = NULL;
                        subB = NULL;
                        ssd->channel_head[channel].chip_head[chip].die_head[die].token = old_plane;
                        return FAILURE;
                    }
                } else {
                    subA = NULL;
                    subB = NULL;
                    ssd->channel_head[channel].chip_head[chip].die_head[die].token = old_plane;
                    return ERROR;
                }
            }
        }
    }

    if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeA].free_page <
        (ssd->parameter->page_block * ssd->parameter->block_plane * ssd->parameter->gc_hard_threshold)) {
        gc_node = (struct gc_operation *) malloc(sizeof(struct gc_operation));
        alloc_assert(gc_node, "gc_node");
        memset(gc_node, 0, sizeof(struct gc_operation));

        gc_node->next_node = NULL;
        gc_node->chip = chip;
        gc_node->die = die;
        gc_node->plane = planeA;
        gc_node->block = 0xffffffff;
        gc_node->page = 0;
        gc_node->state = GC_WAIT;
        gc_node->priority = GC_UNINTERRUPT;
        gc_node->next_node = ssd->channel_head[channel].gc_command;
        ssd->channel_head[channel].gc_command = gc_node;
        ssd->gc_request++;
    }
    if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[planeB].free_page <
        (ssd->parameter->page_block * ssd->parameter->block_plane * ssd->parameter->gc_hard_threshold)) {
        gc_node = (struct gc_operation *) malloc(sizeof(struct gc_operation));
        alloc_assert(gc_node, "gc_node");
        memset(gc_node, 0, sizeof(struct gc_operation));

        gc_node->next_node = NULL;
        gc_node->chip = chip;
        gc_node->die = die;
        gc_node->plane = planeB;
        gc_node->block = 0xffffffff;
        gc_node->page = 0;
        gc_node->state = GC_WAIT;
        gc_node->priority = GC_UNINTERRUPT;
        gc_node->next_node = ssd->channel_head[channel].gc_command;
        ssd->channel_head[channel].gc_command = gc_node;
        ssd->gc_request++;
    }

    return SUCCESS;
}

/*
*函数的功能是修改找到的page页的状态以及相应的dram中映射表的值
*/
struct ssd_info *
flash_page_state_modify(struct ssd_info *ssd, struct sub_request *sub, unsigned int channel, unsigned int chip,
                        unsigned int die, unsigned int plane, unsigned int block, unsigned int page) {
    unsigned int ppn, full_page;
    struct local *location;
    struct direct_erase *new_direct_erase, *direct_erase_node;

    full_page = ~(0xffffffff << ssd->parameter->subpage_page);
    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].last_write_page = page;
    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].free_page_num--;

    if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].last_write_page >
            ssd->parameter->page_block) {
        printf("error! the last write page larger than %d\n", ssd->parameter->page_block);
        while (1) {}
    }

    if (ssd->dram->map->map_entry[sub->lpn].state ==
        0)                                          /*this is the first logical page*/
    {
        ssd->dram->map->map_entry[sub->lpn].pn = find_ppn(ssd, channel, chip, die, plane, block, page);
        ssd->dram->map->map_entry[sub->lpn].state = sub->state;
    } else                                                                                      /*这个逻辑页进行了更新，需要将原来的页置为失效*/
    {
        ppn = ssd->dram->map->map_entry[sub->lpn].pn;
        location = find_location(ssd, ppn);
        ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].valid_state = 0;        //表示某一页失效，同时标记valid和free状态都为0
        ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].free_state = 0;         //表示某一页失效，同时标记valid和free状态都为0
        ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].page_head[location->page].lpn = 0;
        ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].invalid_page_num++;
        if (ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].blk_head[location->block].invalid_page_num ==
            ssd->parameter->page_block)    //该block中全是invalid的页，可以直接删除
        {
            new_direct_erase = (struct direct_erase *) malloc(sizeof(struct direct_erase));
            alloc_assert(new_direct_erase, "new_direct_erase");
            memset(new_direct_erase, 0, sizeof(struct direct_erase));

            new_direct_erase->block = location->block;
            new_direct_erase->next_node = NULL;
            direct_erase_node = ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].erase_node;
            if (direct_erase_node == NULL) {
                ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].erase_node = new_direct_erase;
            } else {
                new_direct_erase->next_node = ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].erase_node;
                ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].erase_node = new_direct_erase;
            }
        }
        free(location);
        location = NULL;
        ssd->dram->map->map_entry[sub->lpn].pn = find_ppn(ssd, channel, chip, die, plane, block, page);
        ssd->dram->map->map_entry[sub->lpn].state = (ssd->dram->map->map_entry[sub->lpn].state | sub->state);
    }

    sub->ppn = ssd->dram->map->map_entry[sub->lpn].pn;
    sub->location->channel = channel;
    sub->location->chip = chip;
    sub->location->die = die;
    sub->location->plane = plane;
    sub->location->block = block;
    sub->location->page = page;

    ssd->program_count++;
    ssd->channel_head[channel].program_count++;
    ssd->channel_head[channel].chip_head[chip].program_count++;
    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].free_page--;
    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[page].lpn = sub->lpn;
    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[page].valid_state = sub->state;
    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[page].free_state = (
            (~(sub->state)) & full_page);
    ssd->write_flash_count++;

    return ssd;
}


/********************************************
*函数的功能就是让两个位置不同的page位置相同
*********************************************/
struct ssd_info *
make_same_level(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die, unsigned int plane,
                unsigned int block, unsigned int aim_page) {
    int i = 0, step, page;
    struct direct_erase *new_direct_erase, *direct_erase_node;

    page = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].last_write_page +
           1;                  /*需要调整的当前块的可写页号*/
    step = aim_page - page;
    while (i < step) {
        ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[page +
                                                                                                             i].valid_state = 0;     /*表示某一页失效，同时标记valid和free状态都为0*/
        ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[page +
                                                                                                             i].free_state = 0;      /*表示某一页失效，同时标记valid和free状态都为0*/
        ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].page_head[page +
                                                                                                             i].lpn = 0;

        ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].invalid_page_num++;

        ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].free_page_num--;

        ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].free_page--;

        i++;
    }

    ssd->waste_page_count += step;

    ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].last_write_page =
            aim_page - 1;

    if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].invalid_page_num ==
        ssd->parameter->page_block)    /*该block中全是invalid的页，可以直接删除*/
    {
        new_direct_erase = (struct direct_erase *) malloc(sizeof(struct direct_erase));
        alloc_assert(new_direct_erase, "new_direct_erase");
        memset(new_direct_erase, 0, sizeof(struct direct_erase));

        direct_erase_node = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node;
        if (direct_erase_node == NULL) {
            ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node = new_direct_erase;
        } else {
            new_direct_erase->next_node = ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node;
            ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].erase_node = new_direct_erase;
        }
    }

    if (ssd->channel_head[channel].chip_head[chip].die_head[die].plane_head[plane].blk_head[block].last_write_page >
            ssd->parameter->page_block) {
        printf("error! the last write page larger than %d\n", ssd->parameter->page_block);
        while (1) {}
    }

    return ssd;
}


/****************************************************************************
*在处理高级命令的写子请求时，这个函数的功能就是计算处理时间以及处理的状态转变
*功能还不是很完善，需要完善，修改时注意要分为静态分配和动态分配两种情况
*****************************************************************************/
struct ssd_info *compute_serve_time(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die,
                                    struct sub_request **subs, unsigned int subs_count, unsigned int command) {
    unsigned int i = 0;
    unsigned int max_subs_num = 0;
    struct sub_request *sub = NULL, *p = NULL;
    struct sub_request *last_sub = NULL;
    max_subs_num = ssd->parameter->die_chip * ssd->parameter->plane_die;

    if ((command == INTERLEAVE_TWO_PLANE) || (command == COPY_BACK)) {
        for (i = 0; i < max_subs_num; i++) {
            if (subs[i] != NULL) {
                last_sub = subs[i];
                subs[i]->current_state = SR_W_TRANSFER;
                subs[i]->current_time = ssd->current_time;
                subs[i]->next_state = SR_COMPLETE;
                subs[i]->next_state_predict_time = ssd->current_time + 7 * ssd->parameter->time_characteristics.tWC +
                                                   (subs[i]->size * ssd->parameter->subpage_capacity) *
                                                   ssd->parameter->time_characteristics.tWC;
                subs[i]->complete_time = subs[i]->next_state_predict_time;

                delete_from_channel(ssd, channel, subs[i]);
            }
        }
        ssd->channel_head[channel].current_state = CHANNEL_TRANSFER;
        ssd->channel_head[channel].current_time = ssd->current_time;
        ssd->channel_head[channel].next_state = CHANNEL_IDLE;
        ssd->channel_head[channel].next_state_predict_time = last_sub->complete_time;

        ssd->channel_head[channel].chip_head[chip].current_state = CHIP_WRITE_BUSY;
        ssd->channel_head[channel].chip_head[chip].current_time = ssd->current_time;
        ssd->channel_head[channel].chip_head[chip].next_state = CHIP_IDLE;
        ssd->channel_head[channel].chip_head[chip].next_state_predict_time =
                ssd->channel_head[channel].next_state_predict_time + ssd->parameter->time_characteristics.tPROG;
    } else if (command == TWO_PLANE) {
        for (i = 0; i < max_subs_num; i++) {
            if (subs[i] != NULL) {

                subs[i]->current_state = SR_W_TRANSFER;
                if (last_sub == NULL) {
                    subs[i]->current_time = ssd->current_time;
                } else {
                    subs[i]->current_time = last_sub->complete_time + ssd->parameter->time_characteristics.tDBSY;
                }

                subs[i]->next_state = SR_COMPLETE;
                subs[i]->next_state_predict_time =
                        subs[i]->current_time + 7 * ssd->parameter->time_characteristics.tWC +
                        (subs[i]->size * ssd->parameter->subpage_capacity) * ssd->parameter->time_characteristics.tWC;
                subs[i]->complete_time = subs[i]->next_state_predict_time;
                last_sub = subs[i];

                delete_from_channel(ssd, channel, subs[i]);
            }
        }
        ssd->channel_head[channel].current_state = CHANNEL_TRANSFER;
        ssd->channel_head[channel].current_time = ssd->current_time;
        ssd->channel_head[channel].next_state = CHANNEL_IDLE;
        ssd->channel_head[channel].next_state_predict_time = last_sub->complete_time;

        ssd->channel_head[channel].chip_head[chip].current_state = CHIP_WRITE_BUSY;
        ssd->channel_head[channel].chip_head[chip].current_time = ssd->current_time;
        ssd->channel_head[channel].chip_head[chip].next_state = CHIP_IDLE;
        ssd->channel_head[channel].chip_head[chip].next_state_predict_time =
                ssd->channel_head[channel].next_state_predict_time + ssd->parameter->time_characteristics.tPROG;
    } else if (command == INTERLEAVE) {
        for (i = 0; i < max_subs_num; i++) {
            if (subs[i] != NULL) {

                subs[i]->current_state = SR_W_TRANSFER;
                if (last_sub == NULL) {
                    subs[i]->current_time = ssd->current_time;
                } else {
                    subs[i]->current_time = last_sub->complete_time;
                }
                subs[i]->next_state = SR_COMPLETE;
                subs[i]->next_state_predict_time =
                        subs[i]->current_time + 7 * ssd->parameter->time_characteristics.tWC +
                        (subs[i]->size * ssd->parameter->subpage_capacity) * ssd->parameter->time_characteristics.tWC;
                subs[i]->complete_time = subs[i]->next_state_predict_time;
                last_sub = subs[i];

                delete_from_channel(ssd, channel, subs[i]);
            }
        }
        ssd->channel_head[channel].current_state = CHANNEL_TRANSFER;
        ssd->channel_head[channel].current_time = ssd->current_time;
        ssd->channel_head[channel].next_state = CHANNEL_IDLE;
        ssd->channel_head[channel].next_state_predict_time = last_sub->complete_time;

        ssd->channel_head[channel].chip_head[chip].current_state = CHIP_WRITE_BUSY;
        ssd->channel_head[channel].chip_head[chip].current_time = ssd->current_time;
        ssd->channel_head[channel].chip_head[chip].next_state = CHIP_IDLE;
        ssd->channel_head[channel].chip_head[chip].next_state_predict_time =
                ssd->channel_head[channel].next_state_predict_time + ssd->parameter->time_characteristics.tPROG;
    } else if (command == NORMAL) {
        subs[0]->current_state = SR_W_TRANSFER;
        subs[0]->current_time = ssd->current_time;
        subs[0]->next_state = SR_COMPLETE;
        subs[0]->next_state_predict_time = ssd->current_time + 7 * ssd->parameter->time_characteristics.tWC +
                                           (subs[0]->size * ssd->parameter->subpage_capacity) *
                                           ssd->parameter->time_characteristics.tWC;
        subs[0]->complete_time = subs[0]->next_state_predict_time;

        delete_from_channel(ssd, channel, subs[0]);

        ssd->channel_head[channel].current_state = CHANNEL_TRANSFER;
        ssd->channel_head[channel].current_time = ssd->current_time;
        ssd->channel_head[channel].next_state = CHANNEL_IDLE;
        ssd->channel_head[channel].next_state_predict_time = subs[0]->complete_time;

        ssd->channel_head[channel].chip_head[chip].current_state = CHIP_WRITE_BUSY;
        ssd->channel_head[channel].chip_head[chip].current_time = ssd->current_time;
        ssd->channel_head[channel].chip_head[chip].next_state = CHIP_IDLE;
        ssd->channel_head[channel].chip_head[chip].next_state_predict_time =
                ssd->channel_head[channel].next_state_predict_time + ssd->parameter->time_characteristics.tPROG;
    } else {
        return NULL;
    }

    return ssd;

}


/*****************************************************************************************
*函数的功能就是把子请求从ssd->subs_w_head或者ssd->channel_head[channel].subs_w_head上删除
******************************************************************************************/
struct ssd_info *delete_from_channel(struct ssd_info *ssd, unsigned int channel, struct sub_request *sub_req) {
    struct sub_request *sub, *p;

    /******************************************************************
	*完全动态分配子请求就在ssd->subs_w_head上
	*不是完全动态分配子请求就在ssd->channel_head[channel].subs_w_head上
	*******************************************************************/
    if ((ssd->parameter->allocation_scheme == 0) && (ssd->parameter->dynamic_allocation == 0)) {
        sub = ssd->subs_w_head;
    } else {
        sub = ssd->channel_head[channel].subs_w_head;
    }
    p = sub;

    while (sub != NULL) {
        if (sub == sub_req) {
            if ((ssd->parameter->allocation_scheme == 0) && (ssd->parameter->dynamic_allocation == 0)) {
                if (ssd->parameter->ad_priority2 == 0) {
                    ssd->real_time_subreq--;
                }

                if (sub ==
                    ssd->subs_w_head)                                                     /*将这个子请求从sub request队列中删除*/
                {
                    if (ssd->subs_w_head != ssd->subs_w_tail) {
                        ssd->subs_w_head = sub->next_node;
                        sub = ssd->subs_w_head;
                        continue;
                    } else {
                        ssd->subs_w_head = NULL;
                        ssd->subs_w_tail = NULL;
                        p = NULL;
                        break;
                    }
                }//if (sub==ssd->subs_w_head)
                else {
                    if (sub->next_node != NULL) {
                        p->next_node = sub->next_node;
                        sub = p->next_node;
                        continue;
                    } else {
                        ssd->subs_w_tail = p;
                        ssd->subs_w_tail->next_node = NULL;
                        break;
                    }
                }
            }//if ((ssd->parameter->allocation_scheme==0)&&(ssd->parameter->dynamic_allocation==0))
            else {
                if (sub == ssd->channel_head[channel].subs_w_head)                               /*将这个子请求从channel队列中删除*/
                {
                    if (ssd->channel_head[channel].subs_w_head != ssd->channel_head[channel].subs_w_tail) {
                        ssd->channel_head[channel].subs_w_head = sub->next_node;
                        sub = ssd->channel_head[channel].subs_w_head;
                        continue;;
                    } else {
                        ssd->channel_head[channel].subs_w_head = NULL;
                        ssd->channel_head[channel].subs_w_tail = NULL;
                        p = NULL;
                        break;
                    }
                }//if (sub==ssd->channel_head[channel].subs_w_head)
                else {
                    if (sub->next_node != NULL) {
                        p->next_node = sub->next_node;
                        sub = p->next_node;
                        continue;
                    } else {
                        ssd->channel_head[channel].subs_w_tail = p;
                        ssd->channel_head[channel].subs_w_tail->next_node = NULL;
                        break;
                    }
                }//else
            }//else
        }//if (sub==sub_req)
        p = sub;
        sub = sub->next_node;
    }//while (sub!=NULL)

    return ssd;
}


struct ssd_info *
un_greed_interleave_copyback(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die,
                             struct sub_request *sub1, struct sub_request *sub2) {
    unsigned int old_ppn1, ppn1, old_ppn2, ppn2, greed_flag = 0;

    old_ppn1 = ssd->dram->map->map_entry[sub1->lpn].pn;
    get_ppn(ssd, channel, chip, die, sub1->location->plane,
            sub1);                                  /*找出来的ppn一定是发生在与子请求相同的plane中,才能使用copyback操作*/
    ppn1 = sub1->ppn;

    old_ppn2 = ssd->dram->map->map_entry[sub2->lpn].pn;
    get_ppn(ssd, channel, chip, die, sub2->location->plane,
            sub2);                                  /*找出来的ppn一定是发生在与子请求相同的plane中,才能使用copyback操作*/
    ppn2 = sub2->ppn;

    if ((old_ppn1 % 2 == ppn1 % 2) && (old_ppn2 % 2 == ppn2 % 2)) {
        ssd->copy_back_count++;
        ssd->copy_back_count++;

        sub1->current_state = SR_W_TRANSFER;
        sub1->current_time = ssd->current_time;
        sub1->next_state = SR_COMPLETE;
        sub1->next_state_predict_time = ssd->current_time + 14 * ssd->parameter->time_characteristics.tWC +
                                        ssd->parameter->time_characteristics.tR +
                                        (sub1->size * ssd->parameter->subpage_capacity) *
                                        ssd->parameter->time_characteristics.tWC;
        sub1->complete_time = sub1->next_state_predict_time;

        sub2->current_state = SR_W_TRANSFER;
        sub2->current_time = sub1->complete_time;
        sub2->next_state = SR_COMPLETE;
        sub2->next_state_predict_time = sub2->current_time + 14 * ssd->parameter->time_characteristics.tWC +
                                        ssd->parameter->time_characteristics.tR +
                                        (sub2->size * ssd->parameter->subpage_capacity) *
                                        ssd->parameter->time_characteristics.tWC;
        sub2->complete_time = sub2->next_state_predict_time;

        ssd->channel_head[channel].current_state = CHANNEL_TRANSFER;
        ssd->channel_head[channel].current_time = ssd->current_time;
        ssd->channel_head[channel].next_state = CHANNEL_IDLE;
        ssd->channel_head[channel].next_state_predict_time = sub2->complete_time;

        ssd->channel_head[channel].chip_head[chip].current_state = CHIP_WRITE_BUSY;
        ssd->channel_head[channel].chip_head[chip].current_time = ssd->current_time;
        ssd->channel_head[channel].chip_head[chip].next_state = CHIP_IDLE;
        ssd->channel_head[channel].chip_head[chip].next_state_predict_time =
                ssd->channel_head[channel].next_state_predict_time + ssd->parameter->time_characteristics.tPROG;

        delete_from_channel(ssd, channel, sub1);
        delete_from_channel(ssd, channel, sub2);
    } //if ((old_ppn1%2==ppn1%2)&&(old_ppn2%2==ppn2%2))
    else if ((old_ppn1 % 2 == ppn1 % 2) && (old_ppn2 % 2 != ppn2 % 2)) {
        ssd->interleave_count--;
        ssd->copy_back_count++;

        sub1->current_state = SR_W_TRANSFER;
        sub1->current_time = ssd->current_time;
        sub1->next_state = SR_COMPLETE;
        sub1->next_state_predict_time = ssd->current_time + 14 * ssd->parameter->time_characteristics.tWC +
                                        ssd->parameter->time_characteristics.tR +
                                        (sub1->size * ssd->parameter->subpage_capacity) *
                                        ssd->parameter->time_characteristics.tWC;
        sub1->complete_time = sub1->next_state_predict_time;

        ssd->channel_head[channel].current_state = CHANNEL_TRANSFER;
        ssd->channel_head[channel].current_time = ssd->current_time;
        ssd->channel_head[channel].next_state = CHANNEL_IDLE;
        ssd->channel_head[channel].next_state_predict_time = sub1->complete_time;

        ssd->channel_head[channel].chip_head[chip].current_state = CHIP_WRITE_BUSY;
        ssd->channel_head[channel].chip_head[chip].current_time = ssd->current_time;
        ssd->channel_head[channel].chip_head[chip].next_state = CHIP_IDLE;
        ssd->channel_head[channel].chip_head[chip].next_state_predict_time =
                ssd->channel_head[channel].next_state_predict_time + ssd->parameter->time_characteristics.tPROG;

        delete_from_channel(ssd, channel, sub1);
    }//else if ((old_ppn1%2==ppn1%2)&&(old_ppn2%2!=ppn2%2))
    else if ((old_ppn1 % 2 != ppn1 % 2) && (old_ppn2 % 2 == ppn2 % 2)) {
        ssd->interleave_count--;
        ssd->copy_back_count++;

        sub2->current_state = SR_W_TRANSFER;
        sub2->current_time = ssd->current_time;
        sub2->next_state = SR_COMPLETE;
        sub2->next_state_predict_time = ssd->current_time + 14 * ssd->parameter->time_characteristics.tWC +
                                        ssd->parameter->time_characteristics.tR +
                                        (sub2->size * ssd->parameter->subpage_capacity) *
                                        ssd->parameter->time_characteristics.tWC;
        sub2->complete_time = sub2->next_state_predict_time;

        ssd->channel_head[channel].current_state = CHANNEL_TRANSFER;
        ssd->channel_head[channel].current_time = ssd->current_time;
        ssd->channel_head[channel].next_state = CHANNEL_IDLE;
        ssd->channel_head[channel].next_state_predict_time = sub2->complete_time;

        ssd->channel_head[channel].chip_head[chip].current_state = CHIP_WRITE_BUSY;
        ssd->channel_head[channel].chip_head[chip].current_time = ssd->current_time;
        ssd->channel_head[channel].chip_head[chip].next_state = CHIP_IDLE;
        ssd->channel_head[channel].chip_head[chip].next_state_predict_time =
                ssd->channel_head[channel].next_state_predict_time + ssd->parameter->time_characteristics.tPROG;

        delete_from_channel(ssd, channel, sub2);
    }//else if ((old_ppn1%2!=ppn1%2)&&(old_ppn2%2==ppn2%2))
    else {
        ssd->interleave_count--;

        sub1->current_state = SR_W_TRANSFER;
        sub1->current_time = ssd->current_time;
        sub1->next_state = SR_COMPLETE;
        sub1->next_state_predict_time = ssd->current_time + 14 * ssd->parameter->time_characteristics.tWC +
                                        ssd->parameter->time_characteristics.tR +
                                        2 * (ssd->parameter->subpage_page * ssd->parameter->subpage_capacity) *
                                        ssd->parameter->time_characteristics.tWC;
        sub1->complete_time = sub1->next_state_predict_time;

        ssd->channel_head[channel].current_state = CHANNEL_TRANSFER;
        ssd->channel_head[channel].current_time = ssd->current_time;
        ssd->channel_head[channel].next_state = CHANNEL_IDLE;
        ssd->channel_head[channel].next_state_predict_time = sub1->complete_time;

        ssd->channel_head[channel].chip_head[chip].current_state = CHIP_WRITE_BUSY;
        ssd->channel_head[channel].chip_head[chip].current_time = ssd->current_time;
        ssd->channel_head[channel].chip_head[chip].next_state = CHIP_IDLE;
        ssd->channel_head[channel].chip_head[chip].next_state_predict_time =
                ssd->channel_head[channel].next_state_predict_time + ssd->parameter->time_characteristics.tPROG;

        delete_from_channel(ssd, channel, sub1);
    }//else

    return ssd;
}


struct ssd_info *un_greed_copyback(struct ssd_info *ssd, unsigned int channel, unsigned int chip, unsigned int die,
                                   struct sub_request *sub1) {
    unsigned int old_ppn, ppn;

    old_ppn = ssd->dram->map->map_entry[sub1->lpn].pn;
    get_ppn(ssd, channel, chip, die, 0,
            sub1);                                                     /*找出来的ppn一定是发生在与子请求相同的plane中,才能使用copyback操作*/
    ppn = sub1->ppn;

    if (old_ppn % 2 == ppn % 2) {
        ssd->copy_back_count++;
        sub1->current_state = SR_W_TRANSFER;
        sub1->current_time = ssd->current_time;
        sub1->next_state = SR_COMPLETE;
        sub1->next_state_predict_time = ssd->current_time + 14 * ssd->parameter->time_characteristics.tWC +
                                        ssd->parameter->time_characteristics.tR +
                                        (sub1->size * ssd->parameter->subpage_capacity) *
                                        ssd->parameter->time_characteristics.tWC;
        sub1->complete_time = sub1->next_state_predict_time;

        ssd->channel_head[channel].current_state = CHANNEL_TRANSFER;
        ssd->channel_head[channel].current_time = ssd->current_time;
        ssd->channel_head[channel].next_state = CHANNEL_IDLE;
        ssd->channel_head[channel].next_state_predict_time = sub1->complete_time;

        ssd->channel_head[channel].chip_head[chip].current_state = CHIP_WRITE_BUSY;
        ssd->channel_head[channel].chip_head[chip].current_time = ssd->current_time;
        ssd->channel_head[channel].chip_head[chip].next_state = CHIP_IDLE;
        ssd->channel_head[channel].chip_head[chip].next_state_predict_time =
                ssd->channel_head[channel].next_state_predict_time + ssd->parameter->time_characteristics.tPROG;
    }//if (old_ppn%2==ppn%2)
    else {
        sub1->current_state = SR_W_TRANSFER;
        sub1->current_time = ssd->current_time;
        sub1->next_state = SR_COMPLETE;
        sub1->next_state_predict_time = ssd->current_time + 14 * ssd->parameter->time_characteristics.tWC +
                                        ssd->parameter->time_characteristics.tR +
                                        2 * (ssd->parameter->subpage_page * ssd->parameter->subpage_capacity) *
                                        ssd->parameter->time_characteristics.tWC;
        sub1->complete_time = sub1->next_state_predict_time;

        ssd->channel_head[channel].current_state = CHANNEL_TRANSFER;
        ssd->channel_head[channel].current_time = ssd->current_time;
        ssd->channel_head[channel].next_state = CHANNEL_IDLE;
        ssd->channel_head[channel].next_state_predict_time = sub1->complete_time;

        ssd->channel_head[channel].chip_head[chip].current_state = CHIP_WRITE_BUSY;
        ssd->channel_head[channel].chip_head[chip].current_time = ssd->current_time;
        ssd->channel_head[channel].chip_head[chip].next_state = CHIP_IDLE;
        ssd->channel_head[channel].chip_head[chip].next_state_predict_time =
                ssd->channel_head[channel].next_state_predict_time + ssd->parameter->time_characteristics.tPROG;
    }//else

    delete_from_channel(ssd, channel, sub1);

    return ssd;
}


/****************************************************************************************
*函数的功能是在处理读子请求的高级命令时，需要找与one_page相匹配的另外一个page即two_page
*没有找到可以和one_page执行two plane或者interleave操作的页,需要将one_page向后移一个节点
*****************************************************************************************/
struct sub_request *
find_interleave_twoplane_page(struct ssd_info *ssd, struct sub_request *one_page, unsigned int command) {
    struct sub_request *two_page;

    if (one_page->current_state != SR_WAIT) {
        return NULL;
    }
    if (((ssd->channel_head[one_page->location->channel].chip_head[one_page->location->chip].current_state ==
          CHIP_IDLE) ||
         ((ssd->channel_head[one_page->location->channel].chip_head[one_page->location->chip].next_state ==
           CHIP_IDLE) &&
          (ssd->channel_head[one_page->location->channel].chip_head[one_page->location->chip].next_state_predict_time <=
           ssd->current_time)))) {
        two_page = one_page->next_node;
        if (command == TWO_PLANE) {
            while (two_page != NULL) {
                if (two_page->current_state != SR_WAIT) {
                    two_page = two_page->next_node;
                } else if ((one_page->location->chip == two_page->location->chip) &&
                           (one_page->location->die == two_page->location->die) &&
                           (one_page->location->block == two_page->location->block) &&
                           (one_page->location->page == two_page->location->page)) {
                    if (one_page->location->plane != two_page->location->plane) {
                        return two_page;                                                       /*找到了与one_page可以执行two plane操作的页*/
                    } else {
                        two_page = two_page->next_node;
                    }
                } else {
                    two_page = two_page->next_node;
                }
            }//while (two_page!=NULL)
            if (two_page ==
                NULL)                                                               /*没有找到可以和one_page执行two_plane操作的页,需要将one_page向后移一个节点*/
            {
                return NULL;
            }
        }//if(command==TWO_PLANE)
        else if (command == INTERLEAVE) {
            while (two_page != NULL) {
                if (two_page->current_state != SR_WAIT) {
                    two_page = two_page->next_node;
                } else if ((one_page->location->chip == two_page->location->chip) &&
                           (one_page->location->die != two_page->location->die)) {
                    return two_page;                                                           /*找到了与one_page可以执行interleave操作的页*/
                } else {
                    two_page = two_page->next_node;
                }
            }
            if (two_page ==
                NULL)                                                                /*没有找到可以和one_page执行interleave操作的页,需要将one_page向后移一个节点*/
            {
                return NULL;
            }//while (two_page!=NULL)
        }//else if(command==INTERLEAVE)

    }
    {
        return NULL;
    }
}


/*************************************************************************
*在处理读子请求高级命令时，利用这个还是查找可以执行高级命令的sub_request
**************************************************************************/
int
find_interleave_twoplane_sub_request(struct ssd_info *ssd, unsigned int channel, struct sub_request *sub_request_one,
                                     struct sub_request *sub_request_two, unsigned int command) {
    sub_request_one = ssd->channel_head[channel].subs_r_head;
    while (sub_request_one != NULL) {
        sub_request_two = find_interleave_twoplane_page(ssd, sub_request_one,
                                                        command);                /*找出两个可以做two_plane或者interleave的read子请求，包括位置条件和时间条件*/
        if (sub_request_two == NULL) {
            sub_request_one = sub_request_one->next_node;
        } else if (sub_request_two !=
                   NULL)                                                            /*找到了两个可以执行two plane操作的页*/
        {
            break;
        }
    }

    if (sub_request_two != NULL) {
        if (ssd->request_queue !=
            ssd->request_tail) {                                                                                         /*确保interleave read的子请求是第一个请求的子请求*/
            if ((ssd->request_queue->lsn - ssd->parameter->subpage_page) <
                (sub_request_one->lpn * ssd->parameter->subpage_page)) {
                if ((ssd->request_queue->lsn + ssd->request_queue->size + ssd->parameter->subpage_page) >
                    (sub_request_one->lpn * ssd->parameter->subpage_page)) {
                } else {
                    sub_request_two = NULL;
                }
            } else {
                sub_request_two = NULL;
            }
        }//if (ssd->request_queue!=ssd->request_tail)
    }//if (sub_request_two!=NULL)

    if (sub_request_two != NULL) {
        return SUCCESS;
    } else {
        return FAILURE;
    }

}


/**************************************************************************
*这个函数非常重要，读子请求的状态转变，以及时间的计算都通过这个函数来处理
*还有写子请求的执行普通命令时的状态，以及时间的计算也是通过这个函数来处理的
****************************************************************************/
Status go_one_step(struct ssd_info *ssd, struct sub_request *sub1, struct sub_request *sub2, unsigned int aim_state,
                   unsigned int command) {
    unsigned int i = 0, j = 0, k = 0, m = 0;
    long long time = 0;
    struct sub_request *sub = NULL;
    struct sub_request *sub_twoplane_one = NULL, *sub_twoplane_two = NULL;
    struct sub_request *sub_interleave_one = NULL, *sub_interleave_two = NULL;
    struct local *location = NULL;
    if (sub1 == NULL) {
        return ERROR;
    }
#ifdef DEBUG
            printf("enter the go_one_step function,sub address point is %lld,sub channel %d, sub chip %d, sub die %d, sub plane %d, sub block %d, sub page %d, sub sub-page %d, aim_state is %d, command is %d\n",sub1, sub1->location->channel,sub1->location->chip,sub1->location->die,sub1->location->plane,sub1->location->block,sub1->location->page,sub1->location->sub_page, aim_state, command);
#endif

    /***************************************************************************************************
	*处理普通命令时，读子请求的目标状态分为以下几种情况SR_R_READ，SR_R_C_A_TRANSFER，SR_R_DATA_TRANSFER
	*写子请求的目标状态只有SR_W_TRANSFER
	****************************************************************************************************/
    if (command == NORMAL) {
#ifdef DEBUG
            printf("enter normal command\n");
#endif

        sub = sub1;
        location = sub1->location;
        switch (aim_state) {
            case SR_R_READ: {
                /*****************************************************************************************************
			    *这个目标状态是指flash处于读数据的状态，sub的下一状态就应该是传送数据SR_R_DATA_TRANSFER
			    *这时与channel无关，只与chip有关所以要修改chip的状态为CHIP_READ_BUSY，下一个状态就是CHIP_DATA_TRANSFER
			    ******************************************************************************************************/

#ifdef DEBUG
                                printf("enter normal command's SR_R_REAS,and ssd->current_time is %llu \n",ssd->current_time);
#endif

                sub->current_time = ssd->current_time;
                sub->current_state = SR_R_READ;
                sub->next_state = SR_R_DATA_TRANSFER;
                sub->next_state_predict_time = ssd->current_time + ssd->parameter->time_characteristics.tR;

                ssd->channel_head[location->channel].chip_head[location->chip].current_state = CHIP_READ_BUSY;
                ssd->channel_head[location->channel].chip_head[location->chip].current_time = ssd->current_time;
                ssd->channel_head[location->channel].chip_head[location->chip].next_state = CHIP_DATA_TRANSFER;
                ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time =
                        ssd->current_time + ssd->parameter->time_characteristics.tR;

#ifdef DEBUGSUSPEND
                                        printf("SR_R_READ: sub information: channel %d, chip %d, die %d, plane %d, ppn %d, sub->current_time %lld, sub->next_time %lld, ssd->current_time %lld, ssd->current_state %d, ssd->next_state %d ,chip %d  next_predict_time %lld\n",sub->location->channel, sub->location->chip, sub->location->die, sub->location->plane, sub->ppn,  sub->current_time, sub->next_state_predict_time, ssd->current_time,sub->current_state, sub->next_state, location->chip, ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time );
#endif


                break;
            }
            case SR_R_C_A_TRANSFER: {
                /*******************************************************************************************************
				*目标状态是命令地址传输时，sub的下一个状态就是SR_R_READ
				*这个状态与channel，chip有关，所以要修改channel，chip的状态分别为CHANNEL_C_A_TRANSFER，CHIP_C_A_TRANSFER
				*下一状态分别为CHANNEL_IDLE，CHIP_READ_BUSY
				*******************************************************************************************************/

                sub->current_time = ssd->current_time;
                sub->current_state = SR_R_C_A_TRANSFER;
                sub->next_state = SR_R_READ;
                sub->next_state_predict_time = ssd->current_time + 7 * ssd->parameter->time_characteristics.tWC;
                sub->begin_time = ssd->current_time;

                ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].add_reg_ppn = sub->ppn;
                ssd->read_count++;

                ssd->channel_head[location->channel].current_state = CHANNEL_C_A_TRANSFER;
                ssd->channel_head[location->channel].current_time = ssd->current_time;
                ssd->channel_head[location->channel].next_state = CHANNEL_IDLE;
                ssd->channel_head[location->channel].next_state_predict_time =
                        ssd->current_time + 7 * ssd->parameter->time_characteristics.tWC;

                ssd->channel_head[location->channel].chip_head[location->chip].current_state = CHIP_C_A_TRANSFER;
                ssd->channel_head[location->channel].chip_head[location->chip].current_time = ssd->current_time;
                ssd->channel_head[location->channel].chip_head[location->chip].next_state = CHIP_READ_BUSY;
                ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time =
                        ssd->current_time + 7 * ssd->parameter->time_characteristics.tWC;

#ifdef DEBUGSUSPEND
                                        printf("SR_R_C_A_TRANSFER: sub information: channel %d, chip %d, die %d, plane %d, ppn %d, sub->current_time %lld, sub->next_time %lld, ssd->current_time %lld, ssd->current_state %d, ssd->next_state %d, chip %d  next_predict_time %lld  \n",sub->location->channel, sub->location->chip, sub->location->die, sub->location->plane, sub->ppn,  sub->current_time, sub->next_state_predict_time, ssd->current_time,sub->current_state, sub->next_state,location->chip,      ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time );
#endif


                break;

            }
            case SR_R_DATA_TRANSFER: {
                /**************************************************************************************************************
				*目标状态是数据传输时，sub的下一个状态就是完成状态SR_COMPLETE
				*这个状态的处理也与channel，chip有关，所以channel，chip的当前状态变为CHANNEL_DATA_TRANSFER，CHIP_DATA_TRANSFER
				*下一个状态分别为CHANNEL_IDLE，CHIP_IDLE。
				***************************************************************************************************************/


                sub->current_time = ssd->current_time;
                sub->current_state = SR_R_DATA_TRANSFER;
                sub->next_state = SR_COMPLETE;
                sub->next_state_predict_time = ssd->current_time + (sub->size * ssd->parameter->subpage_capacity) *
                                                                   ssd->parameter->time_characteristics.tRC;
                sub->complete_time = sub->next_state_predict_time;

                ssd->channel_head[location->channel].current_state = CHANNEL_DATA_TRANSFER;
                ssd->channel_head[location->channel].current_time = ssd->current_time;
                ssd->channel_head[location->channel].next_state = CHANNEL_IDLE;
                ssd->channel_head[location->channel].next_state_predict_time = sub->next_state_predict_time;

                ssd->channel_head[location->channel].chip_head[location->chip].current_state = CHIP_DATA_TRANSFER;
                ssd->channel_head[location->channel].chip_head[location->chip].current_time = ssd->current_time;
                ssd->channel_head[location->channel].chip_head[location->chip].next_state = CHIP_IDLE;
                ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time = sub->next_state_predict_time;

                ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].add_reg_ppn = -1;

#ifdef DEBUGSUSPEND
                printf("SR_R_DATA_TRANSFER: sub information: channel %d, chip %d, die %d, plane %d, ppn %d, sub->current_time %lld, sub->next_time %lld, ssd->current_time %lld, ssd->current_state %d, ssd->next_state %d, chip %d  next_predict_time %lld   ,\n",sub->location->channel, sub->location->chip, sub->location->die, sub->location->plane, sub->ppn,  sub->current_time, sub->next_state_predict_time, ssd->current_time, sub->current_state, sub->next_state,location->chip,    ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time   );
#endif



                break;
            }
            case SR_W_TRANSFER: { //value == 206 the total time consumer.
                /******************************************************************************************************
				*这是处理写子请求时，状态的转变以及时间的计算
				*虽然写子请求的处理状态也像读子请求那么多，但是写请求都是从上往plane中传输数据
				*这样就可以把几个状态当一个状态来处理，就当成SR_W_TRANSFER这个状态来处理，sub的下一个状态就是完成状态了
				*此时channel，chip的当前状态变为CHANNEL_TRANSFER，CHIP_WRITE_BUSY
				*下一个状态变为CHANNEL_IDLE，CHIP_IDLE
				*******************************************************************************************************/
#ifdef DEBUG
                                printf("enter normal command's SR_W_TRANSFERS,and ssd->current_time is %llu \n",ssd->current_time);
#endif

                sub->current_time = ssd->current_time;
                sub->current_state = SR_W_TRANSFER;
                sub->next_state = SR_COMPLETE;
                sub->next_state_predict_time = ssd->current_time + 7 * ssd->parameter->time_characteristics.tWC +
                                               (sub->size * ssd->parameter->subpage_capacity) *
                                               ssd->parameter->time_characteristics.tWC;
                sub->complete_time = sub->next_state_predict_time + ssd->parameter->time_characteristics.tPROG;//lxcprogram added tPROG. here also can be true if not adding the tPROG in sub.
                time = sub->complete_time;

                ssd->channel_head[location->channel].current_state = CHANNEL_TRANSFER;
                ssd->channel_head[location->channel].current_time = ssd->current_time;
                ssd->channel_head[location->channel].next_state = CHANNEL_IDLE;
                ssd->channel_head[location->channel].next_state_predict_time = time;

                ssd->channel_head[location->channel].chip_head[location->chip].current_state = CHIP_WRITE_BUSY;
                ssd->channel_head[location->channel].chip_head[location->chip].current_time = ssd->current_time;
                ssd->channel_head[location->channel].chip_head[location->chip].next_state = CHIP_IDLE;
                ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time =
                        time + ssd->parameter->time_characteristics.tPROG;

                break;
            }
//lxcprogram  here is the finer-grained.       divided the write into (cmd_and_data_transfer operation((7+2048)*25ns=51375ns) + 5 chip inner iterations operations(5*(8000+32000)=200,000ns)).
    case SR_W_C_A_DATA_TRANSFER: {   //value == 204 
                /******************************************************************************************************
				*这是处理写子请求时，bus and chip begin to be busy.状态的转变以及时间的计算
				*此时channel，chip的当前状态变为CHANNEL_TRANSFER，CHIP_WRITE_BUSY
				*下一个状态变为CHANNEL_IDLE，CHIP_WRITE_BUSY
				*******************************************************************************************************/
#ifdef DEBUG
                                printf("enter normal command's SR_W_C_A_DATA_TRANSFER,and ssd->current_time is %llu \n",ssd->current_time);
#endif


 if (ssd->dram->map->map_entry[sub->lpn].state !=
        0)                                    /*说明这个逻辑页之前有写过，需要使用先读出来，再写下去，否则直接写下去即可*/
    {
        if ((sub->state & ssd->dram->map->map_entry[sub->lpn].state) ==
            ssd->dram->map->map_entry[sub->lpn].state)   /*可以覆盖*/
        {
            sub->next_state_predict_time = ssd->current_time + 7 * ssd->parameter->time_characteristics.tWC +
                                           (sub->size * ssd->parameter->subpage_capacity) *
                                           ssd->parameter->time_characteristics.tWC;
        } else {
            sub->next_state_predict_time = ssd->current_time + 7 * ssd->parameter->time_characteristics.tWC +
                                           ssd->parameter->time_characteristics.tR +
                                           (size((ssd->dram->map->map_entry[sub->lpn].state ^ sub->state))) *
                                           ssd->parameter->time_characteristics.tRC +
                                           (sub->size * ssd->parameter->subpage_capacity) *
                                           ssd->parameter->time_characteristics.tWC;
            ssd->read_count++;
            ssd->update_read_count++;
        }
    } else {
        sub->next_state_predict_time = ssd->current_time + 7 * ssd->parameter->time_characteristics.tWC +
                                       (sub->size * ssd->parameter->subpage_capacity) *
                                       ssd->parameter->time_characteristics.tWC;
    }









                sub->current_time = ssd->current_time;
                sub->current_state = SR_W_C_A_DATA_TRANSFER;
                sub->next_state = SR_W_DATA_TRANSFER;
               // sub->next_state_predict_time = ssd->current_time + 7 * ssd->parameter->time_characteristics.tWC +
               //                                (sub->size * ssd->parameter->subpage_capacity) *
               //                                ssd->parameter->time_characteristics.tWC;
                //sub->complete_time = sub->next_state_predict_time;
                //time = sub->complete_time;

                ssd->channel_head[location->channel].current_state = CHANNEL_TRANSFER;
                ssd->channel_head[location->channel].current_time = ssd->current_time;
                ssd->channel_head[location->channel].next_state = CHANNEL_IDLE;
                ssd->channel_head[location->channel].next_state_predict_time =  sub->next_state_predict_time;

                ssd->channel_head[location->channel].chip_head[location->chip].current_state = CHIP_WRITE_DATA_CMD_BUSY;
                ssd->channel_head[location->channel].chip_head[location->chip].current_time = ssd->current_time;
                ssd->channel_head[location->channel].chip_head[location->chip].next_state = CHIP_WRITE_BUSY;
                ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time = sub->next_state_predict_time;





#ifdef DEBUGSUSPEND
                printf("SR_W_C_A_DATA_TRANSFER: sub information: channel %d, chip %d, die %d, plane %d, ppn %d, sub->current_time %lld, sub->next_time %lld, ssd->current_time %lld, ssd->current_state %d, ssd->next_state %d, chip %d  next_predict_time %lld    \n",sub->location->channel, sub->location->chip, sub->location->die, sub->location->plane, sub->ppn,  sub->current_time, sub->next_state_predict_time, ssd->current_time, sub->current_state, sub->next_state,location->chip,    ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time   );
#endif


                break;
            }
    case SR_W_DATA_TRANSFER_ONE_PROG_OF_ITERATIONS: {// value == 205  //just do on time of the program in one iterations.
                /******************************************************************************************************
				*这是处理写子请求时，状态的转变以及时间的计算
			        *此时channel，chip的当前状态变为CHANNEL_IDLE，CHIP_WRITE_BUSY
				*下一个状态变为CHIP_IDLE
				*******************************************************************************************************/
#ifdef DEBUG
                                printf("enter normal command's SR_W_DATA_TRANSFER_ONE_PROG_OF_ITERATIONS,and ssd->current_time is %llu \n",ssd->current_time);
#endif
 

                sub->current_time = ssd->current_time;
                sub->current_state = SR_W_DATA_TRANSFER;
                sub->next_state_predict_time = ssd->current_time + ssd->parameter->time_characteristics.tPROG_w_progam;

                ssd->channel_head[location->channel].chip_head[location->chip].current_state = CHIP_WRITE_BUSY;
                ssd->channel_head[location->channel].chip_head[location->chip].current_time = ssd->current_time;
                ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time = sub->next_state_predict_time; 
                ssd->channel_head[location->channel].chip_head[location->chip].next_state = CHIP_WRITE_BUSY;
                sub->next_state = SR_W_DATA_TRANSFER;


#ifdef DEBUGSUSPEND
                printf("SR_W_DATA_TRANSFER_ONE_PROG_OF_ITERATIONS: sub information: channel %d, chip %d, die %d, plane %d, ppn %d, sub->current_time %lld, sub->next_time %lld, ssd->current_time %lld, ssd->current_state %d, ssd->next_state %d, chip %d  next_predict_time %lld    \n",sub->location->channel, sub->location->chip, sub->location->die, sub->location->plane, sub->ppn,  sub->current_time, sub->next_state_predict_time, ssd->current_time, sub->current_state, sub->next_state,location->chip,    ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time   );
#endif


                break;
            }

case SR_W_DATA_TRANSFER_ONE_VERIFY_OF_ITERATIONS: {// value == 207  //just do on time of the verify in one iterations.
                /******************************************************************************************************
				*这是处理写子请求时，状态的转变以及时间的计算
			        *此时channel，chip的当前状态变为CHANNEL_IDLE，CHIP_WRITE_BUSY
				*下一个状态变为CHIP_IDLE
				*******************************************************************************************************/
#ifdef DEBUG
                                printf("enter normal command's SR_W_DATA_TRANSFER_ONE_VERIFY_OF_ITERATIONS,and ssd->current_time is %llu \n",ssd->current_time);
#endif
 

                sub->current_time = ssd->current_time;
                sub->current_state = SR_W_DATA_TRANSFER;
                sub->next_state_predict_time = ssd->current_time + ssd->parameter->time_characteristics.tPROG_w_verify;

                ssd->channel_head[location->channel].chip_head[location->chip].current_state = CHIP_WRITE_BUSY;
                ssd->channel_head[location->channel].chip_head[location->chip].current_time = ssd->current_time;
                ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time = sub->next_state_predict_time; 
                ssd->channel_head[location->channel].chip_head[location->chip].next_state = CHIP_WRITE_BUSY;
                sub->next_state = SR_W_DATA_TRANSFER;


#ifdef DEBUGSUSPEND
                printf("SR_W_DATA_TRANSFER_ONE_VERIFY_OF_ITERATIONS: sub information: channel %d, chip %d, die %d, plane %d, ppn %d, sub->current_time %lld, sub->next_time %lld, ssd->current_time %lld, ssd->current_state %d, ssd->next_state %d, chip %d  next_predict_time %lld    \n",sub->location->channel, sub->location->chip, sub->location->die, sub->location->plane, sub->ppn,  sub->current_time, sub->next_state_predict_time, ssd->current_time, sub->current_state, sub->next_state,location->chip,    ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time   );
#endif


                break;
            }


            default :
                return ERROR;

        }//switch(aim_state)
    }//if(command==NORMAL)
   else if (command == TWO_PLANE) {
#ifdef DEBUG
            printf("enter TWO_PLANE\n");
#endif

        /**********************************************************************************************
		*高级命令TWO_PLANE的处理，这里的TWO_PLANE高级命令是读子请求的高级命令
		*状态转变与普通命令一样，不同的是在SR_R_C_A_TRANSFER时计算时间是串行的，因为共用一个通道channel
		*还有SR_R_DATA_TRANSFER也是共用一个通道
		**********************************************************************************************/
        if ((sub1 == NULL) || (sub2 == NULL)) {
            return ERROR;
        }
        sub_twoplane_one = sub1;
        sub_twoplane_two = sub2;
        location = sub1->location;

        switch (aim_state) {
            case SR_R_C_A_TRANSFER: {
#ifdef DEBUG
                                printf("enter TWO_PLANE,in SRC_R_C_A_TRANSFER,ssdd->current_time is %llu \n",ssd->current_time);
#endif

                sub_twoplane_one->current_time = ssd->current_time;
                sub_twoplane_one->current_state = SR_R_C_A_TRANSFER;
                sub_twoplane_one->next_state = SR_R_READ;
                sub_twoplane_one->next_state_predict_time =
                        ssd->current_time + 14 * ssd->parameter->time_characteristics.tWC;
                sub_twoplane_one->begin_time = ssd->current_time;

                ssd->channel_head[sub_twoplane_one->location->channel].chip_head[sub_twoplane_one->location->chip].die_head[sub_twoplane_one->location->die].plane_head[sub_twoplane_one->location->plane].add_reg_ppn = sub_twoplane_one->ppn;
                ssd->read_count++;

                sub_twoplane_two->current_time = ssd->current_time;
                sub_twoplane_two->current_state = SR_R_C_A_TRANSFER;
                sub_twoplane_two->next_state = SR_R_READ;
                sub_twoplane_two->next_state_predict_time = sub_twoplane_one->next_state_predict_time;
                sub_twoplane_two->begin_time = ssd->current_time;

                ssd->channel_head[sub_twoplane_two->location->channel].chip_head[sub_twoplane_two->location->chip].die_head[sub_twoplane_two->location->die].plane_head[sub_twoplane_two->location->plane].add_reg_ppn = sub_twoplane_two->ppn;
                ssd->read_count++;
                ssd->m_plane_read_count++;

                ssd->channel_head[location->channel].current_state = CHANNEL_C_A_TRANSFER;
                ssd->channel_head[location->channel].current_time = ssd->current_time;
                ssd->channel_head[location->channel].next_state = CHANNEL_IDLE;
                ssd->channel_head[location->channel].next_state_predict_time =
                        ssd->current_time + 14 * ssd->parameter->time_characteristics.tWC;

                ssd->channel_head[location->channel].chip_head[location->chip].current_state = CHIP_C_A_TRANSFER;
                ssd->channel_head[location->channel].chip_head[location->chip].current_time = ssd->current_time;
                ssd->channel_head[location->channel].chip_head[location->chip].next_state = CHIP_READ_BUSY;
                ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time =
                        ssd->current_time + 14 * ssd->parameter->time_characteristics.tWC;


                break;
            }
            case SR_R_DATA_TRANSFER: {
#ifdef DEBUG
                                printf("enter TWO_PLANE,and ssd->current_time is %llu \n",ssd->current_time);
#endif

                sub_twoplane_one->current_time = ssd->current_time;
                sub_twoplane_one->current_state = SR_R_DATA_TRANSFER;
                sub_twoplane_one->next_state = SR_COMPLETE;
                sub_twoplane_one->next_state_predict_time = ssd->current_time + (sub_twoplane_one->size *
                                                                                 ssd->parameter->subpage_capacity) *
                                                                                ssd->parameter->time_characteristics.tRC;
                sub_twoplane_one->complete_time = sub_twoplane_one->next_state_predict_time;

                sub_twoplane_two->current_time = sub_twoplane_one->next_state_predict_time;
                sub_twoplane_two->current_state = SR_R_DATA_TRANSFER;
                sub_twoplane_two->next_state = SR_COMPLETE;
                sub_twoplane_two->next_state_predict_time = sub_twoplane_two->current_time + (sub_twoplane_two->size *
                                                                                              ssd->parameter->subpage_capacity) *
                                                                                             ssd->parameter->time_characteristics.tRC;
                sub_twoplane_two->complete_time = sub_twoplane_two->next_state_predict_time;

                ssd->channel_head[location->channel].current_state = CHANNEL_DATA_TRANSFER;
                ssd->channel_head[location->channel].current_time = ssd->current_time;
                ssd->channel_head[location->channel].next_state = CHANNEL_IDLE;
                ssd->channel_head[location->channel].next_state_predict_time = sub_twoplane_one->next_state_predict_time;

                ssd->channel_head[location->channel].chip_head[location->chip].current_state = CHIP_DATA_TRANSFER;
                ssd->channel_head[location->channel].chip_head[location->chip].current_time = ssd->current_time;
                ssd->channel_head[location->channel].chip_head[location->chip].next_state = CHIP_IDLE;
                ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time = sub_twoplane_one->next_state_predict_time;

                ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].add_reg_ppn = -1;

                break;
            }
            default :
                return ERROR;
        }//switch(aim_state)
    }//else if(command==TWO_PLANE)
    else if (command == INTERLEAVE) {
#ifdef DEBUG
            printf("enter INTERLEAVE\n");
#endif

        if ((sub1 == NULL) || (sub2 == NULL)) {
            return ERROR;
        }
        sub_interleave_one = sub1;
        sub_interleave_two = sub2;
        location = sub1->location;

        switch (aim_state) {
            case SR_R_C_A_TRANSFER: {
#ifdef DEBUG
                                printf("enter INTERLEAVE,and ssd->current_time is %llu \n",ssd->current_time);
#endif

                sub_interleave_one->current_time = ssd->current_time;
                sub_interleave_one->current_state = SR_R_C_A_TRANSFER;
                sub_interleave_one->next_state = SR_R_READ;
                sub_interleave_one->next_state_predict_time =
                        ssd->current_time + 14 * ssd->parameter->time_characteristics.tWC;
                sub_interleave_one->begin_time = ssd->current_time;

                ssd->channel_head[sub_interleave_one->location->channel].chip_head[sub_interleave_one->location->chip].die_head[sub_interleave_one->location->die].plane_head[sub_interleave_one->location->plane].add_reg_ppn = sub_interleave_one->ppn;
                ssd->read_count++;

                sub_interleave_two->current_time = ssd->current_time;
                sub_interleave_two->current_state = SR_R_C_A_TRANSFER;
                sub_interleave_two->next_state = SR_R_READ;
                sub_interleave_two->next_state_predict_time = sub_interleave_one->next_state_predict_time;
                sub_interleave_two->begin_time = ssd->current_time;

                ssd->channel_head[sub_interleave_two->location->channel].chip_head[sub_interleave_two->location->chip].die_head[sub_interleave_two->location->die].plane_head[sub_interleave_two->location->plane].add_reg_ppn = sub_interleave_two->ppn;
                ssd->read_count++;
                ssd->interleave_read_count++;

                ssd->channel_head[location->channel].current_state = CHANNEL_C_A_TRANSFER;
                ssd->channel_head[location->channel].current_time = ssd->current_time;
                ssd->channel_head[location->channel].next_state = CHANNEL_IDLE;
                ssd->channel_head[location->channel].next_state_predict_time =
                        ssd->current_time + 14 * ssd->parameter->time_characteristics.tWC;

                ssd->channel_head[location->channel].chip_head[location->chip].current_state = CHIP_C_A_TRANSFER;
                ssd->channel_head[location->channel].chip_head[location->chip].current_time = ssd->current_time;
                ssd->channel_head[location->channel].chip_head[location->chip].next_state = CHIP_READ_BUSY;
                ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time =
                        ssd->current_time + 14 * ssd->parameter->time_characteristics.tWC;

                break;

            }
            case SR_R_DATA_TRANSFER: {
#ifdef DEBUG
                                printf("SR_R_DATA_TRANSFER TWO_PLANE,and ssd->current_time is %llu \n",ssd->current_time);
#endif

                sub_interleave_one->current_time = ssd->current_time;
                sub_interleave_one->current_state = SR_R_DATA_TRANSFER;
                sub_interleave_one->next_state = SR_COMPLETE;
                sub_interleave_one->next_state_predict_time = ssd->current_time + (sub_interleave_one->size *
                                                                                   ssd->parameter->subpage_capacity) *
                                                                                  ssd->parameter->time_characteristics.tRC;
                sub_interleave_one->complete_time = sub_interleave_one->next_state_predict_time;

                sub_interleave_two->current_time = sub_interleave_one->next_state_predict_time;
                sub_interleave_two->current_state = SR_R_DATA_TRANSFER;
                sub_interleave_two->next_state = SR_COMPLETE;
                sub_interleave_two->next_state_predict_time = sub_interleave_two->current_time +
                                                              (sub_interleave_two->size *
                                                               ssd->parameter->subpage_capacity) *
                                                              ssd->parameter->time_characteristics.tRC;
                sub_interleave_two->complete_time = sub_interleave_two->next_state_predict_time;

                ssd->channel_head[location->channel].current_state = CHANNEL_DATA_TRANSFER;
                ssd->channel_head[location->channel].current_time = ssd->current_time;
                ssd->channel_head[location->channel].next_state = CHANNEL_IDLE;
                ssd->channel_head[location->channel].next_state_predict_time = sub_interleave_two->next_state_predict_time;

                ssd->channel_head[location->channel].chip_head[location->chip].current_state = CHIP_DATA_TRANSFER;
                ssd->channel_head[location->channel].chip_head[location->chip].current_time = ssd->current_time;
                ssd->channel_head[location->channel].chip_head[location->chip].next_state = CHIP_IDLE;
                ssd->channel_head[location->channel].chip_head[location->chip].next_state_predict_time = sub_interleave_two->next_state_predict_time;

                ssd->channel_head[location->channel].chip_head[location->chip].die_head[location->die].plane_head[location->plane].add_reg_ppn = -1;

                break;
            }
            default :
                return ERROR;
        }//switch(aim_state)
    }//else if(command==INTERLEAVE)
    else {
        printf("\nERROR: Unexpected command !\n");
        return ERROR;
    }

    return SUCCESS;
}

