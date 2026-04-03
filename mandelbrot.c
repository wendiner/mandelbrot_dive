// Mandelbrot Dive
// by wendiner, 2026
// GNU General Public License v3.0
// Runs on Allegro 5
// https://github.com/liballeg/allegro5
//
// Incomplete and incredibly unoptimized Mandelbrot
// Set "dive" program. Uses multithreading.
// The first real program I've made with Allegro.


#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define nproc 12
#define dispWidth 640
#define dispHeight 480
#define startX -0.343806077
#define startY -0.61127804
#define desiredZoom 30000
#define maxFrames 60000000

#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>

unsigned int testPoint(double x, double y, unsigned int maxDepth) { // check if a point is within the set
  double z[] = {0.0, 0.0};
  double c[] = {x, y};
  double* z_hist = (double*) malloc((maxDepth + 3) * 2 * sizeof(double)); // so we can check for repeats

  unsigned int iterations = 0;
  while (iterations < maxDepth && pow(z[0], 2) + pow(z[1], 2) <= 4) {
    iterations++;
    double old_z[] = {z[0], z[1]};

    z[0] = pow(old_z[0], 2) - pow(old_z[1], 2) + c[0];
    z[1] = 2 * old_z[0] * old_z[1] + c[1];
    
    bool repeat = false;
    for (int i = 0; i <= maxDepth; i++) // checking for repeats
      if (z_hist[i * 2] == z[0] && z_hist[i * 2 + 1] == z[1]) {
        iterations = maxDepth;
        repeat = true;
      }

    if (repeat)
      break;

    z_hist[iterations * 2] = z[0];
    z_hist[iterations * 2 + 1] = z[1];
  }
  
  free(z_hist);
  return iterations;
}

unsigned int pixelData[dispWidth * dispHeight]; // calculated number of iterations
double centerX = startX;
double centerY = startY;
double zoom = 80;
unsigned int maxIts = 200; // max depth for the iterative function
int threadsDone = nproc; // used to prevent multithreading issues
bool adjustIts = true;
bool recenter = true;
unsigned int frameCount = 0;


void indToCoords(unsigned int ind, double* res) {
  res[0] = centerX - dispWidth / 2 / zoom + (ind % dispWidth) / zoom;
  res[1] = centerY - dispHeight / 2 / zoom + floor(ind / dispWidth) / zoom;
}

void* slaveLabor(void* ass) { // first time using multithreading so I gave it a silly name. "ass" is short for "assignment"
  unsigned int start = ((unsigned int*)ass)[0];
  unsigned int end = ((unsigned int*)ass)[1];
  for (int i = start; i < end; i++) {
    double xy[2];
    indToCoords(i, xy);
    unsigned int its = testPoint(xy[0], xy[1], maxIts);
    pixelData[i] = its;
  }

  threadsDone++;
  return 0;
}

