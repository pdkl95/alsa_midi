#!/usr/bin/env ruby

require 'rubygems'
require 'gtk2'

$LOAD_PATH << File.dirname(__FILE__) + '/lib'
require 'alsa_midi'

c = AlsaMIDI::Client.new :tx => 1
puts c.to_details

e = AlsaMIDI::Event.new
p = AlsaMIDI::Pattern.new

port = c.ports_tx.first

sleep 4
port.note_on! 0, 55, 123
port.note_on! 1, 58, 95

port.note_off! 0, 55, 33
port.note_off! 1, 58, 22

sleep 1

port.note! 10, 66, 121


sleep 2000