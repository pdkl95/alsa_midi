#ifndef ALSA_PORT_H
#define ALSA_PORT_H

struct seq_port {
  struct alsa_midi_seq_client *client;
  int       port_id;
};
typedef struct seq_port port_t;

#define GET_PORT_STRUCT(obj)            \
  port_t *port;                         \
  Data_Get_Struct(obj, port_t, port);

#define GET_PORT GET_PORT_STRUCT(self)

#define PORT_STR(varname, client_id, port_id) \
  VALUE varname = PRINTF2("%d:%d",            \
                          INT2NUM(client_id), \
                          INT2NUM(port_id))

#define SUBS_CAP_MASK (SND_SEQ_PORT_CAP_SUBS_READ|SND_SEQ_PORT_CAP_SUBS_WRITE)
#define SUBS_CAP_ONLY(x) (x & SUBS_CAP_MASK)
#define SUBS_CAP SUBS_CAP_ONLY(find_caps(self))

void Init_aMIDI_Port();

#endif /*ALSA_Port_H*/
