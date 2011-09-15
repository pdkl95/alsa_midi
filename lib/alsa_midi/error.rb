module AlsaMIDI
  class Error < RuntimeError
    class Param < Error
      def title
        super("Bad Parameter")
      end
    end
    
    def initialize(*args)
      @opt = args.last.is_a?(Hash) ? args.pop : {}
      @msg = args.shift
      @opt[:msg] ||= @msg if @msg
      if @opt.has_key?(:expected)
        ex = @opt[:expected]
        @opt[:expected] = [ex] unless ex.is_a? Array
      end
    end

    def line(str)
      @lines.push(str)
    end

    def line_prefix
      "#{self.class} **> "
    end

    def title(str=nil)
      str = "Unknown" unless str
      ">>> #{str} Exception <<<"
    end

    def msg
      "ERROR: #{@opt[:msg]}"
    end

    def given_to_s
      "#{@opt[:given].class} (#{@opt[:given].inspect})"
    end

    def default_line_header(step)
      step.to_s.capitalize
    end

    def default_line_to_s(step)
      x = @opt[step]
      x.is_a?(String) ? x : x.inspect
    end

    def line_part(step, type)
      func = "#{step}_#{type}"
      respond_to?(func) ? send(func) : send("default_line_#{type}", step)
    end

    def raw_line_for(step)
      if respond_to?(step)
        send(step)
      else
        [line_part(step, :header), line_part(step, :to_s)]
      end
    end
    
    def line_for(step)
      ln = raw_line_for(step)
      if ln.is_a? Array
        pfx, str = ln
        pfx = " #{pfx}" while pfx.length < 10
        line "#{pfx}: #{str}"
      else
        line ln
      end
    end

    def opt_line(step)
      line_for(step) if @opt.has_key?(step)
    end

    def err_lines
      @lines = []
      line_for :title
      [:msg, :param, :expected, :given
      ].each do |step|
        opt_line(step)
      end
      @lines
    end
    
    def err_str
      err_lines.map do |line|
        "#{line_prefix}#{line}"
      end.join("\n")
    end

    def message
      "\n#{err_str}\n"
    end
  end
end
