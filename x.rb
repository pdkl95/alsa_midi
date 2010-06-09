#!/usr/bin/env ruby

$LOAD_PATH << File.dirname(__FILE__) + '/lib'

require 'alsa_midi'

c = AlsaMIDI::Client.new :tx => 2, :rx => ['input_aaa', 'input_bbb']
puts c.to_details

e = AlsaMIDI::Event.new
p = AlsaMIDI::Pattern.new

sleep 2
AlsaMIDI::RT.worker.send_midi
sleep 2
AlsaMIDI::RT.worker.send_midi
AlsaMIDI::RT.worker.send_midi
AlsaMIDI::RT.worker.send_midi
AlsaMIDI::RT.worker.send_midi
sleep 5
AlsaMIDI::RT.worker.stop!
exit
while true
  c.each_tx_port do |tx|
    tx.send_note
  end
  puts "sleep..."
  sleep 1
end

sleep 2000
