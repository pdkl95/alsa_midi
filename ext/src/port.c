#include "alsa_midi_seq.h"

static aMIDI_inline int check_channel_number(uint8_t ch)
{
  if (ch < 1 || ch > 16) {
    rb_raise(aMIDI_ParamError,
             "MIDI channel must be 1-16 (got %d)", ch);
    return 1;
  } else {
    return 0;
  }
}

static unsigned int find_caps(VALUE self)
{
  return NUM2INT(rb_funcall(self, rb_intern("cap_flags"), 0));
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
  GET_PORT;
  VALUE proc;
  ID sym_call = rb_intern("call");

  if (!rb_block_given_p()) {
    rb_raise(aMIDI_ParamError, "each_connected requires a block!");
    return Qnil;
  }
  proc = rb_block_proc();

  snd_seq_port_info_t *pinfo;
  snd_seq_port_info_alloca(&pinfo);
  snd_seq_get_port_info(port->client->seq, port->port_id, pinfo);

  const snd_seq_addr_t *port_addr = snd_seq_port_info_get_addr(pinfo);

  snd_seq_query_subscribe_t *subs;
  snd_seq_query_subscribe_alloca(&subs);
  snd_seq_query_subscribe_set_root(subs, port_addr);
  //snd_seq_query_subscribe_set_type(subs, SUBS_CAP);
  snd_seq_query_subscribe_set_index(subs, 0);

  int count = 0;
  while (snd_seq_query_port_subscribers(port->client->seq, subs) >= 0) {
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

static ev_t *get_ev(port_t *port, int ch)
{
  ev_t *ev = fifo_read(port->client->ev_free);
  if (ev == NULL) {
    rb_raise(aMIDI_Error, "No free snd_seq_event_t objects on the FIFO!");
    return NULL;
  }
  
  ev->mem     = EV_MEM_FIFO;
  ev->port_id = port->port_id;
  ev->channel = ch;

  return ev;
}

static aMIDI_inline void send_ev(port_t *port, ev_t *ev)
{
  fifo_write(port->client->ev_tx, ev);
}

static aMIDI_inline void send_eva(port_t *port, uint8_t ch, ev_atomic_t *ev_a)
{
  check_channel_number(ch);

  ev_t *ev = get_ev(port, ch-1);
  ev->atomic.raw = ev_a->raw;

  send_ev(port, ev);
}

static VALUE send_note_common(VALUE self, VALUE ch, VALUE note, VALUE vel,
                              VALUE dur, uint8_t ev_type)
{
  GET_PORT;
  int channel  = NUM2INT(ch);
  int noteval  = NUM2INT(note);
  int velocity = NUM2INT(vel);
  int duration = dur == Qnil ? 0 : NUM2INT(dur);

#if 0
  char *n_type;
  VALUE s;

  switch(ev_type) {
  case EV_TYPE_NOTE:    n_type = "ON+OFF";   break;
  case EV_TYPE_NOTEON:  n_type = "ON";       break;
  case EV_TYPE_NOTEOFF: n_type = "OFF";      break;
  default:              n_type = "Unknown!"; break;
  }
  s = PRINTF5("NOTE<%s> ch=%d, note=%d, vel=%d, dur=%d",
              rb_str_new2(n_type),
              INT2NUM(channel),  INT2NUM(noteval),
              INT2NUM(velocity), INT2NUM(duration));
  DEBUG_MSG(self, "info", s);
#endif

  ev_atomic_t ev_a;
  ev_a.field.type     = ev_type;
  ev_a.field.note     = noteval;
  ev_a.field.velocity = velocity;
  ev_a.field.flags    = EV_FLAG_ACTIVE;
  ev_a.field.duration = duration;

  send_eva(port, channel, &ev_a);
  return self;
}

static VALUE PortTX_note_on(VALUE self, VALUE ch, VALUE note, VALUE vel)
{
  return send_note_common(self, ch, note, vel, Qnil, EV_TYPE_NOTEON);
}

static VALUE PortTX_note_off(VALUE self, VALUE ch, VALUE note, VALUE vel)
{
  return send_note_common(self, ch, note, vel, Qnil, EV_TYPE_NOTEOFF);
}

static VALUE PortTX_note(VALUE self, VALUE ch, VALUE note, VALUE vel, VALUE dur)
{
  return send_note_common(self, ch, note, vel, dur, EV_TYPE_NOTE);
}

static VALUE Port_create_seq_mono(VALUE self, VALUE channel, VALUE num_steps)
{
  int ch  = NUM2INT(channel);
  int len = NUM2INT(num_steps);

  if (check_channel_number(ch)) {
    return Qnil;
  }

  if (len < 1 || len > LOOPER_SEQ_MAX_STEPS) {
    PARAM_ERR_RETURN("num_steps must be 1-" Q(LOOPER_SEQ_MAX_STEPS));
  }
}

STD_FREE(Port, port_t);

static VALUE Port_new(VALUE klass, VALUE c, VALUE name)
{
  port_t *port = ALLOC(port_t);
  VALUE self = Data_Wrap_Struct(klass, NULL, Port_free, port);

  GET_CLIENT_STRUCT(c);
  port->client = client;

  SET_IV(client, c);
  SET_IV(requested_port_name, name);

  name = rb_funcall(self, rb_intern("port_name"), 0);

  port->port_id = snd_seq_create_simple_port(port->client->seq, 
                                             StringValuePtr(name),
                                             find_caps(self),
                                             SND_SEQ_PORT_TYPE_MIDI_GENERIC |
                                             SND_SEQ_PORT_TYPE_APPLICATION);
  //DEBUG_NUM("@port_id = %d", port->port_id);

  if (port->port_id < 0) {
    rb_raise(aMIDI_AlsaError, "Error creating ALSA sequencer port.");
  }

  SET_IV(port_id, INT2NUM(port->port_id));
  SET_IV(name, name);

  rb_obj_call_init(self, 0, NULL);
  return self;
}

void Init_aMIDI_Port()
{
  EIGENFUNC(Port, new, 2);

  GETTER(PortTX, cap_flags);
  GETTER(PortRX, cap_flags);

  FUNC_X(PortTX, note_on,  3);
  FUNC_X(PortTX, note_off, 3);
  FUNC_X(PortTX, note,     4);

  FUNC(Port, each_connected,  0);
  FUNC(Port, create_seq_mono, 1);
}
