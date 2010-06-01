#!/usr/bin/env ruby
require 'mkmf'

DIR_NAME = 'alsa_midi'
LIB_NAME = 'looper'

ALSA_PKG = 'alsa'          # for pkg-config
ALSA_H   = 'asoundlib.h'

fail "Cannot find pkg-config!" unless find_executable('pkg-config')
pkg = pkg_config(ALSA_PKG) or fail "Missing package #{ALSA_PKG} in pkg-config!"
header_dir = pkg[0].sub(/^-I/,'').sub(/\s*$/,'')
puts "using ALSA headers in: #{header_dir.inspect}"

['stdio.h', 'stdlib.h', 'unistd.h'
].each do |hdr|
  have_header(hdr) or fail "missing: basic headers #{hdr.inspect}"
end

have_header(ALSA_H) or fail "missing: alsa header!"
#&& have_library('asound', 'snd_seq_open')

['OPEN_DUPLEX',    'PORT_TYPE_APPLICATION',
 'PORT_CAP_READ',  'PORT_CAP_SUBS_READ',
 'PORT_CAP_WRITE', 'PORT_CAP_SUBS_WRITE'
].map do |m|
  "SND_SEQ_#{m}"
end.each do |m|
  have_macro(m, ALSA_H) or fail "Missing ALSA macro: #{m}"
end

['snd_seq_tick_time_t',
 #'snd_seq_t'
 #'snd_seq_queue_tempo_t'
].each do |t|
  have_type(t, ALSA_H) or fail "Missing ALSA type: #{t}"
end

#dir_config(DIR_NAME)
#create_makefile("#{DIR_NAME}/#{LIB_NAME}", DIR_NAME)
create_makefile 'alsa_midi', 'alsa_midi'
