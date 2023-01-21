# xpac
This is a salvaged source of xpacman by Peter Warden, downloaded from the 
ibiblio mirror. Copied and relicensed with the attempt at preserving this
old piece of Linux-related entertainment. Renamed *just-in-case* in order
to appease the copyright goblins.

Not much development of this antique code is planned. Mostly the boilerplate
for git, build and packaging.

## Branches

* `orig` contains the original source, fixed only to build itself
* `main` is the upstream by the new maintainer

## Links

Downloaded from [Original Source Mirror](https://www.ibiblio.org/pub/Linux/games/arcade/pacman/).

## How to run 
Some forms of X11, CC and make are required. Other than that, just go:

```
git clone git@github.com:kamiljdudek/xpac
cd xpac
make
sudo make install
xpac

```

## TODO
* `configure` script
* `.desktop` menu shortcut files
* `debuginfo` package
* Translations
* Icon pixmap
* Remove all implicit casts
