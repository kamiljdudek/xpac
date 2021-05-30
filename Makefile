# Patched Makefile for xpacman

              LIBS = X11
               PGM = xpacman

           INSTALL = /usr/bin/install
      INSTALLFLAGS = -c
      INSTPGMFLAGS = -s
           DESTDIR = /usr/local/bin/

                CC = /usr/bin/gcc
            CFLAGS = -O3 -s
           
                RM = rm -f


${PGM}: ${OBJS}
	$(RM) $@
	$(CC) $(CFLAGS) ${PGM}.c -o $@ -l${LIBS}

clean:
	$(RM) ${OBJS}
	$(RM) ${PGM}

install:
	$(INSTALL) $(INSTALLFLAGS) $(INSTPGMFLAGS) $(PGM) $(DESTDIR)$(PGM)
	
uninstall:
	$(RM) $(DESTDIR)$(PGM)

