This is the source code for Minutor 2.1
web/ contains the source code for the web-based pack builder.  The live version
can be found at http://seancode.com/minutor/packs

The Makefile inside web/ will use the Closure Compiler to compile all the .js files
into a single editor.min.js.  To host the pack builder on your own website, you
only need editor.min.js, index.html, main.css, and the mods/ folder.

CONVENTIONS:
------------

The coding convetion is standardized on the result of Google's cpplint.
https://github.com/google/styleguide/tree/gh-pages/cpplint

We also use clang's static analyzer.  The options tested are in `check.sh`.


COMPILING:
---------

[Is described in the Wiki](https://github.com/mrkite/minutor/wiki/Self-Compile)
