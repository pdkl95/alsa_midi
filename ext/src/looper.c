#include "alsa_midi_seq.h"

#define LOOPER_TYPE_ID(klass, val)         \
  static VALUE klass##_type_id(VALUE self) \
  {                                        \
    return INT2NUM(val);                   \
  }
LOOPER_TYPE_ID(Looper,      LOOPER_UNDEF);
LOOPER_TYPE_ID(LooperSeq16, LOOPER_SEQ16);

static VALUE Looper_set_note(VALUE self, VALUE n, VALUE d)
{
  GET_LOOPER;
  int note      = NUM2INT(n);
  long duration = NUM2LONG(d);
  int i;

  for (i=0; i<looper->seq_len; i++) {
    looper->seq_ev[i].note = note;
    looper->seq_ev[i].velocity = 127;
    looper->seq_ev[i].delay.tv_sec = 0;
    looper->seq_ev[i].delay.tv_sec = duration;
  }
  return self;
}

static VALUE LooperSeq16_get_as_array(VALUE self, VALUE idx)
{
  GET_LOOPER;
  int x = NUM2INT(idx);

  if (looper->seq_ev[x % looper->seq_len].off) {
    return Qfalse;
  } else {
    return Qtrue;
  }
}

static VALUE LooperSeq16_set_as_array(VALUE self, VALUE idx, VALUE newopt)
{
  GET_LOOPER;
  int x = NUM2INT(idx);
  int val = 0;
  if (FALSEY(newopt)) {
    val = 1;
  }
  looper->seq_ev[x % looper->seq_len].off = val;

  if(val) {
    return Qfalse;
  } else {
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
  
  switch(looper->type) {
  case LOOPER_SEQ16:
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

  looper->type = find_type_id(self);
  switch(looper->type) {
  case LOOPER_SEQ16:
    looper->seq_len = 16;
    looper->seq_ev  = ALLOC_N(ev_t, 16);
    for (i=0; i<looper->seq_len; i++) {
      looper->seq_ev[i].type    = EV_NOTE;
      looper->seq_ev[i].mem     = EV_MEM_LOOPER;
      looper->seq_ev[i].port_id = looper->port_id;
      looper->seq_ev[i].channel = looper->channel;
      looper->seq_ev[i].off     = 1;
    }
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

  FUNC(Looper,      type_id, 0);
  FUNC(LooperSeq16, type_id, 0);

  GETARY(LooperSeq16);
  SETARY(LooperSeq16);

  FUNC(Looper, set_note, 2);
}
