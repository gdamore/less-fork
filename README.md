# LESS FORK

This is a version of LESS, a file viewer, that has been modified heavily.
It is based on the original program, located at

	[http://www.greenwoodsoftware.com/less/](http://www.greenwoodsoftware.com/less/)

However, it is modified to work better on POSIX systems, and uses APIs that
older systems and Microsoft platforms might lack.

The OpenBSD team has also contributed a number of improvements, mostly
oriented around safety and hardening of the code.

To build it, type `make STD=<x>` where `x` is one of:

* `ILLUMOS`
* `MACOS`
* `SUSv4`
* `SUSv3`
* `SUSv2`

Note that **SUSv1** and earlier are not supported.  These older systems are
missing the vsnprintf function.

See the Makefile for more details.  There is a `STD=default` option you can
try if your platform isn't listed.

See the [README.less](README.less) and [README.illumos](README.illumos) for more information.

Enjoy!

	- Garrett D'Amore <garrett@damore.org>, Nov. 5, 2015
