DEFS = -DDBUS_API_SUBJECT_TO_CHANGE -DGCONF_DISABLE_DEPRECATED
INCLUDES = @GLIB_CFLAGS@ @DBUS_CFLAGS@ @GCONF_CFLAGS@ @OSSO_IC_CFLAGS@
LIBS = @GLIB_LIBS@ @DBUS_LIBS@ @GCONF_LIBS@ @OSSO_IC_LIBS@

AM_CFLAGS = $(CONCFLAGS)

lib_LTLIBRARIES = libconic.la
libconic_la_SOURCES = conicconnection.c \
	conicevent.c conicevent-private.h \
	conicconnectionevent.c conicconnectionevent-private.h \
	conicstatisticsevent.c conicstatisticsevent-private.h \
	coniciap.c coniciap-private.h
libconic_la_LDFLAGS = -version-info 0:0:0

conicincludedir = $(includedir)/conic
conicinclude_HEADERS =  conicconnection.h \
	conicevent.h \
	conicconnectionevent.h \
	conicstatisticsevent.h \
	coniciap.h \
	conic.h

MAINTAINERCLEANFILES = Makefile.in
