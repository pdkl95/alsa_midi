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
static aMIDI_inline ts_t add_timespec(ts_t *a, ts_t b)
{
  ts_t x;
  a->tv_sec  += b.tv_sec;
  a->tv_nsec += b.tv_nsec;
  if (a->tv_nsec >= MAX_NSEC) {
    a->tv_sec  += 1;
    a->tv_nsec -= MAX_NSEC;
  }
  return x;
}

static aMIDI_inline int cmp_timespec(ts_t a, ts_t b)
{
  if      (a.tv_sec > b.tv_sec) return  1;
  else if (a.tv_sec < b.tv_sec) return -1;
  else {
    if      (a.tv_nsec > b.tv_nsec) return  1;
    else if (a.tv_nsec < b.tv_nsec) return -1;
    else return 0;
  }
}

/*************************************************************************
 * Worker Thread
 */
static aMIDI_inline void CWorker_send_note(client_t *client, ev_t *ev,
                                           ev_atomic_t *ev_a, uint8_t is_noteon)
{
  snd_seq_event_t e;
  uint8_t note = midi_note_from_ev_atomic(ev_a, ev->scale);
  uint8_t vel  = ev_a->field.velocity;

  snd_seq_ev_clear(&e);
  snd_seq_ev_set_source(&e, ev->port_id);
  snd_seq_ev_set_subs(&e);
  if (is_noteon) {
    snd_seq_ev_set_noteon(&e, ev->channel, note, vel);
  } else {
    snd_seq_ev_set_noteoff(&e, ev->channel, note, vel);
  }
  snd_seq_ev_set_direct(&e);
  snd_seq_event_output_direct(client->seq, &e);
  //printf("\nNOTE: %d, ch=%d\n", note, ev->channel);
}

static aMIDI_inline void CWorker_schedule_ev(client_t *client, ev_t *ev)
{
  ev->alarm_clock = client->clock_total + ev->atomic.field.duration;
#if 0
  printf("SCHED: alarm_clock = %ld\n(now): clock_total =%ld\ndur = %d",
         ev->alarm_clock, client->clock_total, ev->atomic.field.duration);
#endif

  if (client->thread_delay_pool) {
    client->thread_delay_pool->prev = ev;
  }
  ev->next = client->thread_delay_pool;
  ev->prev = NULL;
  client->thread_delay_pool = ev;
}

static aMIDI_inline void CWorker_unschedule_ev(client_t *client, ev_t *ev)
{
  ev->alarm_clock = 0;

  if (ev->prev) {
    ev->prev->next = ev->next;
  } else {
    client->thread_delay_pool = ev->next;
  }
  if (ev->next) {
    ev->next->prev = ev->prev;
  }
}

static aMIDI_inline void CWorker_return_ev(client_t *client, ev_t *ev)
{
  if (ev->mem == EV_MEM_FIFO) {
    fifo_write(client->ev_free, ev);
  }
}

static void CWorker_process_ev(client_t *client, ev_t *ev)
{
  ev_atomic_t x;
  x.raw = ev->atomic.raw;

  if (EVa_IS_NOTE(x)) {
    if (ev->alarm_clock) {
      CWorker_send_note(client, ev, &x, 0);
      CWorker_unschedule_ev(client, ev);
      CWorker_return_ev(client, ev);
    } else {
      CWorker_send_note(client, ev, &x, 1);
      CWorker_schedule_ev(client, ev);
    }

  } else if (EVa_IS_NOTEON(x)) {
    CWorker_send_note(client, ev, &x, 1);
    CWorker_return_ev(client, ev);

  } else if (EVa_IS_NOTEOFF(x)) {
    CWorker_send_note(client, ev, &x, 0);
    CWorker_return_ev(client, ev);

  } else { // noop - unknown event type
    CWorker_return_ev(client, ev);
  }
}

