#define CHAR_W 8
#define CHAR_H 8

static const char* num[] = {
  "  ****  "
  " **  ** "
  " **  ** "
  " **  ** "
  " **  ** "
  " **  ** "
  " **  ** "
  "  ****  ",

  "    **  "
  "   ***  "
  "  ****  "
  "    **  "
  "    **  "
  "    **  "
  "    **  "
  "   **** ",

  "  ****  "
  " **  ** "
  "     ** "
  "    **  "
  "   **   "
  "  **    "
  " **   * "
  " ****** ",

  "  ****  "
  " **  ** "
  "     ** "
  "    **  "
  "     ** "
  "     ** "
  " **  ** "
  "  ****  ",

  "     ** "
  "    *** "
  "   **** "
  "  ** ** "
  " **  ** "
  " *******"
  "     ** "
  "     ** ",

  " ****** "
  " **   * "
  " **     "
  " *****  "
  "     ** "
  "     ** "
  " **  ** "
  "  ****  ",

  "  ****  "
  " **  ** "
  " **     "
  " *****  "
  " **  ** "
  " **  ** "
  " **  ** "
  "  ****  ",

  " ****** "
  " *   ** "
  "     ** "
  "    **  "
  "    **  "
  "   **   "
  "   **   "
  "  **    ",

  "  ****  "
  " **  ** "
  " **  ** "
  "  ****  "
  " **  ** "
  " **  ** "
  " **  ** "
  "  ****  ",

  "  ****  "
  " **  ** "
  " **  ** "
  "  ***** "
  "     ** "
  "     ** "
  " **  ** "
  "  ****  ",
};

void
render (char* buf, int n)
{
  int col;
  int row;
  const char *ch = num[n];
  
  for (col = 0; col < CHAR_W; ++col) {
    buf[col] = 0;
    for (row = 0; row < CHAR_H; ++row) {
      int pixel = ch[row * CHAR_W + col];
      buf[col] |= (pixel != ' ') << row;
    }
  }
}

#ifdef TEST
#include <stdio.h>

int
main ()
{
  int i, row, col;
  char buf[CHAR_W * 10];

  for (i = 0; i < 10; ++i)
    render (buf + CHAR_W * i, i);

  for (row = 0; row < CHAR_H; ++row) {
    for (i = 0; i < 10; ++i) {
      for (col = 0; col < CHAR_W; ++col) {
        printf ("%c", (buf[col + i * CHAR_W] & (1 << row))? '*' : ' ');
      }
    }
    printf ("\n");
  }
}

#endif
