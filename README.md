# LESS FORK

## License

This version of Less (which is a fork, see below), is made available under
the terms of the Less License, located in the file [LICENSE](LICENSE).
Some of the source files may be also distributed under the
[GPLv3](http://www.gnu.org/licenses/gpl-3.0.en.html), and the implementations
of strlcpy and strlcat (which will not be used if your platform supplies them)
are provided courtesy of OpenBSD under an MIT-style license. (See the source
code files for more specific details.)

The original Less v458 carried the following notice:

    This is the distribution of less, version 458, released 04 Apr 2013.
    This program is part of the GNU project (http://www.gnu.org).

    This program is free software.  You may redistribute it and/or
    modify it under the terms of either:

    1. The GNU General Public License, as published by the Free
       Software Foundation; either version 3, or (at your option) any
       later version.  A copy of this license is in the file COPYING.
    or
    2. The Less License, in the file LICENSE.

This modified distribution is generally made available under the terms of Less
License, and we have chosen the Less License as the terms under which we
distribute our changes.

## About

This is a version of LESS, a file viewer, that has been modified heavily.
It is based on the Mark Nudelman's original program, Less v458.  The original
less is located at
[http://www.greenwoodsoftware.com/less/](http://www.greenwoodsoftware.com/less/)

This version is modified to work better on POSIX systems, and uses APIs that
older systems and non-POSIX platforms might lack.  In many cases these newer
APIs have improved safety or performance.

It is also modified to substantially improve its compliance with the POSIX
specifications when running as `more`.  I believe that when called as `more`,
less fully implements both Issue 6 and Issue 7 specified behaviors of the more
utility.

Finally, substantial effort has been invested to improve the readability and
maintainability of the code, and to avoid reimplementing library functions
found on standard systems.

An incomplete list of changes is located in the file [CHANGES](CHANGES).

## Acknowledgements

Mark Nudelman created the original Less program, which I have used for
many years on many platforms.

The OpenBSD team has contributed a number of improvements, mostly
oriented around safety and hardening of the code.

## Building

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

Enjoy!

	- Garrett D'Amore <garrett@damore.org>, Nov. 6, 2015
