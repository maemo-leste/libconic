SUBDIRS = conic tests

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = conic.pc

MAINTAINERCLEANFILES = \
        Makefile.in config.h.in configure \
        install-sh ltmain.sh missing mkinstalldirs \
        config.log config.status config.guess config.sub \
        build-stamp compile depcomp acinclude.m4 aclocal.m4

EXTRA_DIST = \
	debian/changelog \
	debian/compat \
	debian/control \
	debian/copyright \
	debian/rules

if DOXYGEN_DOCS_ENABLED
  DOXYGEN_LINE = $(DOXYGEN) doxygen.cfg
else 
  DOXYGEN_LINE = 
endif

.PHONY : doc

doc: 
	$(DOXYGEN_LINE)

deb: dist
	-mkdir $(top_builddir)/debian-build
	cd $(top_builddir)/debian-build && tar zxf ../$(top_builddir)/$(PACKAGE)-$(VERSION).tar.gz
	cd $(top_builddir)/debian-build/$(PACKAGE)-$(VERSION) && dpkg-buildpackage -rfakeroot -sa -D
	-rm -rf $(top_builddir)/debian-build/$(PACKAGE)-$(VERSION)
