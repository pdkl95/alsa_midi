#include "alsa_midi.h"
#include "pattern.h"
#include "client.h"

static VALUE client_set_queue_len(VALUE self, VALUE new_queue_length)
{
  GET_CLIENT;
  int len = NUM2INT(new_queue_length);
  snd_seq_set_client_pool_output(client->handle, len);
  printf("Allocated sequencer queue (length == %d)\n", len);
  return INT2NUM(len);
}

static VALUE client_set_name(VALUE self, VALUE new_name)
{
  GET_CLIENT;
  client->name = StringValuePtr(new_name);
  printf("NEW name: %s\n", client->name);
  snd_seq_set_client_name(client->handle, client->name);
  return new_name;
}

static VALUE client_get_name(VALUE self)
{
  GET_CLIENT;
  return client->name ? rb_str_new2(client->name) : Qnil;
}

static VALUE client_set_bpm(VALUE self, VALUE new_bpm)
{
  GET_CLIENT;
  client->bpm = NUM2INT(new_bpm);

  snd_seq_queue_tempo_t *queue_tempo;
  snd_seq_queue_tempo_malloc(&queue_tempo);

  double tpq = (double)(client->ticks_per_quarter);
  double bpm = (double)(client->bpm);
  client->tempo = (int)(6e7 / (bpm * tpq * tpq));
  printf("New Tempo: %d bpm, %d us/tick\n", client->bpm, client->tempo);

  snd_seq_queue_tempo_set_tempo(queue_tempo, client->tempo);
  snd_seq_queue_tempo_set_ppq(queue_tempo, client->ticks_per_quarter);
  snd_seq_set_queue_tempo(client->handle, client->queue_id, queue_tempo);

  snd_seq_queue_tempo_free(queue_tempo);
  return new_bpm;
}

static VALUE client_get_bpm(VALUE self)
{
  GET_CLIENT;
  return INT2NUM(client->bpm);
}

static VALUE client_get_tempo(VALUE self)
{
  GET_CLIENT;
  return INT2NUM(client->tempo);
}

static VALUE seq_startup(VALUE self)
{
  GET_CLIENT;
  int npfd;
  struct pollfd *pfd;

  snd_seq_start_queue(client->handle, client->queue_id, NULL);
  snd_seq_drain_output(client->handle);
  npfd = snd_seq_poll_descriptors_count(client->handle, POLLIN);
  pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
  snd_seq_poll_descriptors(client->handle, pfd, npfd, POLLIN);

}

static void clear_queue(alsa_midi_client_t *client)
{
  if (client->queue_id) {
    printf("Clearing sequencer queue...\n");
    //snd_seq_clear_queue(client->queue_id);
    snd_seq_drain_output(client->handle);
    sleep(SHUTDOWN_WAIT_TIME);
    snd_seq_stop_queue(client->handle, client->queue_id, NULL);
    printf("Sequencer queue cleared!\n");
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

static VALUE client_alloc(VALUE klass)
{
  alsa_midi_client_t *client;
  VALUE obj = Data_Make_Struct(klass, alsa_midi_client_t,
                               NULL, client_free, client);

  client->name              = DEFAULT_CLIENT_NAME;
  client->bpm               = DEFAULT_BPM;
  client->ticks_per_quarter = DEFAULT_TICKS_PER_QUARTER;
  client->queue_id          = 0;

  if (snd_seq_open(&(client->handle), "default", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
    rb_raise(aMIDI_AlsaError, "Error opening ALSA sequencer.");
    exit(1);
  }

  return obj;
}

void Init_aMIDI_Client()
{
  rb_define_alloc_func(aMIDI_Client, client_alloc);
  rb_define_method(aMIDI_Client, "name",          client_get_name,      0);
  rb_define_method(aMIDI_Client, "name=",         client_set_name,      1);
  rb_define_method(aMIDI_Client, "queue_length=", client_set_queue_len, 1);
  rb_define_method(aMIDI_Client, "tempo",         client_get_tempo,     0);
  rb_define_method(aMIDI_Client, "bpm",           client_get_bpm,       0);
  rb_define_method(aMIDI_Client, "bpm=",          client_set_bpm,       1);
}