static void CWorker_process_looper(client_t *client, looper_t *looper)
{
  int i;
  ev_t *ev;
  ev_atomic_t x;

  switch(looper->type) {
  case LOOPER_SEQ:
  case LOOPER_SEQ_MONO:
    ev = LOOPER_EV_AT(looper, client->beat_total);
    x.raw = ev->atomic.raw;

    if (client->clock == 0) {
      //write(1, "N", 1);
      if (EVa_ACTIVE(&x)) {
        CWorker_send_note(client, ev, &x, 1);
        ev->alarm_clock = client->clock_total + x.field.duration;
      }
    }
    if (ev->alarm_clock && (client->clock_total == ev->alarm_clock)) {
      //write(1, "X", 1);
      ev->alarm_clock = 0;
      CWorker_send_note(client, ev, &x, 0);
    }
    break;
  }
}

static void *CWorker_thread(void *param)
{
  int ret;
  ev_t *ev;
  looper_t *loop;
  ts_t time_next, time_now;
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

  clock_gettime(GETTIME_CLOCK, &time_next);
  while(client->thread_running) {
    clock_gettime(GETTIME_CLOCK, &time_now);
    add_timespec(&time_next, client->thread_period);

    if (cmp_timespec(time_now, time_next) > 0) {
      fprintf(stderr, RT_WRK "*** RT Deadline miss! ***\n");
      fprintf(stderr, RT_WRK "      now: %ld s, %ld ns\n",
              time_now.tv_sec, time_now.tv_nsec);
      fprintf(stderr, RT_WRK "     next: %ld s, %ld ns\n",
              time_next.tv_sec, time_next.tv_nsec);
    }

    clock_nanosleep(NSLEEP_CLOCK, TIMER_ABSTIME, &time_next, NULL);

    /* process async events from the TX FIFO */
    //write(1, ".", 1);
    FIFO_FLUSH(client->ev_tx, ev) {
      //write(1, "#", 1);
      ev->alarm_clock = 0;
      CWorker_process_ev(client, ev);
    }

    /* process delayed events */
    ev = client->thread_delay_pool;
    while(ev) {
      //write(1, ",", 1);
      if (ev->alarm_clock) {
        if (ev->alarm_clock == client->clock_total) {
          //write(1, "A", 1);
          CWorker_process_ev(client, ev);
        }
      } else {
        fprintf(stderr, RT_WRK
                "Alarm scheduled event, but no alarm_clock value?!");
        CWorker_unschedule_ev(client, ev);
      }
      ev = ev->next;
    }

    /* process all looper widgets */
    loop = client->looper_widgets;
    while(loop) {
      //write(1,"L",1);
      CWorker_process_looper(client, loop);
      loop = loop->next;
    }

    /* advance the song pointers */
    client->clock++;
    client->clock_total++;
    if (client->clock >= client->clocks_per_beat) {
      client->clock = 0;
      client->beat++;
      client->beat_total++;
      if (client->beat >= client->beats_per_measure) {
        client->beat = 0;
        client->measure++;
        //write(1,"M",1);
      } else {
        //write(1,"b",1);
      }
    }
  }

  pthread_exit(&client->thread_exit_status);
  return NULL;
}

/******************************************************************
 * ruby Client interface
 */
static VALUE Client_worker_start(VALUE self)
{
  GET_CLIENT;
  ts_t tv;

  clock_getres(CLOCK_MONOTONIC, &tv);
  VALUE res = PRINTF2("Timer Resolution: %d s, %d ns",
                      INT2NUM(tv.tv_sec), INT2NUM(tv.tv_nsec));
  DEBUG_MSG(self, "debug", res);

  if (pthread_attr_init(&client->thread_attr)) {
    rb_raise(aMIDI_CWorkerError, "Couldn't init pthread_attr object?!");
  }

  DEBUG("Starting RT worker thread...");
  client->thread_running = 1;
  if (pthread_create(&client->thread, &client->thread_attr,
                     CWorker_thread, client)) {
    rb_raise(aMIDI_CWorkerError, "Couldn't spawn realtime thread!");
  }
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
  if (snd_seq_set_client_name(client->seq, client->name)) {
    rb_raise(aMIDI_AlsaError, "Couldn't set client name!");
  }
  return new_name;
}

