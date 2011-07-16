# x11type/Makefile.am
#
AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = x11type
#man_MANS = x11type.1
x11type_SOURCES = x11type.cc

mrproper: maintainer-clean
	rm -f aclocal.m4 configure.scan depcomp missing install-sh config.h.in
	rm -f Makefile.in configure autoscan*.log config.guess config.sub
	rm -f ltmain.sh configure.in
doc:
	yodl2man -o x11type.1 x11type.yodl
