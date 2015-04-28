Riemann C client library
========================

[![Build Status](https://img.shields.io/travis/algernon/riemann-c-client/master.svg?style=flat-square)](https://travis-ci.org/algernon/riemann-c-client)
[![Coverage Status](https://img.shields.io/coveralls/algernon/riemann-c-client.svg?style=flat-square)](https://coveralls.io/r/algernon/riemann-c-client)
[![Latest release](https://img.shields.io/github/release/algernon/riemann-c-client.svg?style=flat-square)](https://github.com/algernon/riemann-c-client/releases/latest)

This is a C client library for the [Riemann][riemann] monitoring
system, providing a convenient and simple API, high test coverage and
a copyleft license, along with API and ABI stability.

 [riemann]: http://riemann.io/

The library uses [semantic versioning][semver].

 [semver]: http://semver.org/

Features
--------

 * Sending events over TCP, TLS and UDP
 * Launching queries (TCP & TLS only)
 * Support for tags and attributes on events
 * Ability to send multiple events in a single message
 * Convenient and straightforward API (see the [API docs][api-docs]
   and the [demo](#demo) below!)
 * A comprehensive test suite
 * API and ABI stability (including symbol versioning on platforms
   where it is available).

 [api-docs]: https://algernon.github.io/riemann-c-client/

Installation
------------

The library follows the usual autotools way of installation:

    $ git clone git://github.com/algernon/riemann-c-client.git
    $ cd riemann-c-client
    $ autoreconf -i
    $ ./configure && make && make check && make install

For the build to succeed, one will need libtool 2.2+ (only if building
from a git checkout), the [protobuf-c compiler][protoc]. Optionally,
for TLS support, one needs [GnuTLS][gnutls] 2.8+, and to enable the
JSON output support in `riemann-client`, one also needs the
[json-c][json-c] library installed.

 [protoc]: http://protobuf-c.googlecode.com
 [gnutls]: http://www.gnutls.org/
 [json-c]: https://github.com/json-c/json-c/wiki

From this point onward, the library is installed and fully functional,
and one can use `pkg-config` to compile programs against it:

    ${CC} $(pkg-config --cflags --libs riemann-client) demo.c -o demo -Wall

If, for some reason the build fails, one may need to regenerate the
`protobuf-c-compiler` generated headers (changes in the compiler are
known to cause issues). To do this, do a `make distclean` first, and
then start over from `configure`.

To build the API documentation, one will have to have
[Doxygen](http://www.doxygen.org/) installed.

Demo
----

A simple program that sends a static event to [Riemann][riemann] is
included below. A few more useful programs are included in the
[src][src] directory of the source code.

 [src]: https://github.com/algernon/riemann-c-client/tree/master/src

```c
#include <riemann/riemann-client.h>
#include <riemann/simple.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
main (void)
{
  riemann_client_t *client;
  riemann_message_t *r;

  client = riemann_client_create (RIEMANN_CLIENT_TCP, "localhost", 5555);
  if (!client)
    {
      fprintf (stderr, "Error while connecting to Riemann: %s\n",
               strerror (errno));
      exit (EXIT_FAILURE);
    }

  r = riemann_communicate_event
    (client,
     RIEMANN_EVENT_FIELD_HOST, "localhost",
     RIEMANN_EVENT_FIELD_SERVICE, "demo-client",
     RIEMANN_EVENT_FIELD_STATE, "ok",
     RIEMANN_EVENT_FIELD_TAGS, "demo-client", "riemann-c-client", NULL,
     RIEMANN_EVENT_FIELD_NONE);

  if (!r)
    {
      fprintf (stderr, "Error while sending message: %s\n", strerror (errno));
      exit (EXIT_FAILURE);
    }

  if (r->ok != 1)
    {
      fprintf (stderr,  "Error communicating with Riemann: %s\n",
               (r->error) ? r->error : strerror (errno));
      exit (EXIT_FAILURE);
    }

  riemann_message_free (r);
  riemann_client_free (client);

  return EXIT_SUCCESS;
}
```

Why?
----

There already is an [existing library][gkos-riemann], and before
sitting down to write an independent one, I evaluated that one first.
Unfortunately, it failed on a few key points, like licensing, lack of
tests and promise of stability, sometimes awkward API, and so on and
so forth.

 [gkos-riemann]: https://github.com/gkos/riemann-c-client

I could have sent patches correcting these, but it was much easier to
just write a library that is designed from the ground up for an API I
find convenient and useful.

License
-------

Copyright (C) 2013, 2014, 2015 Gergely Nagy <algernon@madhouse-project.org>,
released under the terms of the
[GNU Lesser General Public License][lgpl], version 3+.

 [lgpl]: http://www.gnu.org/licenses/lgpl.html
