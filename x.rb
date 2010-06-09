#!/usr/bin/env ruby

$LOAD_PATH << File.dirname(__FILE__) + '/lib'

require 'alsa_midi'

c = AlsaMIDI::Client.new :tx => 1, :rx => ['input_aaa', 'input_bbb']
puts c.to_details

e = AlsaMIDI::Event.new
p = AlsaMIDI::Pattern.new

port = c.ports_tx.first

sleep 4
port.note_on! 0, 55, 123
port.note_on! 1, 58, 95

port.note_off! 0, 55, 33
port.note_off! 1, 58, 22


sleep 2000
