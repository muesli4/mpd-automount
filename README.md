mpd-automount - An automounter that interacts with MPD
======================================================

Purpose
-------

`mpd-automount` is a small C++ program that interacts with UDisks2 via D-Bus to automount removable devices as a normal user and then add them as symbolic links to the MPD library. This linking is done with a bash script that requires `mpc` which selectively update the MPD database.

At the moment the MPD library is fixed at `~/Music`. If someone is interested I will add a configuration file.

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
