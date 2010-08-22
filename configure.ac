AC_INIT(Radroach, devel)
AC_PROG_CC
AC_LANG(C)
AC_SEARCH_LIBS(cfg_init, confuse, [], [AC_MSG_ERROR(libconfuse missing)])
AC_SEARCH_LIBS(dlopen, dl, [], [AC_MSG_ERROR(libdl missing)])
AC_SEARCH_LIBS(scm_init_guile, guile, [], [AC_MSG_ERROR(Guile missing)])
AC_ARG_ENABLE(debug,
	AC_HELP_STRING([--enable-debug], [Build for debugging]),
	CFLAGS="$CFLAGS -O0 -g -DDEBUG=1", CFLAGS="$CFLAGS -DDEBUG=0")
AC_HEADER_STDBOOL
AC_CONFIG_FILES(Makefile)
AC_OUTPUT
