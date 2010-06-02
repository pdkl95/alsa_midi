#!/usr/bin/env ruby

$LOAD_PATH << './lib'

require 'alsa_midi'

c = AlsaMIDI::Client.new
puts "client: #{c.inspect}"

sleep 2
