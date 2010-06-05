#ifndef RT_H
#define RT_H

#include <pthread.h>

typedef struct timespec ts_t;

struct rt_worker {
  pthread_t      thread;
  pthread_attr_t attr;

  int running;
  int exit_status;

  ts_t period;
  
};
typedef struct rt_worker rt_t;

#define GET_RT_STRUCT(obj)         \
  rt_t *rt;                        \
  Data_Get_Struct(obj, rt_t, rt);

#define GET_RT GET_RT_STRUCT(self)


extern void Init_aMIDI_RT();

#endif /*RT_H*/
