/*****************************************************************************************************************************
This project was supported by the National Basic Research 973 Program of China under Grant No.2011CB302301
Huazhong University of Science and Technology (HUST)   Wuhan National Laboratory for Optoelectronics

FileName： ssd.h
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
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include "initialize.h"
#include "flash.h"
#include "pagemap.h"

#define MAX_INT64  0x7fffffffffffffffll

//lxcv1
//#define DEBUGONLYGC //open for debug
//#define DEBUG //open for debug
//#define DEBUGQUEUE //open for monitering queue length and head state.
//#define DEBUGSUSPEND //open for suspend debug
//#define DEBUGTRACE_OUT //open for trace_out function debug
//#define DEBUG_CURR_TIME //open for debug for ssd_current_time



#define ERASE_ITERATIONS  2 //for erase iteration


void printfsub_request_status(struct ssd_info *ssd);
void printf_ssd_request_queue(struct ssd_info *ssd);
//lxcprogram
void printf_ssd_gc_node(struct ssd_info *ssd);
//lxcprogram_gc
int judging_read_in_gc_chip(struct ssd_info *ssd, int channel);
int printf_every_chip_read_subrequest(struct ssd_info *ssd);
int printf_every_chip_static_subrequest(struct ssd_info *ssd);
//lxcprogram_de-prioritize
int calc_req_slack_time(struct request * req);
//lxcprogram_GC
int printf_gc_node_information(struct gc_operation* gc_node);
    


//lxcprogram_for_adjust_write_suspension_number
int statistic_write_request_number(struct ssd_info *ssd);
int no_greedy_RF_suspension(struct ssd_info *ssd);





struct ssd_info *simulate(struct ssd_info *);

int get_requests(struct ssd_info *);

struct ssd_info *buffer_management(struct ssd_info *);

unsigned int lpn2ppn(struct ssd_info *, unsigned int lsn);

struct ssd_info *distribute(struct ssd_info *);

void trace_output(struct ssd_info *);

void statistic_output(struct ssd_info *);

unsigned int size(unsigned int);

unsigned int transfer_size(struct ssd_info *, int, unsigned int, struct request *);

int64_t find_nearest_event(struct ssd_info *);

void free_all_node(struct ssd_info *);

struct ssd_info *make_aged(struct ssd_info *);

struct ssd_info *no_buffer_distribute(struct ssd_info *);
