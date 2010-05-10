AC_INIT(Radroach, devel)
AC_PROG_CC
AC_LANG(C)
AC_SEARCH_LIBS([cfg_init, cfg_parse], confuse, [], [AC_MSG_ERROR(libconfuse missing)])
AC_SEARCH_LIBS(dlopen, dl, [], [AC_MSG_ERROR(libdl missing)])
AC_OUTPUT(Makefile)