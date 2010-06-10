#include "alsa_midi_seq.h"

#ifdef HAVE_TIME_H
#include <time.h>
#else
#error "Must have time.h for clock_nanosleep()!"
#endif

#ifdef HAVE_SCHED_H
#include <sched.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#define MAX_NSEC 1000000000L
static aMIDI_inline ts_t add_timespec(ts_t a, ts_t b)
{
  ts_t x;
  x.tv_sec  = a.tv_sec  + b.tv_sec;
  x.tv_nsec = a.tv_nsec + b.tv_nsec ;
  if (x.tv_nsec >= MAX_NSEC) {
    x.tv_sec  += 1;
    x.tv_nsec = x.tv_nsec - MAX_NSEC;
  }
  return x;
}

static aMIDI_inline void CWorker_send_note(client_t *client, ev_t *ev)
{
  snd_seq_event_t e;
  snd_seq_ev_clear(&e);
  snd_seq_ev_set_source(&e, ev->port_id);
  snd_seq_ev_set_subs(&e);
  snd_seq_ev_set_noteon(&e, ev->channel, midi_note_from_ev(ev), ev->velocity);
  snd_seq_ev_set_direct(&e);
  snd_seq_event_output_direct(client->seq, &e);
}

static aMIDI_inline void CWorker_process_ev(client_t *client, ev_t *ev)
{
  switch(ev->type) {
  case EV_NOTE:
    CWorker_send_note(client, ev);
    break;
  default:
    break;
  }
  fifo_write(client->ev_free, ev);
}

static aMIDI_inline void CWorker_period(client_t *client)
{
  ev_t *ev;  
  //write(1, ".", 1);
  FIFO_FLUSH(client->ev_tx, ev) {
    //write(1, "#", 1);
    CWorker_process_ev(client, ev);
  }
}

static void *CWorker_thread(void *param)
{
  int ret;
  ts_t time;
  client_t *client = (client_t *)param;
  client->thread_exit_status = 0;
  
#ifdef USE_SETSCHEDULER
  // Set realtime priority for this thread
  if(!geteuid()) {
    struct sched_param sp;
    memset(&sp, 0, sizeof(sp));
    sp.sched_priority = sched_get_priority_max(SCHED_FIFO);
    if (sched_setscheduler(0, SCHED_FIFO, &sp) < 0) {
      perror(RT_WRK "sched_setscheduler(SCHED_FIFO)");
      fprintf(stderr, RT_WRK "using normal (non-realtime) priority!\n");
    }
  } else {
    fprintf(stderr, RT_WRK "Not root! Skipping realtime priority scheduler!\n");
  }
#else
# warning "Disabling sched_setscheduler() because we are missing features!"
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

STD_INT_ACCESSOR(Client, client_t, ppq);
STD_INT_ACCESSOR(Client, client_t, bpm);

static VALUE Client_get_client_id(VALUE self)
{
  GET_CLIENT;
  return INT2NUM(client->client_id);
}

static void Client_free_pre(client_t *client)
{
  ev_t *ev;
  FIFO_FLUSH(client->ev_free, ev) {
    xfree(ev);
  }
  FIFO_FLUSH(client->ev_tx, ev) {
    xfree(ev);
  }
  fifo_free(client->ev_free);
  fifo_free(client->ev_tx);
}

static void Client_new_preinit(VALUE self, client_t *client)
{
  int i;
  ev_t *ev;

  client->name    = DEFAULT_CLIENT_NAME;
  client->bpm     = DEFAULT_BPM;
  client->ppq     = DEFAULT_PPQ;
  client->ev_tx   = fifo_alloc(EV_FIFO_SIZE);
  client->ev_free = fifo_alloc(EV_FIFO_SIZE);

  for(i=0; i<(EV_FIFO_SIZE-1); i++) {
    ev = ALLOC(ev_t);
    fifo_write_ex(client->ev_free, ev);
  }

  if (snd_seq_open(&(client->seq), "default", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
    rb_raise(aMIDI_AlsaError, "Error opening ALSA sequencer.");
    exit(1);
  }
  client->client_id = snd_seq_client_id(client->seq);
}

static void Client_new_postinit(VALUE self, client_t *client)
{
  rb_funcall(self, rb_intern("worker_start!"), 0);
}

STD_ALLOC_SETUP(Client, client_t);

void Init_aMIDI_Client()
{
  CLASS_NEW(Client);
  
  FUNC_X(Client, worker_start, 0);
  FUNC_X(Client, worker_stop,  0);

  FUNC_Q(Client, running, 0);

  ACCESSOR(Client, name);
  ACCESSOR(Client, bpm);
  ACCESSOR(Client, ppq);

  GETTER(Client, client_id);
}
