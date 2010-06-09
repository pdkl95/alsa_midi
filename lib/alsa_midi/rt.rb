module AlsaMIDI
  class RT
    class << self
      def worker
        @worker ||= new
      end

      def register_client(client)
        worker.register_client(client)
      end
    end

    def initialize(opt={})
      @opt = DEFAULT_RT_OPT.merge(opt)
      @rtc = @opt[:rtc]
      @clients = []
      setup!
    end

    def register_client(new_client)
      info "New client: #{new_client}"
      @clients << new_client
      new_client.rt_worker = self
      start! unless running?
    end
  end
end
