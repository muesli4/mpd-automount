mpd-automount - An automounter that interacts with MPD
======================================================

Purpose
-------

`mpd-automount` is a small C++ program that interacts with UDisks2 via D-Bus to automount removable devices as a normal user and then add them as symbolic links to the MPD library. This linking is done with a bash script that requires `mpc` which selectively updates the MPD database.

Configuration
-------------

`mpd-automount` will create a configuration file in `$XDG_CONFIG_HOME` or `$HOME/.config` if it is missing. The default config is:
```
MPD_LIBRARY_PATH=~/Music
LINK_SUB_DIRECTORY=.
```
`LINK_SUB_DIRECTORY` specifies where, relative to the library path, the links are placed.

Dependencies
------------

* udisks2
    * http://www.freedesktop.org/wiki/Software/udisks
    * Arch Linux Package: udisks2

* mpc


Compiling
---------

* autoreconf -vfi
* ./configure
* make
* make install

Running
-------

Either run `mpd-automount` or use `systemctl --user start mpd-automount` (`systemctl --user enable mpd-automount` to always start it).


Credits
-------
This program has been forked from [usermount](https://github.com/tom5760/usermount).
