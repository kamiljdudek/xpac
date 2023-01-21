# Patched Makefile for xpac

           VERSION = 0.13

              LIBS = X11
               PGM = xpac

           INSTALL = /usr/bin/install
      INSTALLFLAGS = -D -g 0 -o 0 -m 644
      INSTPGMFLAGS = -D -s -g 0 -o 0 -m 755
           DESTDIR = /usr/local/bin/

             MANDB = /usr/bin/mandb
        MANDBFLAGS = -p
        MANDESTDIR = /usr/local/share/man/man6/

                CC = /usr/bin/gcc
            CFLAGS = -O3 -s
       CDEBUGFLAGS = -O0 -g -Wall -Wextra -std=iso9899:2017

                RM = /bin/rm -f
                RD = /bin/rmdir
                GZ = /bin/gzip
             MKDIR = /bin/mkdir -p

            RPMBLD = /usr/bin/rpmbuild
               TAR = /bin/tar


${PGM}:
	$(RM) $@
	$(CC) $(CFLAGS) src/$(PGM).c -o $@ -l$(LIBS)
	
dist:
	$(RM) $(PGM)
	$(CC) $(CFLAGS) src/$(PGM).c -o $(PGM) -l$(LIBS)
	$(GZ) -c docs/$(PGM).6 > docs/$(PGM).6.gz
	$(TAR) -cJf $(PGM)-$(VERSION).tar.xz -C .. $(PGM)/$(PGM) $(PGM)/LICENSE $(PGM)/docs/$(PGM).6.gz
	$(RM) $(PGM)
	$(RM) docs/$(PGM).6.gz
	
debug:
	$(CC) $(CDEBUGFLAGS) src/$(PGM).c -o $(PGM) -l$(LIBS)

rpm:
	$(MKDIR) $(CURDIR)/rpmbuild/SOURCES
	$(TAR) -cJf $(CURDIR)/rpmbuild/SOURCES/$(PGM)-$(VERSION).tar.xz -C .. $(PGM)/src $(PGM)/docs $(PGM)/LICENSE $(PGM)/Makefile
	$(RPMBLD) --verbose --define '_topdir $(CURDIR)/rpmbuild' --define '_version $(VERSION)' -ba xpac.spec

clean:
	$(RM) $(PGM)
	$(RM) -r rpmbuild
	$(RM) $(PGM).6.gz
	$(RM) $(PGM)-$(VERSION).tar.xz

install:
	$(GZ) -c docs/$(PGM).6 > $(PGM).6.gz
	$(MKDIR) $(MANDESTDIR)
	$(INSTALL) $(INSTALLFLAGS) $(PGM).6.gz $(MANDESTDIR)
	$(MANDB) $(MANDBFLAGS)
	$(MKDIR) $(DESTDIR)
	$(INSTALL) $(INSTPGMFLAGS) $(PGM) $(DESTDIR)
	
uninstall:
	$(RM) $(DESTDIR)$(PGM)
	$(RM) $(MANDESTDIR)$(PGM).6.gz
	$(MANDB)

