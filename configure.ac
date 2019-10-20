AC_INIT(logviewer, 0.1)

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([aux-bin])

AM_INIT_AUTOMAKE([1.11 foreign])

AM_SILENT_RULES([yes])

AC_PROG_CC
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX(14)

WXCONFIG=wx-config
AC_ARG_WITH(wx-config,
[[  --with-wx-config=FILE
    Use the given path to wx-config when determining
    wxWidgets configuration; defaults to "wx-config"]],
[
    if test "$withval" != "yes" -a "$withval" != ""; then
        WXCONFIG=$withval
    fi
])
AC_MSG_CHECKING([wxWidgets version])
if wxversion=`$WXCONFIG --version`; then
    AC_MSG_RESULT([$wxversion])
else
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([wxWidgets is required. Try --with-wx-config.])
fi

WX_CPPFLAGS="`$WXCONFIG --cppflags base,core,adv,aui,ribbon,stc,xrc`"
WX_CXXFLAGS="`$WXCONFIG --cxxflags base,core,adv,aui,ribbon,stc,xrc`"
WX_LIBS="`$WXCONFIG --libs base,core,adv,aui,ribbon,stc,xrc`"

AC_SUBST(WX_CPPFLAGS)
AC_SUBST(WX_CXXFLAGS)
AC_SUBST(WX_LIBS)

AC_PROG_CPP
AC_PROG_CXX


dnl ***************************************************************************
dnl Internationalization
dnl ***************************************************************************
IT_PROG_INTLTOOL([0.35.0])

GETTEXT_PACKAGE=logviewer
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])
AM_GLIB_GNU_GETTEXT



AC_OUTPUT([
Makefile
src/Makefile
po/Makefile.in
share/Makefile
share/icons/Makefile
share/icons/16/Makefile
share/icons/32/Makefile
])
