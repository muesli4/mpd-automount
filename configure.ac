AC_INIT([mpd-automount], [0.0.1], [muesli4@gmail.com])

#AC_CONFIG_SRCDIR([src/config.h.in])
#AC_CONFIG_HEADER(src/config.h)

AC_CONFIG_MACRO_DIRS([m4])
AM_INIT_AUTOMAKE([-Wall foreign])

AC_SUBST([AM_CXXFLAGS], ["-std=c++17 -std=gnu++17 -Wall"])

AC_PROG_CXX
PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES(UDISKS2, udisks2)
#PKG_CHECK_MODULES(CONFIG, libconfig++)

AC_CONFIG_FILES([Makefile src/Makefile systemd/Makefile systemd/mpd-automount.service])

AC_ARG_WITH([systemduserunitdir],
            AS_HELP_STRING([--with-systemduserunitdir=DIR], [Directory for systemd user service files]),
            [], [with_systemduserunitdir=$($PKG_CONFIG --variable=systemduserunitdir systemd)])
if test "x$with_systemduserunitdir" != xno; then
            AC_SUBST([systemduserunitdir], [$with_systemduserunitdir])
fi

MPD_AUTOMOUNT_BINARY=${bindir}/mpd-automount
AX_DEFINE_DIR(MPD_AUTOMOUNT_BINARY, MPD_AUTOMOUNT_BINARY, [Location of binary])

#AM_GNU_GETTEXT([external])
#AM_GNU_GETTEXT_VERSION([0.19.7])

AC_OUTPUT