int main(int argc, char** argv) {
    if(!al_init()) {
      printf("couldn't initialize allegro\n");
      return 1;
    }

    if (!al_init_image_addon()) {
      printf("couldn't initialize image addon\n");
      return 1;
    }

    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
    if(!timer) {
      printf("couldn't initialize timer\n");
      return 1;
    }

    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    if(!queue) {
      printf("couldn't initialize queue\n");
      return 1;
    }    

    ALLEGRO_DISPLAY* disp = al_create_display(dispWidth, dispHeight);
    if(!disp) {
      printf("couldn't initialize display\n");
      return 1;
    }

    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    bool done = false;
    bool redraw = false;
    ALLEGRO_EVENT event;

    pthread_t threadIDs[nproc][1];
    bool threadsActive = false;
    unsigned int assignments[nproc][2];
    for (unsigned int i = 0; i < nproc; i++) {
      assignments[i][0] = (i == 0) ? 0 : (dispWidth * dispHeight / nproc * (i - 1));
      assignments[i][1] = (i == nproc - 1) ? (dispWidth * dispHeight - 1) : (dispWidth * dispHeight / nproc * i);
    }

    FILE* fp = fopen("./palettes/palette.bin", "r");
    if (!fp) {
      printf("failed to open palette file\n");
      return 1;
    }

    for (int i = 0; i < 7; i++) // skip magic number
      fgetc(fp);

    unsigned int paletteLength;
    fread(&paletteLength, sizeof(unsigned int), 1, fp);
    printf("Palette length: %i\n", paletteLength);
    unsigned int* palette = (unsigned int*) malloc(paletteLength * sizeof(unsigned int));
    fread(palette, sizeof(unsigned int), paletteLength, fp);
    
    al_start_timer(timer);
    while(1) {
      al_wait_for_event(queue, &event);

      switch(event.type) {
        case ALLEGRO_EVENT_TIMER:
          if (threadsDone < nproc)
            break;

          if (threadsActive) {
            threadsActive = false;

            if (frameCount >= maxFrames) {
              done = true;
              break;
            }

            if (zoom > desiredZoom && adjustIts) {
              bool colors[maxIts] = {};
              unsigned int variety = 0;
              for (unsigned int i = 0; i < dispWidth * dispHeight; i++)
                if (!colors[pixelData[i]]) {
                  colors[pixelData[i]] = true;
                  variety++;
                }
              
              if (variety < paletteLength * 9 / 10) { // prevents black pixels
                printf("Increasing depth!\n");
                maxIts += 20;
                break;
              }
            }
            
            printf("Centered at (%lf, %lf)\n", centerX, centerY);

            if (zoom > desiredZoom && recenter) { // finds a good place to recenter
              unsigned int best = 0; // largest value in pixel data
              unsigned int holder = 0; // index of said value
              for (int i = 0; i < dispWidth * dispHeight; i++) {
                double xy[2];
                double xy_c[2];
                indToCoords(i, xy);
                indToCoords(holder, xy_c);

                if (pixelData[i] > best && pixelData[i] < maxIts)
                  best = pixelData[i];
                if (pixelData[i] == best && pow(xy[0] - centerX, 2) + pow(xy[1] - centerY, 2) <= pow(xy_c[0] - centerX, 2) + pow(xy_c[1] - centerY, 2))
                  holder = i;
              }

              if (holder != 0) {
                double newXY[2];
                indToCoords(holder, newXY);
                if (newXY[0] != centerX && newXY[1] != centerY) {
                  centerX = newXY[0];
                  centerY = newXY[1];
                  printf("Recentered at (%lf, %lf)\n", centerX, centerY);
                }
              }
            }
            
            printf("Max depth: %i\n", maxIts);

            zoom *= 1.5;
            printf("Zoom: %lf\n", zoom);

            redraw = true;
          } else {
            threadsActive = true;
            threadsDone = 0;
            for (int i = 0; i < nproc; i++)
              pthread_create(threadIDs[i], NULL, slaveLabor, assignments[i]);
          }
          break;

        case ALLEGRO_EVENT_DISPLAY_CLOSE:
          done = true;
          break;
      }

      if (done)
          break;

      if (redraw) {
        // histogram plotting based on
        // https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set#Histogram_coloring
        unsigned int numIterationsPerPixel[maxIts + 1];
        for (unsigned int i = 0; i < maxIts + 1; i++)
          numIterationsPerPixel[i] = 0;
        for (int i = 0; i < dispWidth * dispHeight; i++)
          numIterationsPerPixel[pixelData[i]]++;

        unsigned int total = 0;
        for (int i = 0; i <= maxIts; i++)
          total += numIterationsPerPixel[i];

        double hues[dispWidth * dispHeight];
        for (unsigned int i = 0; i < dispWidth * dispHeight; i++)
          hues[i] = 0;
        for (int i = 0; i < dispWidth * dispHeight; i++)
          for (int j = 0; j <= pixelData[i]; j++)
            hues[i] += (double) numIterationsPerPixel[j] / (double) total;

        al_clear_to_color(al_map_rgb(0, 0, 0));
        ALLEGRO_BITMAP* bmp = al_get_backbuffer(disp);

        for (int i = 0; i < dispHeight; i++) {
          ALLEGRO_LOCKED_REGION* lock = al_lock_bitmap_region(bmp, 0, i, dispWidth, 1, ALLEGRO_PIXEL_FORMAT_ABGR_8888, ALLEGRO_LOCK_WRITEONLY);
          unsigned int* data = (unsigned int*)lock->data;
          for (int j = 0; j < dispWidth; j++) {
            double val = hues[i * dispWidth + j];
            data[j] = palette[(unsigned int) (val * (paletteLength - 1))];
          }

          al_unlock_bitmap(bmp);
        }

        char leadingZeroes[255];
        char filename[255];
        unsigned int places = (unsigned int) log10(maxFrames) + 1;
        snprintf(leadingZeroes, 255, "./frames/frame-%%0%ii.png", places);
        snprintf(filename, 255, leadingZeroes, frameCount);

        al_save_bitmap(filename, bmp);
        al_flip_display();

        frameCount++;

        redraw = false;
      }
    }

    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    
    free(palette);

    return 0;
}