static VALUE Client_get_name(VALUE self)
{
  GET_CLIENT;
  return client->name ? rb_str_new2(client->name) : Qnil;
}

STD_INT_ACCESSOR(Client, client_t, clocks_per_beat);
STD_INT_ACCESSOR(Client, client_t, beats_per_measure);

STD_INT_GETTER(Client, client_t, clock);
STD_INT_GETTER(Client, client_t, beat);
STD_INT_GETTER(Client, client_t, measure);
STD_INT_GETTER(Client, client_t, bpm);

static void set_tempo(client_t *client, int new_bpm)
{
  client->bpm = new_bpm;
  client->clocks_per_minute = client->clocks_per_beat * client->bpm;
  client->clocks_per_second = (float)(client->clocks_per_minute) / 60.0f;

  client->thread_period.tv_sec = 0;
  client->thread_period.tv_nsec = MAX_NSEC / (long)(client->clocks_per_second);

#if 0
  printf("NEW TEMPO:\n");
  printf(" bpm = %d\n",  client->bpm);
  printf(" cpb = %d\n",  client->clocks_per_beat);
  printf(" cpm = %d\n",  client->clocks_per_minute);
  printf(" cps = %f\n",  client->clocks_per_second);
  printf("nsec = %ld\n", client->thread_period.tv_nsec);
#endif
}

static VALUE Client_set_bpm(VALUE self, VALUE new_bpm)
{
  GET_CLIENT;
  set_tempo(client, NUM2INT(new_bpm));
  return new_bpm;
}


static VALUE Client_get_client_id(VALUE self)
{
  GET_CLIENT;
  return INT2NUM(client->client_id);
}

static void Client_free_pre(client_t *client)
{
  ev_t *ev;

  FIFO_FLUSH(client->ev_return, ev) { xfree(ev); }
  FIFO_FLUSH(client->ev_free,   ev) { xfree(ev); }
  FIFO_FLUSH(client->ev_tx,     ev) { xfree(ev); }

  fifo_free(client->ev_return);
  fifo_free(client->ev_free);
  fifo_free(client->ev_tx);
}

static void Client_new_preinit(VALUE self, client_t *client)
{
  int i;
  ev_t *ev;

  client->name              = DEFAULT_CLIENT_NAME;
  client->clocks_per_beat   = DEFAULT_CLOCKS_PER_BEAT;
  client->beats_per_measure = DEFAULT_BEATS_PER_MEASURE;

  client->ev_tx     = fifo_alloc(EV_FIFO_SIZE);
  client->ev_free   = fifo_alloc(EV_FIFO_SIZE);
  client->ev_return = fifo_alloc(EV_FIFO_SIZE);

  client->clock   = 0;
  client->beat    = 0;
  client->measure = 0;

  client->clock_total = 0;
  client->beat_total  = 0;

  client->thread_delay_pool = NULL;
  client->looper_widgets    = NULL;

  for(i=0; i<(EV_FIFO_SIZE-1); i++) {
    ev = ALLOC(ev_t);
    fifo_write_ex(client->ev_free, ev);
  }

  if (snd_seq_open(&(client->seq), "default", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
    rb_raise(aMIDI_AlsaError, "Error opening ALSA sequencer.");
    exit(1);
  }
  client->client_id = snd_seq_client_id(client->seq);

  set_tempo(client, DEFAULT_TEMPO);
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
  ACCESSOR(Client, clocks_per_beat);
  ACCESSOR(Client, beats_per_measure);
  ACCESSOR(Client, bpm);

  GETTER(Client, clock);
  GETTER(Client, beat);
  GETTER(Client, measure);
  GETTER(Client, client_id);
}
