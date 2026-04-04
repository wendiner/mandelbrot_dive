// palette_creator.c
// by wendiner, 2026
// GNU General Public License v3.0
//
// Creates a gradient palette of variable size
// (second argument) by interpolating HSV colors
// and them converting them to RGB. The output
// is written to a file (first argument) with
// the following format:
//
// 00-06: "PALETTE"
// 07-0a: length (32-bit unsigned int, little-endian)
// 0b-0e: color values (RRGGBBAA 32-bit color)
// ...

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// HSV keyframes
#define hsv_len 4
const double colors[][3] = {
  {240, 1.0, 0.6}, // blue
  {60, 1.0, 1.0}, // yellow
  {0, 1.0, 1.0}, // red
  {0, 0, 0} // black
};

// converts HSV (plus opacity) to an RRGGBBAA packed int
unsigned int hsv_to_rgb(double hue, double sat, double val, double alpha) {
  const double c = val * sat;
  const double x = c * (1 - fabs(fmod(hue / 60, 2) - 1));
  const double m = val - c;

  const double convertMatrix[][3] = {
    {c, x, 0},
    {x, c, 0},
    {0, c, x},
    {0, x, c},
    {x, 0, c},
    {c, 0, x}
  };

  const double* rgb_prime = convertMatrix[(unsigned char) floor(hue / 60)];

  const unsigned char r = (unsigned char) ((rgb_prime[0] + m) * 255);
  const unsigned char g = (unsigned char) ((rgb_prime[1] + m) * 255);
  const unsigned char b = (unsigned char) ((rgb_prime[2] + m) * 255);

  unsigned int output = 0;

  output |= r;
  output |= g * 256;
  output |= b * 65536;
  output |= (char) (alpha * 255) * 16777216;

  return output;
}

int main(int argc, char** argv) {
  if (argc < 3) {
    printf("missing arguments\n");
    return 1;
  }

  FILE* fp = fopen(argv[1], "w");
  if (!fp) {
    printf("failed to open file\n");
    return 1;
  }

  unsigned int numColors = atoi(argv[2]); // how granular is the gradient?

  fprintf(fp, "PALETTE"); // magic number
  fwrite(&numColors, sizeof(unsigned int), 1, fp); // number of colors

  for (unsigned int i = 0; i < numColors; i++) {
    const double position = (double) i / (double) numColors * (double) (hsv_len - 1); // current position in gradient, scaled to hsv keyframes
    const double midGrad = fmod(position, 1); // position between previous and next keyframe
    const double* prevColor = colors[(unsigned int) floor(position)]; // previous hsv keyframe
    const double* nextColor = colors[(unsigned int) ceil(position)]; // next hsv keyframe

    const double hsv[3] = { // interpolate between the two neighboring keyframes
      midGrad * (nextColor[0] - prevColor[0]) + prevColor[0],
      midGrad * (nextColor[1] - prevColor[1]) + prevColor[1],
      midGrad * (nextColor[2] - prevColor[2]) + prevColor[2]
    };

    unsigned int rgb = hsv_to_rgb(hsv[0], hsv[1], hsv[2], 1.0); // convert the interpolation into rgb

    fwrite(&rgb, sizeof(unsigned int), 1, fp);
  }
  
  fclose(fp);
  return 0;
}
