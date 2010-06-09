module AlsaMIDI
  class Scale
    CHROMA = 'A A# B C C# D D# E F F# G G#'.split(/\s+/)
    
    class Mode
      STANDARD = {
        :aeolian    => { :white_note => 'A', :spacing => 'T-s-T-T-s-T-T' },
        :locrian    => { :white_note => 'B', :spacing => 's-T-T-s-T-T-T' },
        :ionian     => { :white_note => 'C', :spacing => 'T-T-s-T-T-T-s' },
        :dorian     => { :white_note => 'D', :spacing => 'T-s-T-T-T-s-T' },
        :phrygian   => { :white_note => 'E', :spacing => 's-T-T-T-s-T-T' },
        :lydian     => { :white_note => 'F', :spacing => 'T-T-T-s-T-T-s' },
        :mixolydian => { :white_note => 'G', :spacing => 'T-T-s-T-T-s-T' }
      }
      LIST = [:ionian, :dorian, :phrygian, :lydian, :mixolydian, :aeolian, :locrian]

      attr_reader :name
    
      def initialize(type_name = :ionian, opt = STANDARD[type_name])
        @type       = type_name.to_sym
        @name       = type_name.to_s.capitalize
        @white_note = opt[:white_note]
        @spacing    = opt[:spacing]
      end

      def parse_offsets(str = @spacing)
        str.gsub! /\s+/, '-'
        str.split(/-/).map do |s|
          case s
          when /\A\d+\Z/ then s.to_i
          when 'T'       then 2
          when 's'       then 1
          else raise "unknown scale spacing: #{s.inspect}"
          end
        end
      end

      def find_cumulative_offsets
        returning [] do |list|
          total = 0
          list.push(0)
          offsets.each do |off|
            total += off
            list.push(total)
          end
        end.slice(0,7)
      end

      def offsets
        @offsets || (@offsets = parse_offsets)
      end

      def offsets_cumulative
        @offsets_cumulative || (@offsets_cumulative = find_cumulative_offsets)
      end

      def inspect
        "#<#{self.class} \"#{@name}\" #{@white_note}:[#{offsets.join(' ')}]>"
      end
    end

    def initialize(key='c', mode=:ionian)
      mode = :ionian  if mode == :major
      mode = :aeolian if mode == :minor
      
      @mode = Mode.new(mode)
      @key  = key.to_s.upcase
      @key_offset = CHROMA.index(@key)
    end

    def midi_note(off, octave=0)
      57 + @mode.offsets_cumulative[off] + @key_offset + (12 * octave)
    end

    def offset_to_name(off)
      CHROMA[(off + @key_offset) % 12]
    end

    def note_names
      @note_names ||= @mode.offsets_cumulative.map{ |off| offset_to_name(off) }
    end

    def inspect
      "#<#{self.class} \"#{@key}/#{@mode.name}\":[#{note_names.join(' ')}]>"
    end
  end
end
