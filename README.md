achat low bandwidth command line voice chat using alsa and libopus.

	Usage:
		Server:
			./achat [listener_port]

		Client:
			./achat [server_ip] [port] [capture device] [playback device]

		Example:
			Server:
				./achat 1122

			Client:
				./achat 127.0.0.1 1122 default default
