#include "xmris.h"
#include "sound.h"
#include <sys/stat.h>
#ifdef _WIN32
#include "win-files.h"
#else
#include <sys/file.h>
#endif
#include <fcntl.h>

/* Store only preferences explicitly changed in the menu. */
static const char *const names[] = {"effects", "music", "diamondDifficulty",
  "up", "down", "left", "right", "throw", "pause", "quit", "iconize", "keyboard"
};
static char values[XtNumber(names)][128];

static char *config_path(int create)
{
#ifdef _WIN32
  (void)create;
  return win_pref_file("config");
#else
#if defined(SDL_PLATFORM_IOS)
  const char *base = SDL_GetUserFolder(SDL_FOLDER_DOCUMENTS);
#elif defined(__ANDROID__)
  const char *base = SDL_GetAndroidInternalStoragePath();
  if (!base) return NULL;
#else
  const char *base = getenv("XDG_CONFIG_HOME");
#endif
  const char *parent = "";
  if (!base || *base != '/') {
    base = getenv("HOME");
    if (!base || !*base) return NULL;
    parent = "/.config";
  }
  char *path = malloc(strlen(base) + strlen(parent) + sizeof("/xmris/config.XXXXXX"));
  if (!path) return NULL;
  sprintf(path, "%s%s", base, parent);
  if (create && mkdir(path,0700) && errno != EEXIST) goto failed;
  strcat(path,"/xmris");
  if (create && mkdir(path,0700) && errno != EEXIST) goto failed;
  strcat(path,"/config");
  return path;
failed:
  free(path);
  return NULL;
#endif
}

void options_read(void)
{
  char *path = config_path(0), line[256];
  FILE *file = path ? fopen(path,"r") : NULL;
  free(path);
  memset(values,0,sizeof(values));
  if (!file) return;
  while (fgets(line,sizeof(line),file)) {
    char *value = strchr(line,'=');
    if (!value) continue;
    *value++ = 0;
    value[strcspn(value,"\r\n")] = 0;
    if (strlen(value) >= sizeof(values[0])) continue;
    for (unsigned i=0;i<XtNumber(names);++i)
      if (!strcmp(names[i],line)) strcpy(values[i],value);
  }
  fclose(file);
}

const char *option_default(const char *name)
{
  for (unsigned i=0;i<XtNumber(names);++i)
    if (!strcmp(name,names[i]) && values[i][0]) return values[i];
  return NULL;
}

int options_save(int which)
{
  char *path = config_path(1);
  if (!path) return 0;
  char *tmp = malloc(strlen(path)+8);
  if (!tmp) { free(path); return 0; }
#ifdef _WIN32
  char *lock_path = win_pref_file("config.lock");
  FILE *lock = lock_path ? fopen(lock_path,"a+b") : NULL;
  free(lock_path);
  if (!lock || win_lock(lock,1)) {
    if (lock) fclose(lock);
    free(tmp);
    free(path);
    return 0;
  }
  sprintf(tmp,"%s.tmp",path);
#else
  sprintf(tmp,"%s.XXXXXX",path);
  /* Serialize read-modify-write across running instances. The directory
   * stays in place while the preferences file is atomically replaced. */
  char *slash = strrchr(path,'/');
  *slash = 0;
  int directory = open(path,O_RDONLY | O_DIRECTORY);
  *slash = '/';
  if (directory < 0 || flock(directory,LOCK_EX) < 0) {
    if (directory >= 0) close(directory);
    free(tmp);
    free(path);
    return 0;
  }
#endif
  options_read();
  if (which==0) strcpy(values[0],data.effects ? "true" : "false");
  if (which==1) strcpy(values[1],data.music ? "true" : "false");
  if (which==2) strcpy(values[2],data.diamond_mask==0 ? "classic" : data.diamond_mask==7 ? "hard" : "easy");
  if (which==3)
    for (unsigned i=0;i<KEYS;++i) {
      const char *key = portable_key_name(data.keysyms[i]);
      if (key) snprintf(values[3+i],sizeof(values[0]),"%s",key);
    }
#ifdef _WIN32
  FILE *file = fopen(tmp,"wb");
#else
  int fd = mkstemp(tmp);
  FILE *file = fd<0 ? NULL : fdopen(fd,"w");
#endif
  int ok = file != NULL;
  if (file) {
    for (unsigned i=0;i<XtNumber(names);++i)
      if (values[i][0] && fprintf(file,"%s=%s\n",names[i],values[i])<0) ok=0;
    if (fclose(file)) ok=0;
  }
#ifdef _WIN32
  if (ok) ok = win_replace(tmp,path);
#else
  else if (fd>=0) close(fd);
  if (ok) ok = rename(tmp,path)==0;
#endif
  if (!ok) { perror(path); unlink(tmp); }
  free(tmp);
  free(path);
#ifdef _WIN32
  fclose(lock);
#else
  close(directory);
#endif
  return ok;
}
int sound_effects_enabled(void) { return data.effects; }
