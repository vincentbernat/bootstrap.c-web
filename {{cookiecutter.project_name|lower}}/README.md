{{cookiecutter.project_name}}
=============================

{{cookiecutter.project_description}}

Installation
------------

Execute the following commands:

    $ ./configure
    $ make
    $ sudo make install

The client part needs to be compiled while inside `web/`:

    $ npm install -g grunt
    $ npm install
    $ bower install
    $ grunt

To run the server for development:

    $ grunt serve

Development
-----------

The client part can be updated with:

    $ npm update <!-- small updates -->
    $ npm outdated --depth=0
    $ npm install something@latest --save-dev

Use
---

Most of the documentation is available in the manual page
`{{cookiecutter.project_name|lower}}(8)`.

# TODO:6000 Update the README.md file with a complete description
# TODO:6000 and some usage instructions.
