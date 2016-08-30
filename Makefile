all:
	$(CC) `pkg-config --cflags gstreamer-rtsp-server-1.0` -O2 -o rtsp-server rtsp-server.c `pkg-config --libs gstreamer-rtsp-server-1.0`

clean:
	rm -f rtsp-server
