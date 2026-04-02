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

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("%s: missing arguments\n", argv[0]);
    return 1;
  }

  const char* atoiError;
  const unsigned int numColors = strtonum(argv[1], 2, 4294967295, &atoiError);
  if (atoiError != NULL) {
    printf("%s: %s\n", argv[0], atoiError);
    return 1;
  }

  printf("%i\n", numColors);

  return 0;
}
