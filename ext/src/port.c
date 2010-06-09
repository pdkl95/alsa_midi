#include "alsa_midi_seq.h"

static aMIDI_inline client_t *client_for(VALUE self)
{
  GET_CLIENT_STRUCT(GET_IV(client));
  return client;
}
#define CLIENT_PTR client_t *client = client_for(self)

static unsigned int find_caps(VALUE self)  
{
  return NUM2INT(rb_funcall(self, rb_intern("cap_flags"), 0));
}

static VALUE Port_setup(VALUE self)
{
  CLIENT_PTR;
  IV_STR(name);
  int ret = snd_seq_create_simple_port(client->seq, name, find_caps(self),
                                       SND_SEQ_PORT_TYPE_MIDI_GENERIC |
                                       SND_SEQ_PORT_TYPE_APPLICATION);
  int retval = INT2NUM(ret);
  //DEBUG_NUM("@port_id = %d", ret);

  if (ret < 0) {
    rb_raise(aMIDI_AlsaError, "Error creating sequencer READ port.");
    return Qnil;
  }
  SET_IV(port_id, retval);
  return retval;
}

#define CAP_FLAG(klass, type)                       \
  static VALUE klass##_get_cap_flags(VALUE self)    \
  {                                                 \
    return INT2NUM( SND_SEQ_PORT_CAP_##type |       \
                    SND_SEQ_PORT_CAP_SUBS_##type);  \
  }
CAP_FLAG(PortTX, READ)
CAP_FLAG(PortRX, WRITE)

static VALUE Port_each_connected(VALUE self)
{
  VALUE proc;
  ID sym_call = rb_intern("call");
  CLIENT_PTR;
  IV_INT(port_id);

  if (!rb_block_given_p()) {
    rb_raise(aMIDI_AlsaError, "Error creating sequencer READ port.");
    return Qnil;
  }
  proc = rb_block_proc();

  snd_seq_port_info_t *pinfo;
  snd_seq_port_info_alloca(&pinfo);
  snd_seq_get_port_info(client->seq, port_id, pinfo);

  const snd_seq_addr_t *port_addr = snd_seq_port_info_get_addr(pinfo);

  snd_seq_query_subscribe_t *subs;
  snd_seq_query_subscribe_alloca(&subs);
  snd_seq_query_subscribe_set_root(subs, port_addr);
  //snd_seq_query_subscribe_set_type(subs, SUBS_CAP);
  snd_seq_query_subscribe_set_index(subs, 0);

  int count = 0;
  while (snd_seq_query_port_subscribers(client->seq, subs) >= 0) {
    const int idx = snd_seq_query_subscribe_get_index(subs);
    const snd_seq_addr_t *addr;
    addr = snd_seq_query_subscribe_get_addr(subs);
    snd_seq_query_subscribe_set_index(subs, idx + 1);
    rb_funcall(proc, sym_call, 3,
               INT2NUM(addr->client),
               INT2NUM(addr->port),
               INT2NUM(count));
    count += 1;
  }
  return INT2NUM(count);
}

static snd_seq_event_t *get_ev(client_t *client, int port_id)
{
  snd_seq_event_t *ev = fifo_read(client->ev_free);
  if (ev == NULL) {
    rb_raise(aMIDI_Error, "No free snd_seq_event_t objects on the FIFO!");
    return NULL;
  }
  
  snd_seq_ev_clear(ev);
  snd_seq_ev_set_source(ev, port_id);
  snd_seq_ev_set_subs(ev);

  return ev;
}

static void send_ev(client_t *client, snd_seq_event_t *ev)
{
  snd_seq_ev_set_direct(ev);
  fifo_write(client->ev_tx, ev);
}

static VALUE PortTX_note_on(VALUE self, VALUE ch, VALUE note, VALUE vel)
{
  CLIENT_PTR;
  IV_INT(port_id);

  snd_seq_event_t *ev = get_ev(client, port_id);
  snd_seq_ev_set_noteon(ev, NUM2INT(ch), NUM2INT(note), NUM2INT(vel));
  send_ev(client, ev);

  return self;
}

static VALUE PortTX_note_off(VALUE self, VALUE ch, VALUE note, VALUE vel)
{
  CLIENT_PTR;
  IV_INT(port_id);

  snd_seq_event_t *ev = get_ev(client, port_id);
  snd_seq_ev_set_noteoff(ev, NUM2INT(ch), NUM2INT(note), NUM2INT(vel));
  send_ev(client, ev);

  return self;
}

static VALUE PortTX_note(VALUE self, VALUE ch, VALUE note, VALUE vel)
{
  PortTX_note_on(self, ch, note, vel);
  PortTX_note_off(self, ch, note, vel);
  return self;
}

void Init_aMIDI_Port()
{
  FUNC_X(Port, setup, 0);

  GETTER(PortTX, cap_flags);
  GETTER(PortRX, cap_flags);

  FUNC_X(PortTX, note_on,  3);
  FUNC_X(PortTX, note_off, 3);
  FUNC_X(PortTX, note,     3);

  FUNC(Port,   each_connected, 0);
}
