#include "alsa_midi_seq.h"

static VALUE Client_set_queue_length(VALUE self, VALUE new_queue_length)
{
  GET_CLIENT;
  int len = NUM2INT(new_queue_length);
  //snd_seq_set_client_pool_output(client->handle, len);
  
  DEBUG_VAL("output pool length: %d", new_queue_length);
  return new_queue_length;
}

static VALUE Client_set_name(VALUE self, VALUE new_name)
{
  GET_CLIENT;
  client->name = StringValuePtr(new_name);
  DEBUG_STR("ALSA client name: \"%s\"", client->name);
  snd_seq_set_client_name(client->handle, client->name);
  return new_name;
}

static VALUE Client_get_name(VALUE self)
{
  GET_CLIENT;
  return client->name ? rb_str_new2(client->name) : Qnil;
}

static aMIDI_inline void set_tempo_from_bpm(alsa_midi_client_t *client)
{
  double d_ppq = (double)(client->ppq);
  double d_bpm = (double)(client->bpm);
  client->tempo = (int)(6e7 / (d_bpm * d_ppq * d_ppq));
}

static void update_queue_tempo(VALUE self)
{
  GET_CLIENT;
  set_tempo_from_bpm(client);
  return;
  snd_seq_queue_tempo_t *queue_tempo;
  snd_seq_queue_tempo_alloca(&queue_tempo);
  snd_seq_queue_tempo_set_tempo(queue_tempo, client->tempo);
  snd_seq_queue_tempo_set_ppq(queue_tempo, client->ppq);
  snd_seq_set_queue_tempo(client->handle, client->queue_id, queue_tempo);
  rb_funcall(self, rb_intern("tempo_changed!"), 0);
}

static VALUE Client_set_ppq(VALUE self, VALUE new_ppq)
{
  GET_CLIENT;
  client->ppq = NUM2INT(new_ppq);
  update_queue_tempo(self);
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
  update_queue_tempo(self);
  return new_bpm;
}

static VALUE Client_get_bpm(VALUE self)
{
  GET_CLIENT;
  return INT2NUM(client->bpm);
}

static VALUE Client_get_tempo(VALUE self)
{
  GET_CLIENT;
  return INT2NUM(client->tempo);
}

static VALUE Client_start_queue(VALUE self)
{
  GET_CLIENT;

  DEBUG("GO!");
  //snd_seq_start_queue(client->handle, client->queue_id, NULL);
  //snd_seq_drain_output(client->handle);

  /*  DEBUG("collecting poll descriptors...");
  int           npfd = snd_seq_poll_descriptors_count(client->handle, POLLIN);
  struct pollfd *pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
  DEBUG("pollin");
  snd_seq_poll_descriptors(client->handle, pfd, npfd, POLLIN);*/
}

static void clear_queue(alsa_midi_client_t *client)
{
  if (client->queue_id) {
    printf("draining sequencer output...");
    //snd_seq_clear_queue(client->queue_id);
    snd_seq_drain_output(client->handle);
    sleep(SHUTDOWN_WAIT_TIME);
    snd_seq_stop_queue(client->handle, client->queue_id, NULL);
    printf("sequencer queue cleared!");
  }
}

static VALUE client_clear_queue(VALUE self)
{
  GET_CLIENT;
  clear_queue(client);
}

static void client_free(alsa_midi_client_t *client)
{
  clear_queue(client);
  snd_seq_free_queue(client->handle, client->queue_id);
  client->queue_id = 0;
  free(client);
}

static VALUE Client_alloc(VALUE klass)
{
  alsa_midi_client_t *client;
  VALUE obj = Data_Make_Struct(klass, alsa_midi_client_t,
                               NULL, client_free, client);

  client->name     = DEFAULT_CLIENT_NAME;
  client->bpm      = DEFAULT_BPM;
  client->ppq      = DEFAULT_PPQ;
  client->queue_id = 0;

  set_tempo_from_bpm(client);

  if (snd_seq_open(&(client->handle), "default", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
    rb_raise(aMIDI_AlsaError, "Error opening ALSA sequencer.");
    exit(1);
  }
  rb_iv_set(obj, "@client_id", INT2NUM(snd_seq_client_id(client->handle)));
  return obj;
}

void Init_aMIDI_Client()
{
  CUSTOM_ALLOC(Client);

  ACCESSOR(Client, name);
  ACCESSOR(Client, bpm);
  ACCESSOR(Client, ppq);

  SETTER(Client, queue_length);
  GETTER(Client, tempo);

  FUNC_X(Client, start_queue, 0);
}
