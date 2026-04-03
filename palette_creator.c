// palette_creator.c
// by wendiner, 2026
// GNU General Public License v3.0
//
// Creates a palette of variable size by
// converting HSV values into RGB.
// Outputs a file named "palette.bin" with the
// following format:
//
// 00-06: "PALETTE"
// 07-0a: length (32-bit unsigned int, little-endian)
// 0b-0d: color values (RRGGBB 24-bit color)
// ...

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define numColors 256
#define startHue 50
#define endHue 0
#define startVal 0
#define endVal 100

unsigned int hsv_to_rgb(double hue, double sat, double val) {
  double c = val * sat;
  double x = c * (1 - fabs(fmod(hue / 60, 2) - 1));
  double m = val - c;

  double convertMatrix[][3] = {
    {c, x, 0},
    {x, c, 0},
    {0, c, x},
    {0, x, c},
    {x, 0, c},
    {c, 0, x}
  };

  double* rgb_prime = convertMatrix[(unsigned char) round(hue / 60)];

  unsigned char r = (unsigned char) ((rgb_prime[0] + m) * 255);
  unsigned char g = (unsigned char) ((rgb_prime[1] + m) * 255);
  unsigned char b = (unsigned char) ((rgb_prime[2] + m) * 255);

  printf("%i,%i,%i\n", r, g, b);
  printf("%lf, %lf, %lf\n", c, x, m);

  unsigned int output = 0;

  output |= r;
  output |= g * 256;
  output |= b * 65536;

  return output;
}

int main() {
  unsigned int rgb = hsv_to_rgb(235, 0.66, 0.58);
  FILE* fp = fopen("test.bin", "w");
  fwrite(&rgb, sizeof(unsigned int), 1, fp);
  fclose(fp);
  return 0;
}
