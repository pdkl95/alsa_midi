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

port = c.ports_tx.first
loop = port.create_seq16!(3)
loop.set_note 10, 300000000

sleep 4
port.note_on! 0, 55, 123
port.note_on! 1, 58, 95

port.note_off! 0, 55, 33
port.note_off! 1, 58, 22

sleep 1

port.note! 10, 66, 121, 900000000


sleep 2000
