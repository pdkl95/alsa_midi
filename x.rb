#!/usr/bin/env ruby

$LOAD_PATH << './lib'
#$LOAD_PATH << './ext/alsa_midi_looper'

require 'alsa_midi_looper'

#AlsaMIDI::Base.new
seq = AlsaMIDI::Seq.new
seq.tempo = 144
sleep 20
