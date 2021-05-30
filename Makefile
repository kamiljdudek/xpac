# Patched Makefile for xpac

              LIBS = X11
               PGM = xpac

           INSTALL = /usr/bin/install
      INSTALLFLAGS = -c
      INSTPGMFLAGS = -s
           DESTDIR = /usr/local/bin/

                CC = /usr/bin/gcc
            CFLAGS = -O3 -s
       CDEBUGFLAGS = -O0 -g -std=iso9899:2017

                RM = rm -f


${PGM}: ${OBJS}
	$(RM) $@
	$(CC) $(CFLAGS) ${PGM}.c -o $@ -l${LIBS}
	
debug:
	$(CC) $(CDEBUGFLAGS) $(PGM).c -o $(PGM) -l$(LIBS)

clean:
	$(RM) ${OBJS}
	$(RM) ${PGM}

install:
	$(INSTALL) $(INSTALLFLAGS) $(INSTPGMFLAGS) $(PGM) $(DESTDIR)$(PGM)
	
uninstall:
	$(RM) $(DESTDIR)$(PGM)

