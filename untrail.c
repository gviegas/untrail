/* gviegas 2023/mar. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define fatal(reason) { printf("fatal: %s: %s\n", __func__, reason); exit(1); }

void
readend(const char *name, void **dst, size_t *n)
{
  FILE *f;
  long sz;
  void *p;
  int r;

  if (!dst || !n)
    fatal("NULL param");

  f = fopen(name, "r");
  if (!f)
    fatal("fopen failed");

  r = fseek(f, 0, SEEK_END);
  if (r == -1)
    fatal("fseek failed");

  sz = ftell(f);
  rewind(f);

  if (sz == 0) {
    *dst = NULL;
    *n = 0;
    fclose(f);
    return;
  }

  p = malloc(sz + 1);
  if (!p)
    fatal("malloc failed");

  r = fread(p, 1, sz, f);
  if (r != sz)
    fatal("fread failed");

  fclose(f);
  ((char *)p)[sz] = 0;
  *dst = p;
  *n = sz + 1;
}

int
isws(char c)
{
  return c == ' ' || c == '\t';
}

size_t
untrail(void *p, size_t *n)
{
  char *c, *d, *e;
  size_t diff;

  c = (char *)p + *n;
  e = c;
  diff = *n;

  while ((void *)c > p) {
    d = c - 1;
    while (d != p && isws(*d)) {
      d--;
      (*n)--;
    }

    if (c-d > 1) {
      memmove(d+1, c, e-c);
      e -= c - d - 1;
    }

    c = d;
    while (c != p && *c != '\n')
      c--;
  }

  diff -= *n;
  if (diff > 1 && isws(*(char *)p) && ((char *)p)[1] == '\n') {
    diff++;
    (*n)--;
    memmove(p, (char *)p+1, *n);
  }

  return diff;
}

void
writef(const char *name, const void *p, size_t n)
{
  FILE *f;
  size_t nw;

  f = fopen(name, "w");
  if (!f)
    fatal("fopen failed");

  nw = fwrite(p, 1, n, f);
  if (nw != n)
    fatal("fwrite failed");

  fclose(f);
}

int
main(int argc, char *argv[])
{
  void *p;
  size_t n, diff;

  if (argc < 2) {
    printf("usage: %s in [out]\n"
      "\n  This removes blank characters that precede a new line.\n",
      argv[0]);
    return 0;
  }

  readend(argv[1], &p, &n);
  diff = untrail(p, &n);
  n--;

  if (diff == 0)
    puts("nothing to do");
  else if (argc < 3)
    printf("---- %s\n\n%s\n---- %zu byte%s (removed %zu)\n",
      argv[1], (char *)p, n, n>1?"s":"", diff);
  else {
    writef(argv[2], p, n);
    printf("%zu byte%s (removed %zu)\nwrote result to %s\n",
      n, n>1?"s":"", diff, argv[2]);
  }

  free(p);
}
