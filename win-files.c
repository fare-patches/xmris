#include <errno.h>
#include <windows.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>
#include "win-files.h"

char *win_pref_file(const char *name)
{
  char *base = SDL_GetPrefPath(NULL, "xmris");
  if (!base) return NULL;
  char *path = malloc(strlen(base) + strlen(name) + 1);
  if (path) sprintf(path, "%s%s", base, name);
  SDL_free(base);
  return path;
}

int win_lock(FILE *stream, int acquire)
{
  HANDLE file = (HANDLE)_get_osfhandle(_fileno(stream));
  OVERLAPPED offset = {0};
  BOOL ok = acquire ? LockFileEx(file, LOCKFILE_EXCLUSIVE_LOCK, 0,
                                 MAXDWORD, MAXDWORD, &offset)
                    : UnlockFileEx(file, 0, MAXDWORD, MAXDWORD, &offset);
  if (!ok) errno = EIO;
  return ok ? 0 : -1;
}

int win_replace(const char *source, const char *destination)
{
  return MoveFileExA(source, destination, MOVEFILE_REPLACE_EXISTING) != 0;
}
