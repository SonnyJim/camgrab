LIBS=-lcurl
OBJS= camgrab.o pidfile.o rotate.o cfg.o
CFLAGS += -g
camgrab: $(OBJS)
	$(CC) -o camgrab $(OBJS) $(CFLAGS) $(LIBS)

clean:
	rm -f *.o camgrab
install:
	cp camgrab /usr/local/bin/
	cp camgrab.service /etc/systemd/system/
