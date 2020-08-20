NAME :=		source-testimony

SRCS :=		plugin.c \
		source-testimony.c \
		runmode-testimony.c

# This needs to point to the Suricata include directory.
# CPPFLAGS +=	-I/home/vadim/Desktop/apple/suricata/src
CPPFLAGS += -I/usr/include/nss/
CPPFLAGS += -I/usr/include/nspr/

CPPFLAGS +=	-DSURICATA_PLUGIN -DHAVE_CONFIG_H -I.

LDLIBS += -ltestimony

all:
	$(CC) -fPIC $(CPPFLAGS) -o $(NAME).so -shared $(SRCS) $(LDLIBS)

clean:
	rm -f *.so *~