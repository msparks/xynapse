AC_PREREQ(2.61)
AC_INIT([xynapse], [1.0], ms@quadpoint.org)
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_RANLIB

# Checks for libraries.
AM_PATH_PYTHON([2.6],, [:])
AM_CONDITIONAL([HAVE_PYTHON], [test "$PYTHON" != :])
AX_PYTHON_DEVEL([>= '2.6'])
AX_BOOST_BASE([1.41.0], [], [
  echo "Boost version >= 1.41.0 not found."
  echo "Install updated Boost C++ libraries first."
  exit -1])
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_THREAD


AC_ARG_ENABLE([python-binding],
              AS_HELP_STRING([--disable-python-binding],
                             [build python binding (default=yes)]),
              [ac_enable_python_binding=$enableval],
              [ac_enable_python_binding=yes])
AM_CONDITIONAL(ENABLE_PYTHON_BINDING,
               test "x$ac_enable_python_binding" = "xyes")


AC_CONFIG_FILES([
  Makefile
  src/bindings/python/setup.py
  ])
AC_OUTPUT


echo

if test "x$ac_enable_python_binding" = "xno"; then
	echo "Building with Python support ..... : no"
elif test "x$ac_enable_python_binding" = "xyes" -a ! "$PYTHON"; then
	echo "Building with Python support ..... : no"
else
	echo "Building with Python support ..... : yes"
fi

if test "x$ac_enable_python_binding" = "xyes" -a ! "$PYTHON"; then
	echo "  - You're probably missing the Python header (Python.h) or it couldn't"
	echo "    be found."

	if test -f /etc/debian_version; then
		echo "  - Try: apt-get install python-dev"
	fi
fi

echo

echo "Install prefix ................... : $prefix"