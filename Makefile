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
           RPMDEFS = --define '_topdir $(CURDIR)/rpmbuild' --define '_version $(VERSION)'
               TAR = /bin/tar


${PGM}:
	$(RM) $@
	$(CC) $(CFLAGS) src/$(PGM).c -o $@ -l$(LIBS)
	
dist:
	$(RM) $(PGM)
	$(CC) $(CFLAGS) src/$(PGM).c -o $(PGM) -l$(LIBS)
	$(GZ) -c docs/$(PGM).6 > docs/$(PGM).6.gz
	$(TAR) -cJf $(PGM)-$(VERSION).tar.xz -C .. $(PGM)/$(PGM) $(PGM)/COPYING $(PGM)/docs/$(PGM).6.gz
	$(RM) $(PGM)
	$(RM) docs/$(PGM).6.gz
	
debug:
	$(CC) $(CDEBUGFLAGS) src/$(PGM).c -o $(PGM) -l$(LIBS)

rpm:
	$(TAR) -cJf $(CURDIR)/rpmbuild/SOURCES/$(PGM)-$(VERSION).tar.xz -C .. $(PGM)/src $(PGM)/docs $(PGM)/COPYING $(PGM)/Makefile
	$(RPMBLD) --nodebuginfo $(RPMDEFS) -ba rpmbuild/SPECS/$(PGM).spec

clean:
	$(RM) $(PGM)
	$(RM) -r rpmbuild/BUILD
	$(RM) -r rpmbuild/BUILDROOT
	$(RM) -r rpmbuild/RPMS
	$(RM) -r rpmbuild/SRPMS
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

