# XMRIS 4.1.1 in 2026

xmris (1992-1995) by Nathan Sidwell is a remarkably faithful Unix clone of
Mr. Do!, a 1982 game for 8-bit consoles by Kazutoshi Ueda.
https://en.wikipedia.org/wiki/Mr._Do!
https://www.retrogames.cz/play_273-SNES.php
https://www.arcadearchives.com/en/title/aca-410/
https://www.youtube.com/watch?v=hv2B-p1p8OQ
https://www.planetemu.net/en/rom/mame-roms-merged/mrdo

I (Faré Rideau) am amazed at how much patience Nathan had in precisely
re-creating the behavior of the original game, and
I wonder whether he reverse-engineered the code or
just inferred the rules by observation: the game has plenty of fun
corner cases that interact very well with each other,
many not obvious to the beginner (shooting through walls,
dying and gaining a life at the same time, etc.),
that Nathan perfectly reproduced.

2026 revival and SDL port maintained by François-René (Faré) Rideau.

## What's new in 4.1.1

* The Linux SDL3 frontend shares the X11 game's rules, gardens, scores, and music.
  Press F11 for fullscreen. Integer scaling is the default. Use
  `-scale fit` for fractional scaling that preserves the aspect ratio.
* Press F1 at the title screen in either frontend to change sound effects,
  music, diamond difficulty, or controls. Both frontends share saved preferences.
  The Controls screen shows current bindings and can restore the defaults.
* SDL uses the classic X11 Courier bitmap font. Both games identify themselves
  as `xmris` in the audio mixer.

## Building and running XMRIS

Choose a frontend. For the SDL3 game on Ubuntu, install its build dependencies:

```sh
sudo apt install build-essential pkg-config libsdl3-dev
make -f Makefile.sdl
./xmris-sdl -fullscreen
```

### X11 prerequisites

On Ubuntu, I needed to install `xmkmf` from xutils, the Athena widgets,
and the standard 75dpi bitmap fonts.
The X11 game uses XRender for scaling and SDL2 for sound:
```
sudo apt install build-essential xutils-dev libxaw7-dev xfonts-75dpi libxrender-dev libsdl2-dev
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

### SDL3 on Linux

The SDL frontend builds from the same game, garden, score, and sound sources.
F11 toggles fullscreen. The SDL window centers the garden and scales it in
whole pixels by default. Use `-scale fit` to fill as much of the window as
possible at the original aspect ratio, including in fullscreen. Use `-scale 2` to request a 2× initial window.
Arrow keys move, Space starts or throws, and K on the title screen opens Options. Run `./xmris-sdl -help` for the supported options. The SDL
frontend uses the bundled 18-pixel X11 Courier bitmap font and command-line settings. The X11 game
and the xmred garden editor use the existing imake build and X resources.

### Game options

Press **F1** at the title screen in either frontend. The original **K**
shortcut also works until rebound. The menus always use the arrow keys,
Enter, and Esc. You can toggle sound
effects and background music independently, choose Easy, Hard, or Classic
diamond behavior, or open Controls to review your bindings. Select an action and press Enter
to change just that key. Restore defaults resets the working copy. Save and
return applies your edits, while Esc discards them. Esc returns to the title.
Changes are saved under `$XDG_CONFIG_HOME/xmris/config`, which defaults
to `~/.config/xmris/config`. Command-line settings and explicit X resources
take precedence at startup. The `-nosound` switch disables the audio device
for the whole session.

### Linux score files

Both frontends keep personal scores in `~/.xmris.score`. For a shared
leaderboard with a setgid installation, the administrator must pre-create
`/var/games/xmris.score` as a regular file writable by the game group.
Keep the containing directory under administrator control. The game opens
the shared file before dropping privileges and can use personal scores
when the shared file is unavailable.

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

`Xmris.diamondDifficulty` controls the chance that an eligible split apple
reveals a diamond. The default, `easy`, gives a 1 in 512 chance. Use `hard`
for 1 in 2048. Both allow another diamond after collecting one. The historical
`classic` setting gives a 1 in 256 chance and allows one collection per game.

The chase music speeds up by 50% when the nearest enemy's distance falls to
80 arcade pixels or less, and slows down again at 113 or more (16 pixels per
cell). Between those thresholds it keeps its tempo. xmris measures tunnel
distance for ordinary monsters and direct separation for digging monsters.

In the arcade, the ball starts nine pixels ahead of the player, so a throw
can clear a thin wall. xmris supports the same trick, but checks how far the
hedge has been dug and can refuse a shot the arcade would launch.

I also restored some behavior from the arcade game: there is a .4 second
grace period after eating a cherry to continue a streak, and the last
cherry in the streak is 500 points total, not 550 (with 500 plus regular 50).

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

Solution: install and configure the good old fonts.

## TODO

If you want to make this game better, have an AI add the following features:
* [x] Integer factor window scaling.
* [x] Adding the do re mi fa sol la si do sounds as you eat cherries as in Mr Do!
* [ ] Fullscreen scaling.
* [ ] Porting to other platforms beside X11.
* [ ] Running in a TEE from a deterministic build so as to maintain
      a cheat-resistant global high score chart.
