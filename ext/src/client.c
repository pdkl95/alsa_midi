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

static void CWorker_period(client_t *client)
{
  snd_seq_event_t *ev;  
  write(1, ".", 1);
  FIFO_EACH(client->ev_tx, ev) {
    write(1, "#", 1);
    snd_seq_event_output_direct(client->seq, ev);  
    fifo_write(client->ev_free, ev);
  }
}

static void *CWorker_thread(void *param)
{
  int ret;
  ts_t time;
  client_t *client = (client_t *)param;
  client->thread_exit_status = 0;
  
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
  time = add_timespec(time, client->thread_period);
  while(client->thread_running) {
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &time, NULL);
    CWorker_period(client);
    time = add_timespec(time, client->thread_period);
  }

  pthread_exit(&client->thread_exit_status);
  return NULL;
}

static VALUE Client_worker_start(VALUE self)
{
  GET_CLIENT;
  ts_t tv;

  clock_getres(CLOCK_MONOTONIC, &tv);
  VALUE res = PRINTF2("Timer Resolution: %d s, %d ns",
                      INT2NUM(tv.tv_sec), INT2NUM(tv.tv_nsec));
  DEBUG_MSG(self, "debug", res);

  client->thread_period.tv_sec  = 0;
  client->thread_period.tv_nsec = MAX_NSEC / (4);

  if (pthread_attr_init(&client->thread_attr)) {
    rb_raise(aMIDI_TimerError, "Couldn't init pthread_attr object?!");
  }

  DEBUG("Starting RT worker thread...");
  client->thread_running = 1;
  pthread_create(&client->thread, &client->thread_attr,
                 CWorker_thread, client);
  DEBUG("RT worker thread started!");
  return Qtrue;
}

static VALUE Client_worker_stop(VALUE self)
{
  GET_CLIENT;
  DEBUG("Killing RT worker thread...");
  client->thread_running = 0;
  pthread_join(client->thread, NULL);
  DEBUG("RT worker thread stopped!");
  return Qtrue;
}

static VALUE Client_running(VALUE self)
{
  GET_CLIENT;
  if (client->thread_running) {
    return Qtrue;
  } else {
    return Qfalse;
  }
}

static VALUE Client_set_name(VALUE self, VALUE new_name)
{
  GET_CLIENT;
  client->name = StringValuePtr(new_name);
  DEBUG_STR("ALSA client name: \"%s\"", client->name);
  snd_seq_set_client_name(client->seq, client->name);
  return new_name;
}

static VALUE Client_get_name(VALUE self)
{
  GET_CLIENT;
  return client->name ? rb_str_new2(client->name) : Qnil;
}

static VALUE Client_set_ppq(VALUE self, VALUE new_ppq)
{
  GET_CLIENT;
  client->ppq = NUM2INT(new_ppq);
  return new_ppq;
}

static VALUE Client_get_ppq(VALUE self)
{
  GET_CLIENT;
  return INT2NUM(client->ppq);
}

static VALUE Client_set_bpm(VALUE self, VALUE new_bpm)
{
  GET_CLIENT;
  client->bpm = NUM2INT(new_bpm);
  return new_bpm;
}

static VALUE Client_get_bpm(VALUE self)
{
  GET_CLIENT;
  return INT2NUM(client->bpm);
}

static VALUE Client_get_client_id(VALUE self)
{
  GET_CLIENT;
  return INT2NUM(client->client_id);
}

static void Client_free(client_t *client)
{
  snd_seq_event_t *ev;
  FIFO_EACH(client->ev_free, ev) {
    xfree(ev);
  }
  FIFO_EACH(client->ev_tx, ev) {
    xfree(ev);
  }
  fifo_free(client->ev_free);
  fifo_free(client->ev_tx);
  xfree(client);
}

static VALUE Client_alloc(VALUE klass)
{
  int i;
  snd_seq_event_t *ev;
  client_t *client = ALLOC(client_t);
  VALUE self = Data_Wrap_Struct(klass, NULL, Client_free, client);

  client->name    = DEFAULT_CLIENT_NAME;
  client->bpm     = DEFAULT_BPM;
  client->ppq     = DEFAULT_PPQ;
  client->ev_tx   = fifo_alloc(EV_FIFO_SIZE);
  client->ev_free = fifo_alloc(EV_FIFO_SIZE);

  for(i=0; i<(EV_FIFO_SIZE-1); i++) {
    ev = ALLOC(snd_seq_event_t);
    fifo_write_ex(client->ev_free, ev);
  }

  if (snd_seq_open(&(client->seq), "default", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
    rb_raise(aMIDI_AlsaError, "Error opening ALSA sequencer.");
    exit(1);
  }
  client->client_id = snd_seq_client_id(client->seq);

  return self;
}

void Init_aMIDI_Client()
{
  CUSTOM_ALLOC(Client);
  
  FUNC_X(Client, worker_start, 0);
  FUNC_X(Client, worker_stop,  0);

  FUNC_Q(Client, running, 0);

  ACCESSOR(Client, name);
  ACCESSOR(Client, bpm);
  ACCESSOR(Client, ppq);

  GETTER(Client, client_id);
}
