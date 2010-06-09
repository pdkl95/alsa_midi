#!/usr/bin/env ruby

require 'rubygems'
require 'gtk2'

$LOAD_PATH << File.dirname(__FILE__) + '/lib'
require 'alsa_midi'

$client = AlsaMIDI::Client.new :tx => 1
$port = $client.ports_tx.first
puts "MIDI transmit on: #{$port}"

#########################################################################

class GUI
  def initialize
    @mw = Gtk::Window.new

    @accel = Gtk::AccelGroup.new
    @accel.connect(Gdk::Keyval::GDK_Escape, 0, 0) do
      Gtk.main_quit
    end
    @mw.add_accel_group(@accel)
      
    @mw.signal_connect("delete_event") do
      false
    end
    @mw.signal_connect("destroy") do
      Gtk.main_quit
    end

    @mw.set_title "Virtual Keyboard"

    keys_hbox = Gtk::HBox.new(true, 5)

    @buttons = (0..6).to_a.map do |i|
      b = Gtk::Button.new(i.to_s)
      b.signal_connect("clicked") do
        note(i)
      end
      keys_hbox.pack_start(b)
      b
    end

    menu = Gtk::MenuBar.new

    menu_quit = Gtk::MenuItem.new("Quit!")
    menu_quit.signal_connect("activate") do
      Gtk.main_quit
    end
    menu.append(menu_quit)

    menu_key = Gtk::Menu.new
    ('A'..'G').to_a.each do |key|
      item = Gtk::MenuItem.new(key)
      item.signal_connect('activate') do
        set_key(key)
      end
      menu_key.append(item)
    end
    menu_key_item = Gtk::MenuItem.new('Key')
    menu_key_item.submenu = menu_key
    menu.append(menu_key_item)

    menu_mode = Gtk::Menu.new
    AlsaMIDI::Scale::Mode::LIST.each do |mode|
      item = Gtk::MenuItem.new(mode.to_s.capitalize)
      item.signal_connect('activate') do
        set_mode(mode)
      end
      menu_mode.append(item)
    end
    menu_mode_item = Gtk::MenuItem.new('Mode')
    menu_mode_item.submenu = menu_mode
    menu.append(menu_mode_item)
    
      
    vbox = Gtk::VBox.new(false, 0)
    vbox.pack_start(menu, false, false, 0)
    vbox.pack_start(keys_hbox, true, true, 10)
                      
    @mw.add(vbox)

    @key  = 'C'
    @mode = :ionian
    set_scale!
  end

  def set_key(key)
    @key = key
    set_scale!
  end

  def set_mode(mode)
    @mode = mode
    set_scale!
  end

  def set_scale!
    puts "set scale: #{@key}/#{@mode}"
    @scale = AlsaMIDI::Scale.new(@key, @mode)
    puts @scale.inspect
    @buttons.each_with_index do |b,idx|
      b.label = @scale.note_names[idx]
    end
  end

  def note(offset)
    midi_note = @scale.midi_note(offset)
    note_name = @scale.note_names[offset]
    puts "note: #{offset}, #{note_name} (MIDI_NOTE == #{midi_note})"
    $port.note_on!  0, midi_note, 127
    sleep 0.25
    $port.note_off! 0, midi_note, 127
  end

  def run!
    @mw.show_all
    Gtk.main
  end
end

GUI.new.run!


