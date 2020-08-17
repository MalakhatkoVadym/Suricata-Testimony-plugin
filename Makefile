NAME :=		source-testimony

SRCS :=		plugin.c \
		source-testimony.c \
		runmode-testimony.c

# This needs to point to the Suricata include directory.
CPPFLAGS +=	-I/opt/suricata/src
CPPFLAGS += -I/usr/include/nss/
CPPFLAGS += -I/usr/include/nspr/

CPPFLAGS +=	-DSURICATA_PLUGIN -DHAVE_CONFIG_H -I.
CPPFLAGS +=	-DHAVE_PF_RING_FLOW_OFFLOAD

LDFLAGS += -L/usr/lib/

LDLIBS += -ltestimony

all:
	$(CC) -fPIC $(CPPFLAGS) -o $(NAME).so -shared $(SRCS) $(LDLIBS)

clean:
	rm -f *.so *~