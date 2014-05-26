bootstrap.c+web
===============

`bootstrap.c+web` is a template for simple projects written in C with
autotools and exposing a web API. It is however mostly an
experiment. Can it be as useful as projects like
[HTML5 Boilerplate][]?

Here are the features available:

 - REST endpoint
 - SSE endpoint
 - websocket endpoint
 - serve static files
 - build system based on Grunt for client-side JS stuff
 - Angular.js web app boilerplate

The client side is not the best template that you will find. It lacks
unit tests integration. You may prefer to use something based on
[Yeoman][]-generated templates. Just have a look at the `Makefile.am`
which shows the integration. However, it is fairly complete and has
most features that you would need.

It is derived from a [similar project][] without the web part.

[HTML5 Boilerplate]: http://html5boilerplate.com/
[similar project]: https://github.com/vincentbernat/bootstrap.c
[Yeoman]: http://yeoman.io/

Usage
-----

You need [cookiecutter][], a tool to create projects from project
templates. Once installed (in a virtualenv or just with `pip install
cookiecutter`), you can use the following command:

    cookiecutter https://github.com/vincentbernat/bootstrap.c.git
    cd your-project
    git init
    git add .
    git commit -m "Initial commit"

[cookiecutter]: https://github.com/audreyr/cookiecutter

The small prefix is used to prefix function and structure names. The
default value `bsw` stands for `bootstrap+web`.

Then, use the following command to get the first steps to get started:

    git ls-tree -r --name-only HEAD | \
          xargs grep -nH "T[O]DO:" | \
          sed 's/\([^:]*:[^:]*\):\(.*\)T[O]DO:\(.*\)/\3 (\1)/' | \
          sort -ns | \
          awk '(last != $1) {print ""} {last=$1 ; print}'

Once you are done, your project is ready and you can compile it with
and get a release tarball with:

    sh autogen.sh
    mkdir build
    cd build
    ../configure
    make
    make dist

Once you want to make a release, tag the tree with `git tag 1.3`, then
run the previous commands from the top. You'll get a properly
versioned tarball with a `ChangeLog` file if this is not your first
version.

API
---

There are three API endpoints:

 - a simple HTTP REST endpoing (`/api/1.0/hello`)
 - a SSE enabled endpoint (`/api/1.0/sse`)
 - a websocket endpoint (`/api/1.0/ws`)

They are meant as example and you should modify them.
