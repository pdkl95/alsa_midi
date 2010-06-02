#!/usr/bin/env ruby

$LOAD_PATH << File.dirname(__FILE__) + '/lib'

require 'alsa_midi'

c = AlsaMIDI::Client.new :tx => 6, :rx => ['input_aaa', 'input_bbb']
puts c.to_details

e = AlsaMIDI::Event.new
p = AlsaMIDI::Pattern.new

c.start!

sleep 2000
