#include "alsa_midi_seq.h"

#define LOOPER_TYPE_ID(klass, val)         \
  static VALUE klass##_type_id(VALUE self) \
  {                                        \
    return INT2NUM(val);                   \
  }
LOOPER_TYPE_ID(Looper,        LOOPER_UNDEF);
LOOPER_TYPE_ID(LooperSeq,     LOOPER_SEQ);
LOOPER_TYPE_ID(LooperSeqMono, LOOPER_SEQ_MONO);

static VALUE Looper_set_note(VALUE self, VALUE n, VALUE d)
{
  GET_LOOPER;
  long duration = NUM2LONG(d);
  int i;
  ev_atomic_t x;
  x.field.type     = EV_TYPE_NOTE;
  x.field.flags    = 0;
  x.field.note     = NUM2INT(n);
  x.field.velocity = 127;
  x.field.duration = looper->client->clocks_per_beat / 4;

  for (i=0; i<looper->seq_len; i++) {
    looper->seq_ev[i].atomic.raw = x.raw;
  }
  return self;
}

static VALUE LooperSeq_get_as_array(VALUE self, VALUE idx)
{
  GET_LOOPER;
  int    x = NUM2INT(idx);
  ev_t *ev = LOOPER_EV_AT(looper, x);

  if (EV_ACTIVE(ev)) {
    return Qfalse;
  } else {
    return Qtrue;
  }
}

static VALUE LooperSeq_set_as_array(VALUE self, VALUE idx, VALUE newopt)
{
  GET_LOOPER;
  int    x = NUM2INT(idx);
  ev_t *ev = LOOPER_EV_AT(looper, x);

  if (FALSEY(newopt)) {
    ev_on(ev);
    return Qfalse;
  } else {
    ev_off(ev);
    return Qtrue;
  }
}

static void Looper_free_pre(looper_t *looper)
{
  if (looper->prev) {
    looper->prev->next = looper->next;
  } else {
    looper->client->looper_widgets = looper->next;
  }
  if (looper->next) {
    looper->next->prev = looper->prev;
  }
  
  if (looper->seq_ev) {
    xfree(looper->seq_ev);
  }
}

static int find_type_id(VALUE self)
{
  return NUM2INT(rb_funcall(self, rb_intern("type_id"), 0));
}

static void Looper_new_postinit(VALUE self, looper_t *looper)
{
  int i;
  GET_CLIENT_STRUCT(GET_IV(client));
  IV_INT(port_id);
  IV_INT(channel);

  looper->client  = client;
  looper->port_id = port_id;
  looper->channel = channel;
  looper->scale   = NULL;

  looper->type = find_type_id(self);
  switch(looper->type) {
  case LOOPER_SEQ:
  case LOOPER_SEQ_MONO:
    looper->seq_len = 16;
    looper->seq_ev  = ALLOC_N(ev_t, looper->seq_len);
    for (i=0; i<looper->seq_len; i++) {
      looper->seq_ev[i].mem     = EV_MEM_LOOP;
      looper->seq_ev[i].port_id = looper->port_id;
      looper->seq_ev[i].channel = looper->channel;
    }
    break;
  default:
    looper->seq_len = 0;
    looper->seq_ev = NULL;
    break;
  }
  
  looper->prev = NULL;
  looper->next = client->looper_widgets;
  client->looper_widgets = looper;
}

STD_FREE_CALLBACKS(Looper, looper_t);
STD_NEW_POSTINIT(Looper, looper_t);

void Init_aMIDI_Looper()
{
  CLASS_NEW(Looper);

  FUNC(Looper,        type_id, 0);
  FUNC(LooperSeq,     type_id, 0);
  FUNC(LooperSeqMono, type_id, 0);

  GETARY(LooperSeq);
  SETARY(LooperSeq);

  FUNC(Looper, set_note, 2);
}
