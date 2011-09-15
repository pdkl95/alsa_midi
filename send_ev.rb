#!/usr/bin/env ruby

$LOAD_PATH << File.dirname(__FILE__) + '/lib'

require 'alsa_midi'

[:major, :minor].each do |mode|
  ('A'..'G').to_a.each do |key|
    m = AlsaMIDI::Scale.new(key, mode)
    puts m.inspect
  end
end
#exit

c = AlsaMIDI::Client.new({ :tx => 1,
                           :rx => ['input_aaa', 'input_bbb'],
                           :clocks_per_beat => 4
                         })
puts c.to_details

ev      = AlsaMIDI::Ev.new
ev_note = AlsaMIDI::Ev::Note.new

port = c.tx_ports.first
#loop = port.create_seq16!(3)
#loop.set_note 10, 300000000

port.note! 1, 61, 121, 10
sleep 6

port.note! 10, 62, 121, 10

port.note_on! 1, 55, 123
port.note_on! 2, 58, 95
sleep 1

port.note_off! 1, 55, 33
port.note_off! 2, 58, 22
sleep 2

port.note! 10, 66, 121, 10


sleep 2000
