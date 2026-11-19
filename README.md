# XMRIS in 2026

xmris (1992-1995) by Nathan Sidwell is a remarkably faithful Unix clone of
Mr. Do!, a 1982 game for 8-bit consoles by Kazutoshi Ueda.
https://en.wikipedia.org/wiki/Mr._Do!
https://www.retrogames.cz/play_273-SNES.php
https://www.arcadearchives.com/en/title/aca-410/
https://www.youtube.com/watch?v=hv2B-p1p8OQ

I (Faré Rideau) am amazed at how much patience Nathan had in precisely
re-creating the behavior of the original game, and
I wonder whether he reverse-engineered the code or
just inferred the rules by observation: the game has plenty of fun
corner cases that interact very well with each other,
many not obvious to the beginner (shooting through walls,
dying and gaining a life at the same time, etc.),
that Nathan perfectly reproduced.

## Building an Running XMRIS

### Prerequisites

On Ubuntu, I needed to install `xmkmf` from xutils, the Athena widgets,
and the standard 75dpi bitmap fonts.
Extensions I had Codex write for me require XRender and SDL:
```
apt install -y xutils-dev libxaw7-dev xfonts-75dpi libxrender-dev libsdl2-dev
```

If you want the fonts to work in the same X11 session, you might additionally need to:
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

### To Install
```
make install
make install.man
```

### To Run
```
xmris
```

## Features added 2026

In 2026, I used AIs to add these features that were missing in 1992.

The game now automatically uses the largest whole-number pixel scale
that fits its window, and stays centered as you resize.
You can use an X resource `Xmris.scale: 3` or `Xmris.scale: auto`
to confirm or override the default (`auto`).
You can use the command-line option `-scale 2` to lock it at 2×,
or `-scale auto`, to override the default from the X resource.
This feature was not needed in 1992, when a 436×539 window
(including 1 pixel border each side) was actually bigger
than most people's displays used to be.

The game will also center its display within the window if the user
or window manager oversizes it. This was also not a problem in 1992,
but it is one for me using stumpwm in 2026.

The original X version lacked the classic Mr Do! sound
(do re mi fa sol la si do) as you eat cherries,
as well as the melodies that accompanied the game and various events.
I suppose sound was not generally available on Unix in 1992.
I added game audio, and it is enabled by default.
Set `Xmris.sound: false` in your X resources for quiet play,
or run `xmris -nosound`.
Use `-sound` to enable audio for a single session.
For effects and fanfares without the looping soundtrack,
use `-nomusic` or `Xmris.music: false`.
In xmris, the music accelerates when the normal monsters speed up;
it changes when the center prize releases the drones.

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
* [x] Integer factor window scaling.
* [x] Adding the do re mi fa sol la si do sounds as you eat cherries as in Mr Do!
* [ ] Fullscreen scaling.
* [ ] Porting to other platforms beside X11.
* [ ] Running in a TEE from a deterministic build so as to maintain
      a cheat-resistant global high score chart.
