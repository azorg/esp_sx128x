#include <stdio.h>
#include <math.h>

int main()
{
  float fbw = 1625./1; // kHz
  unsigned bw = (unsigned) fbw;

  //int rs = (1<<22);
  //int rs = (int) (44.37334 * 100001); // 100km + 1m
  //int rs = 58000;
  int rs = 580000;

  // datasheet variant:
  double d1 = (((double) rs) * 150.) /
              (((double) (1 << 12) * fbw) / 1e3); // m

  // my idea:
  // note: ((1625 << 10) / 8000) = 208
  //       150 * 10 / 4 = 375
  int d2 = (((rs * 375) / 208) * (1625 / (int) bw)) / 8; // dm
  
  // simple variant:
  int d3 = (rs * 150) / ((((int) bw) << 12) / 1000); // m

  // from example source:
  int scale = (150 * 1000000 / 4096); // 36621.09375
  int d4 = rs * scale / ((int) rint(fbw * 1e3) / 100);

  printf("rs=%i d1=%.3fm d2=%idm d3=%im d4=%icm\n",
          rs, d1, d2, d3, d4);
  
  return 0;
}
