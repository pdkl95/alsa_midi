#include "alsa_midi_seq.h"

static aMIDI_inline alsa_midi_client_t *client_for(VALUE self)
{
  GET_CLIENT_STRUCT(GET_IV(client));
  return client;
}
#define CLIENT_PTR alsa_midi_client_t *client = client_for(self)

static unsigned int find_caps(VALUE self)  
{
  return NUM2INT(rb_funcall(self, rb_intern("cap_flags"), 0));
}

static VALUE Port_setup(VALUE self)
{
  CLIENT_PTR;
  IV_STR(name);
  int ret = snd_seq_create_simple_port(client->handle, name, find_caps(self),
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

static VALUE PortTX_get_cap_flags(VALUE self)
{
  return INT2NUM(SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ);
}

static VALUE PortRX_get_cap_flags(VALUE self)
{
  return INT2NUM(SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE);
}

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
  snd_seq_get_port_info(client->handle, port_id, pinfo);

  const snd_seq_addr_t *port_addr = snd_seq_port_info_get_addr(pinfo);

  snd_seq_query_subscribe_t *subs;
  snd_seq_query_subscribe_alloca(&subs);
  snd_seq_query_subscribe_set_root(subs, port_addr);
  //snd_seq_query_subscribe_set_type(subs, SUBS_CAP);
  snd_seq_query_subscribe_set_index(subs, 0);

  int count = 0;
  while (snd_seq_query_port_subscribers(client->handle, subs) >= 0) {
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

static VALUE PortTX_send_note(VALUE self)
{
  snd_seq_event_t ev;
  CLIENT_PTR;
  IV_INT(port_id);

  snd_seq_ev_clear(&ev);
  snd_seq_ev_set_source(&ev, port_id);
  //snd_seq_ev_set_broadcast(&ev);
  snd_seq_ev_set_subs(&ev);
  snd_seq_ev_set_direct(&ev);

  snd_seq_ev_set_note(&ev, 0, 64, 127, client->ticks_per_quarter);

  snd_seq_event_output(client->handle, &ev);
  snd_seq_drain_output(client->handle);

  INFO("NOTE!");
  rb_funcall(self, rb_intern("show_status!"), 0);
  return Qtrue;
}

void Init_aMIDI_Port()
{
  FUNC_X(Port, setup, 0);

  GETTER(PortTX, cap_flags);
  GETTER(PortRX, cap_flags);

  FUNC(PortTX, send_note,      0);
  FUNC(Port,   each_connected, 0);
}
