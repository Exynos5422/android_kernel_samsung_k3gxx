CC		= $(CROSS_COMPILE)gcc
BUILD_OUTPUT	:= $(PWD)
PREFIX		:= /usr
DESTDIR		:=

turbostat : turbostat.c
CFLAGS +=	-Wall
<<<<<<< HEAD
CFLAGS +=	-I../../../../arch/x86/include/uapi/
=======
CFLAGS +=	-DMSRHEADER='"../../../../arch/x86/include/uapi/asm/msr-index.h"'
>>>>>>> 6d6f1883acbba69770ae242bdf44b3dbabed7e83

%: %.c
	@mkdir -p $(BUILD_OUTPUT)
	$(CC) $(CFLAGS) $< -o $(BUILD_OUTPUT)/$@

.PHONY : clean
clean :
	@rm -f $(BUILD_OUTPUT)/turbostat

install : turbostat
	install -d  $(DESTDIR)$(PREFIX)/bin
	install $(BUILD_OUTPUT)/turbostat $(DESTDIR)$(PREFIX)/bin/turbostat
	install -d  $(DESTDIR)$(PREFIX)/share/man/man8
	install turbostat.8 $(DESTDIR)$(PREFIX)/share/man/man8
