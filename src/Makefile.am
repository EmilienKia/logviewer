
AM_CPPFLAGS = \
	$(WX_CXXFLAGS) \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DICONSETDIR=\""$(datadir)/$(PACKAGE_NAME)/icons"\"

AM_CFLAGS =\
	 -Wall \
	 -g \
	 $(WX_CPPFLAGS)

bin_PROGRAMS = logviewer

logviewer_SOURCES = app.hpp app.cpp \
	data.hpp data.cpp \
	files.hpp files.cpp \
	frame.hpp frame.cpp \
	model.hpp model.cpp \
	parser.hpp parser.cpp \
	fdartprov.hpp fdartprov.cpp
logviewer_LDFLAGS =
logviewer_LDADD = $(WX_LIBS)
