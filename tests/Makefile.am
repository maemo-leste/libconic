DEFS = -DDBUS_API_SUBJECT_TO_CHANGE
INCLUDES = \
	@GLIB_CFLAGS@ \
	@GOBJECT_CFLAGS@ \
	@DBUS_CFLAGS@ \
	-I$(top_srcdir) \
	-I$(top_srcdir)/conic

LIBS = \
	@GLIB_LIBS@ \
	@GOBJECT_LIBS@ \
	@DBUS_LIBS@ \
	$(top_builddir)/conic/libconic.la

AM_LDFLAGS = -static
AM_CFLAGS = -Wall -Werror -Wmissing-prototypes

bin_PROGRAMS = test-connect test-disconnect test-automatic-events \
		test-statistics test-proxies test-iap test-two-objects \
		test-destruction-at-callback

test_connect_SOURCES = test-connect.c

test_disconnect_SOURCES = test-disconnect.c

test_automatic_events_SOURCES = test-automatic-events.c

test_statistics_SOURCES = test-statistics.c

test_proxies_SOURCES = test-proxies.c

test_iap_SOURCES = test-iap.c

test_two_objects_SOURCES = test-two-objects.c

test_destruction_at_callback_SOURCES = test-destruction-at-callback.c

MAINTAINERCLEANFILES = Makefile.in
