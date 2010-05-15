AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -O0 -Wall -Werror -ggdb -Isrc \
              $(PYTHON_CPPFLAGS) $(BOOST_CPPFLAGS)
AM_LDFLAGS  = $(PYTHON_LDFLAGS) $(PYTHON_EXTRA_LIBS) \
              $(BOOST_LDFLAGS) \
              $(BOOST_PROGRAM_OPTIONS_LIB) \
              $(BOOST_THREAD_LIB)
AM_LIBS     = $(PYTHON_EXTRA_LIBS)


noinst_LIBRARIES = libxynapse.a
libxynapse_a_SOURCES = src/PythonInterpreter.cc src/MessageHandler.cc \
  src/Json.cc
libxynapse_a_CPPFLAGS = -fPIC $(AM_CPPFLAGS)

bin_PROGRAMS = xynapse
xynapse_SOURCES = src/xynapse.cc \
  src/interfaces/TcpInterface.cc
xynapse_LDADD = libxynapse.a


dist_noinst_SCRIPTS = autogen.sh

.PHONY: all deep-clean xynapse-python xynapse-python-clean

if ENABLE_PYTHON_BINDING
PYTHON_BINDING=xynapse-python
endif

all: $(PYTHON_BINDING)

clean-local: xynapse-python-clean

install-exec-local: xynapse-python-install

uninstall-local: xynapse-python-uninstall

xynapse-python: src/bindings/python/xynapse.cc
	cd src/bindings/python && $(PYTHON) setup.py build
	- cp src/bindings/python/build/*/xynapse.so .

xynapse-python-clean:
	cd src/bindings/python && $(PYTHON) setup.py clean --all
	rm -rf src/bindings/python/build

xynapse-python-install:
	cd src/bindings/python && $(PYTHON) setup.py install

# This rule cleans up stuff installed by Python's setup.py. Unfortunately,
# Python's distutils do not provide an uninstall command, so we have to make
# up for it here in uninstall-local hook. This might break if distutils'
# behaviour changes as automake has no control over what distutils install
# command does.
xynapse-python-uninstall:
	rm -rf $(DESTDIR)$(libdir)/python*/site-packages/xynapse*
	rm -rf $(DESTDIR)$(libdir)/python*/dist-packages/xynapse*


deep-clean: distclean
	rm -f aclocal.m4 configure config.sub depcomp missing install-sh
	rm -f config.guess
	rm -rf autom4te.cache
	rm -rf */*.pyc *.pyc
	rm -rf */*.in *.in
	rm -rf xynapse.so