#include "xmris.h"
#include <X11/extensions/Xrender.h>

static Picture source_picture, window_picture;

/* Return false if this visual cannot scale; auto mode can still use 1x. */
int configure_presentation(int scale)
{
  XFixed one = XDoubleToFixed(1);
  XTransform transform = {{{one, 0, 0}, {0, one, 0}, {0, 0, scale * one}}};
  static int unavailable;

  if(scale == 1)
    return 1;
  if(unavailable)
    return 0;
  if(!source_picture)
    {
      int major, minor;
      XRenderPictFormat *format;

      if(!XRenderQueryVersion(display.display, &major, &minor) ||
          (major == 0 && minor < 6) ||
          !(format = XRenderFindVisualFormat(display.display, display.visual)))
        {
          unavailable = 1;
          return 0;
        }
      source_picture = XRenderCreatePicture(display.display, display.copy, format, 0, NULL);
      window_picture = XRenderCreatePicture(display.display, display.window, format, 0, NULL);
      XRenderSetPictureFilter(display.display, source_picture, FilterNearest, NULL, 0);
    }
  XRenderSetPictureTransform(display.display, source_picture, &transform);
  return 1;
}

/* All game coordinates and off-screen pixmaps stay at their native size. */
void present_area(int x, int y, unsigned width, unsigned height)
{
  int scale = data.pixel_scale;
  if(scale == 1)
    XCopyArea(display.display, display.copy, display.window, GCN(GC_COPY),
        x, y, width, height, x, y);
  else if(width && height)
    XRenderComposite(display.display, PictOpSrc, source_picture, None, window_picture,
        scale * x, scale * y, 0, 0, scale * x, scale * y, scale * width, scale * height);
}

void present_load(int left, int right)
{
  int scale = data.pixel_scale;
  XFillRectangle(display.display, display.window, GCN(GC_LOAD),
      left * scale, PIXELY(CELLS_DOWN, CELL_HEIGHT) * scale,
      (right - left + 1) * scale, scale);
}

void video_copy(Pixmap src, Pixmap dst, GC gc, int x, int y, unsigned w, unsigned h, int dx, int dy)
{
  XCopyArea(display.display, src, dst, gc, x, y, w, h, dx, dy);
}

void video_fill(Pixmap dst, GC gc, int x, int y, unsigned w, unsigned h)
{
  XFillRectangle(display.display, dst, gc, x, y, w, h);
}

void video_line(Pixmap dst, GC gc, int x, int y, int dx, int dy)
{
  XDrawLine(display.display, dst, gc, x, y, dx, dy);
}

void video_rect(Pixmap dst, GC gc, int x, int y, unsigned w, unsigned h)
{
  XDrawRectangle(display.display, dst, gc, x, y, w, h);
}

void video_text(Pixmap dst, GC gc, int x, int y, const char *str, int n)
{
  XDrawString(display.display, dst, gc, x, y, str, n);
}

void video_opaque_text(Pixmap dst, GC gc, int x, int y, const char *str, int n)
{
  XDrawImageString(display.display, dst, gc, x, y, str, n);
}


char *key_name(KeySym key) { return XKeysymToString(key); }
void video_sync(void) { XSync(display.display, False); }
void video_pattern(unsigned long foreground, unsigned long background, Pixmap stipple)
{
  XGCValues values;
  values.foreground = foreground;
  values.background = background;
  values.stipple = stipple;
  values.fill_style = FillOpaqueStippled;
  XChangeGC(display.display, GCN(GC_BOARD),
      GCFillStyle | GCForeground | GCBackground | GCStipple, &values);
}
char *garden_resource(const char *name, const char *suffix)
{
  char *resource = XtResolvePathname(display.display, "app-defaults/xmris",
      name, suffix, NULL, NULL, 0, NULL);
  char *path = resource ? strdup(resource) : NULL;
  XtFree(resource);
  return path;
}
