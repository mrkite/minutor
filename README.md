
20190304:by hezhenwei
finally able to show most of part with new nbt.
still:
1.biomes not so correct.
3.not all blocks tranlated.

======== original ===========


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

All Platforms:
Use QtCreator (Qt5 version) and open minutor.pro


How to do a static compile on Windows:
-------------------------------------

Download the qt5.5 sourcecode.

Unzip it whereever you wish, it's a large file and contains a lot of nested
subdirectories, so you'll probably want to put it in <samp>C:\Qt5\src</samp> or
something similar since you could end up running into Windows' path-length
limitations otherwise.

Now edit <samp>qtbase\mkspecs\common\msvc-desktop.conf</samp>

Find the CONFIG line and remove `embed_manifest_dll` and `embed_manifest_exe`
from that line.

Next find `QMAKE_CFLAGS_*` and change `-MD` to `-MT` and `-MDd` to `-MTd`.

Open your developer command prompt (64-bit), cd into the qtbase folder and
run:

```bat
configure -prefix %CD% -debug-and-release -opensource -confirm-license
	-platform win32-msvc2013 -nomake tests -nomake examples
	-opengl desktop -static
nmake
```

If nmake complains about python or perl, install ActivePerl and ActivePython and
try again.  This compile will take a long time.

This should make a static Qt5 with both debug and release libraries.  Now in
QtCreator, go to Tools → Options... and select Qt Versions from Build & Run.
Add a new Qt Version and locate the `qmake.exe` that is inside
<samp>qtbase\bin</samp> of the Qt5 you just compiled.
There will be a warning flag because we didn't compile qmlscene or qmlviewer
or any helpers.  You can ignore that warning.

Then switch over to Kits and make a new kit that uses the Qt version you just
created.  Again, there will be a warning flag for the same reasons as before,
ignore it.

Now compile Minutor using the static Kit.  You should end up with a statically
linked minutor.exe which doesn't require any dlls to run.


Building for Linux:
------------------

Use qmake to generate a makefile then run make.  Or use QtCreator.

If you want to make a .deb package,

```console
$ debuild
```

To make a package for another distribution:

```console
$ pbuilder-dist vivid create   # called only once to generate environment
$ debuild -S -us -uc
$ cd ..
$ pbuilder-dist vivid build *.dsc
```


Building on OSX:
----------------

Make a static compile of Qt 5.5:


```console
$ git clone https://code.qt.io/qt/qt5.git
$ cd qt5
$ perl init-repository --module-subset=default,-qtwebkit,-qtwebkit-examples,-qtwebengine
(wait forever)
$ git checkout 5.5
$ ./configure -prefix $PWD -opensource -confirm-license -nomake tests -nomake
examples -release -static
$ make
(wait forever)
```

Then compile Minutor:

```console
$ cd minutor
$ ~/qt5/qtbase/bin/qmake
$ make
```

You'll end up with a minutor.app in the current directory.
