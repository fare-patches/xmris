# XMRIS in 2026

xmris (1992-1995) by Nathan Sidwell is a remarkably faithful Unix clone of Mr. Do!,
a 1982 game for 8-bit consoles by Kazutoshi Ueda.
https://en.wikipedia.org/wiki/Mr._Do!
https://www.retrogames.cz/play_273-SNES.php
Makes you wonder with how much patience Nathan reverse-engineered the behavior,
vs whether he reverse-engineered the code:
the game has plenty of fun corner cases that interact very well with each other,
some not obvious to the beginner (shooting through walls, dying and gaining a life
at the same time, etc.), that Nathan perfectly reproduced. The original X version lacked the sound (do re mi fa sol la si do) as you eat cherries.
I suppose sound was not generally available on Unix in 1992.

## Building an Running XMRIS

### Prerequisites

On Ubuntu, I needed to install `xmkmf`, the Athena widgets and the standard 75dpi bitmap fonts:
```
apt install -y xutils-dev libxaw7-dev libxrender-dev libsdl2-dev xfonts-75dpi
```

If you want the fonts to work the same X11 session, you might additionally need to:
```
xset fp+ /usr/share/fonts/X11/75dpi
xset fp rehash
```

The rpmfusion docs suggest you also need to install the 100dpi bitmap fonts
(debian package `xfonts-100dpi`), but in my experience they are not necessary to run the game.

### To Build
```
xmkmf
make
```

### To Check That It Runs
```
./xmris
```

Use `./xmris -scale 2` to double the game window with crisp pixels. Cherry sounds are enabled by default. Set
`Xmris.sound: false` in your X resources for quiet play, or run `xmris -nosound`.
Use `-sound` to enable audio for a single session.

### To Install
```
make install
make install.man
```

### To Run
```
xmris
```

## Provenance

This repository was initialized with the contents of
[the xmris 4.0.5 tarball (2008) from sourceforge](https://sourceforge.net/projects/xmris/files/xmris/4.05/),
plus [patches from rpmfusion](https://github.com/rpmfusion/xmris/)
by Hans de Goede <hdegoede@redhat.com> (2010-2011) (also include a rpm spec and icons).

## Troubleshooting

### Missing the 75dpi X fonts
```
X Error of failed request:  BadName (named color or font does not exist)
  Major opcode of failed request:  45 (X_OpenFont)
  Serial number of failed request:  10
  Current serial number in output stream:  11
Maybe we need to install additional X packages on the server side?
```

## TODO

If you want to make this game better, have an AI add the following features:
* [ ] Fullscreen scaling.
* [x] Adding the do re mi fa sol la si do sounds as you eat cherries as in Mr Do!
* [ ] Porting to other platforms beside X11.
* [ ] Running in a TEE from a deterministic build so as to maintain
      a cheat-resistant global high score chart.
