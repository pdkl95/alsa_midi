#include "alsa_midi_seq.h"

#include <time.h>

#ifdef HAVE_SCHED_H
#include <sched.h>
#endif

#define MAX_NSEC 1000000000L
static aMIDI_inline ts_t add_timespec(ts_t a, ts_t b) {
  ts_t x;
  x.tv_sec  = a.tv_sec  + b.tv_sec;
  x.tv_nsec = a.tv_nsec + b.tv_nsec ;
  if (x.tv_nsec >= MAX_NSEC) {
    x.tv_sec  += 1;
    x.tv_nsec = x.tv_nsec - MAX_NSEC;
  }
  return x;
}

static void RTworker_period(rt_t *rt)
{
  snd_seq_event_t *ev;  
  //write(1, ".", 1);
  FIFO_EACH(&rt->midi_tx_fifo, ev) {
    //write(1, "#", 1);
    snd_seq_event_output_direct(rt->client_handle, ev);  
    fifo_write(&rt->free_ev_fifo, ev);
  }
}

static void *RTworker_thread(void *param)
{
  int ret;
  ts_t time;
  rt_t *rt = (rt_t *)param;
  rt->exit_status = 0;
  
#ifdef HAVE_SCHED_SETSCHEDULER
#ifdef HAVE_SCHED_GET_PRIORITY_MAX
  // Set realtime priority for this thread
  struct sched_param sp;
  sp.sched_priority = sched_get_priority_max(SCHED_RR);
  if (sched_setscheduler(0, SCHED_RR, &sp) < 0) {
    perror("sched_setscheduler(SCHED_RR)");
  }
#endif
#endif   

  clock_gettime(CLOCK_MONOTONIC, &time);
  time = add_timespec(time, rt->period);
  while(rt->running) {
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &time, NULL);
    RTworker_period(rt);
    time = add_timespec(time, rt->period);
  }

  pthread_exit(&(rt->exit_status));
  return NULL;
}

static VALUE RT_setup(VALUE self)
{
  GET_RT;
  IV_STR(rtc);
  struct timespec tv;

  clock_getres(CLOCK_MONOTONIC, &tv);
  VALUE res = PRINTF2("Timer Resolution: %d s, %d ns",
                      INT2NUM(tv.tv_sec), INT2NUM(tv.tv_nsec));
  DEBUG_MSG(self, "debug", res);

  rt->period.tv_sec  = 0;
  rt->period.tv_nsec = MAX_NSEC / (128);

  if (pthread_attr_init(&(rt->attr))) {
    rb_raise(aMIDI_TimerError, "Couldn't init pthread_attr object?!");
  }

  
  /*if (pthread_attr_setstacksize(&(rt->attr), PTHREAD_STACK_MIN + MY_STACK_SIZE)) {
    error(2);
    }*/

 
  return Qtrue;
}

static VALUE RT_start(VALUE self)
{
  GET_RT;
  DEBUG("Starting RT thread...");
  rt->running = 1;
  pthread_create(&(rt->thread), &(rt->attr), RTworker_thread, rt);
  DEBUG("RT thread started!");
  return Qtrue;
}

static VALUE RT_stop(VALUE self)
{
  GET_RT;
  DEBUG("Killing RT thread...");
  rt->running = 0;
  pthread_join(rt->thread, NULL);
  DEBUG("RT thread stopped!");
  return Qtrue;
}

static VALUE RT_running(VALUE self)
{
  GET_RT;
  if (rt->running) {
    return Qtrue;
  } else {
    return Qfalse;
  }
}

static VALUE RT_send_midi(VALUE self)
{
  GET_RT;
  fifo_write(&rt->midi_tx_fifo, (void *)3);
}

static void RT_free(rt_t *rt)
{
  snd_seq_event_t *ev;
  FIFO_EACH(&rt->free_ev_fifo, ev) {
    xfree(ev);
  }
  FIFO_EACH(&rt->midi_tx_fifo, ev) {
    xfree(ev);
  }
  fifo_cleanup(&rt->free_ev_fifo);
  fifo_cleanup(&rt->midi_tx_fifo);

  free(rt);
}

static VALUE RT_alloc(VALUE klass)
{
  rt_t *rt;
  VALUE self = Data_Make_Struct(klass, rt_t, NULL, RT_free, rt);

  MIDI_EV_FIFO(rt->midi_tx_fifo);
  MIDI_EV_FIFO(rt->free_ev_fifo);

  int i;
  for(i=0; i<(MIDI_EV_FIFO_SIZE-1); i++) {
    snd_seq_event_t *ev = ALLOC(snd_seq_event_t);
    fifo_write_ex(&rt->free_ev_fifo, ev);
  }

  return self;
}

void Init_aMIDI_RT()
{
  CUSTOM_ALLOC(RT);
 
  FUNC_X(RT, setup, 0);
  FUNC_X(RT, start, 0);
  FUNC_X(RT, stop,  0);

  FUNC(RT, send_midi, 0);

  FUNC_Q(RT, running, 0);
}
