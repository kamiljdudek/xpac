# Patched Makefile for xpac

              LIBS = X11
               PGM = xpac

           INSTALL = /usr/bin/install
             MANDB = /usr/bin/mandb
        MANDBFLAGS = -p
      INSTALLFLAGS = -g 0 -o 0 -m 644
      INSTPGMFLAGS = -s -g 0 -o 0 -m 755
           DESTDIR = /usr/local/bin/
        MANDESTDIR = /usr/local/share/man/man6/

                CC = /usr/bin/gcc
            CFLAGS = -O3 -s
       CDEBUGFLAGS = -O0 -g -std=iso9899:2017

                RM = rm -f
                GZ = /usr/bin/gzip


${PGM}:
	$(RM) $@
	$(CC) $(CFLAGS) $(PGM).c -o $@ -l$(LIBS)
	
debug:
	$(CC) $(CDEBUGFLAGS) $(PGM).c -o $(PGM) -l$(LIBS)

clean:
	$(RM) $(PGM)
	$(RM) $(PGM).6.gz

install:
	$(GZ) -c $(PGM).6 > $(PGM).6.gz
	$(INSTALL) $(INSTALLFLAGS) $(PGM).6.gz $(MANDESTDIR)$(PGM).6.gz
	$(MANDB) $(MANDBFLAGS)
	$(INSTALL) $(INSTPGMFLAGS) $(PGM) $(DESTDIR)$(PGM)
	
uninstall:
	$(RM) $(DESTDIR)$(PGM)
	$(RM) $(MANDESTDIR)$(PGM).6.gz
	$(MANDB)

