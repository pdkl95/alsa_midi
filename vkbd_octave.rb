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
  def main_window
    @mw = Gtk::Window.new

    @accel = Gtk::AccelGroup.new
    @accel.connect(Gdk::Keyval::GDK_Escape, 0, 0) do
      quit_app!
    end
    @mw.add_accel_group(@accel)
      
    @mw.signal_connect("delete_event") do
      false
    end
    @mw.signal_connect("destroy") do
      quit_app!
    end

    @mw.set_title "VKbd Demo"

    vbox = Gtk::VBox.new(false, 0)
    vbox.pack_start(main_menu, false, false, 0)
    vbox.pack_start(main_buttons, true, true, 10)
                      
    @mw.add(vbox)
  end

  def key_menu
    @key_menu ||= returning Gtk::Menu.new do |menu|
      ('A'..'G').to_a.each do |key|
        item = Gtk::MenuItem.new(key)
        item.signal_connect('activate') do
          set_key(key)
        end
        menu.append(item)
      end
    end
  end

  def mode_menu
    @mode_menu ||= returning Gtk::Menu.new do |menu|
      AlsaMIDI::Scale::Mode::LIST.each do |mode|
        item = Gtk::MenuItem.new(mode.to_s.capitalize)
        item.signal_connect('activate') do
          set_mode(mode)
        end
        menu.append(item)
      end
    end
  end
  
  def main_menu
    @main_menu ||= returning Gtk::MenuBar.new do |menu|
      { 'Key'  => key_menu,
        'Mode' => mode_menu
      }.each_pair do |name, m_obj|
        item = Gtk::MenuItem.new(name)
        item.submenu = m_obj
        menu.append(item)
      end
    end
  end

  def main_buttons
    @main_buttons ||= returning Gtk::HBox.new(false, 10) do |hbox|
      b_quit = Gtk::Button.new('Quit!')
      b_quit.signal_connect('clicked') do
        quit_app!
      end
      hbox.pack_start(b_quit)

      b_kbd = Gtk::Button.new('Virt Kbd')
      b_kbd.signal_connect('clicked') do
        if @vkbd_window
          @vkbd_window.destroy
        else
          vkbd_window.show_all
        end
      end
      hbox.pack_start(b_kbd)

      b_loop = Gtk::Button.new('Loop Seq')
      b_loop.signal_connect('clicked') do
        if @loop_window
          @loop_window.destroy
        else
          loop_window.show_all
        end
      end
      hbox.pack_start(b_loop)
    end
  end
  
  def vkbd_window
    @vkbd_window ||= returning Gtk::Window.new do |w|
      w.set_title 'Virtual Keyboard'
      w.set_border_width 10
      keys_hbox = Gtk::HBox.new(true, 5)

      w.signal_connect("delete_event") do
        false
      end
      w.signal_connect("destroy") do
        @vkbd_window = nil
      end
      
      @vkbd_buttons = (0..6).to_a.map do |i|
        b = Gtk::Button.new(i.to_s)
        b.signal_connect("clicked") do
          note(i)
        end
        keys_hbox.pack_start(b)
        b
      end

      w.add(keys_hbox)
    end
  end

  def loop_window
    @loop_window ||= returning Gtk::Window.new do |w|
      w.set_title '(drum) Loop Sequencer'
      w.set_border_width 10

      w.signal_connect("delete_event") do
        false
      end
      w.signal_connect("destroy") do
        @loop_window = nil
      end
      
      grid = Gtk::Table.new(4, 17, true)
      #(1..16).each do |x|
      #  lbl = Gtk::Label.new(x.to_s)
      #  grid.attach_defaults(lbl, x, x+1, 0, 1)
      #end
      [:bass, :snare, :highhat, :crash].each_with_index do |name, idx|
        y = idx
        lbl = Gtk::Label.new(name.to_s)
        grid.attach(lbl, 0, 1, y, y+1, 0, 0, 10, 0)
        (1..16).each do |x|
          b = Gtk::ToggleButton.new(x.to_s)
          b.signal_connect('clicked') do
            status = b.active? ? 'ON' : 'OFF'
            puts "#{name}[#{x}] = #{status}"
          end
          grid.attach_defaults(b, x, x+1, y, y+1)
        end
      end
      w.add(grid)
    end
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
    if @vkbd_buttons
      @vkbd_buttons.each_with_index do |b,idx|
        b.label = @scale.note_names[idx]
      end
    end
  end

  def note(offset)
    midi_note = @scale.midi_note(offset)
    note_name = @scale.note_names[offset]
    puts "note: #{offset}, #{note_name} (MIDI_NOTE == #{midi_note})"
    $port.note!  0, midi_note, 127, 700000000
  end

  def initialize
    main_window
    @key  = 'C'
    @mode = :ionian
    set_scale!
  end

  def quit_app!
    Gtk.main_quit
  end

  def run!
    @mw.show_all
    Gtk.main
  end
end

GUI.new.run!


