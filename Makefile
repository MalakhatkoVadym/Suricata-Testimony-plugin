NAME :=		source-testimony

SRCS :=		plugin.c \
		source-testimony.c \
		runmode-testimony.c

ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

# This needs to point to the Suricata include directory.
# CPPFLAGS +=	-I/home/vadim/Desktop/apple/suricata/src
CPPFLAGS += -I/usr/include/nss3/
CPPFLAGS += -I/usr/include/nspr4/

CPPFLAGS +=	-DSURICATA_PLUGIN -DHAVE_CONFIG_H -I.

LDLIBS += -ltestimony

all:
	$(CC) -fPIC $(CPPFLAGS) -o $(NAME).so -shared $(SRCS) $(LDLIBS)

clean:
	rm -f *.so *~

install: source-testimony.so
	install -d $(PREFIX)/lib/
	install -m 644 source-testimony.so $(PREFIX)/lib/