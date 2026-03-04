
alsa-voice 8 bit voice chat
 probably not the best way to create a voice chat, i pretty much broke all the rules lol

	Usage:
		Server:
			./achat [listener_port] [capture device] [playback device]

		Client:
			./achat [server_ip] [port] [capture device] [playback device]

		Example:
			Server:
				./achat 1122 default default

			Client:
				./achat 127.0.0.1 1122 default default
