#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <ncurses.h>
#include <unistd.h>
#include <wchar.h>
#include <ncursesw/ncurses.h> // Ensure you use the 'w' version
#include <locale.h>
#define MAX_BANANAS 8
#define POINTS_PER_BANANA 120
#define PI 3.1415926535

typedef struct { float x, y, z; float t; } Point;
typedef struct {
    float x, y, rx, ry, rz;
    float drx, dry, drz, speed;
} Banana;

Point model[POINTS_PER_BANANA];

void generate_model() {
    int idx = 0;
    for (int i = 0; i < 15; i++) {
        float t = i / 14.0f;
        float bx = 2.0f * (t - 0.5f);
        float by = 0.5f * (bx * bx) - 0.2f; // Parabolic curve
        float radius = 0.3f * sinf(PI * t) + 0.06f; // Thickness
        for (int a = 0; a < 360; a += 45) {
            float rad = a * PI / 180.0f;
            model[idx].x = bx;
            model[idx].y = by + radius * cosf(rad);
            model[idx].z = radius * sinf(rad);
            model[idx].t = t; // For gradient (green tips)
            idx++;
        }
    }
}

void setup_colors() {
    start_color();
    // Yellow shades (10-15), Green shades (16-17)
    init_color(10, 1000, 1000, 0);   // Bright Yellow
    init_color(11, 900, 850, 0);     // Warm Yellow
    init_color(12, 800, 700, 0);     // Golden
    init_color(13, 700, 600, 0);     // Darker Yellow
    init_color(14, 400, 600, 0);     // Greenish Yellow
    init_color(15, 100, 500, 100);   // Tip Green
    
    init_pair(1, 10, 0); init_pair(2, 11, 0);
    init_pair(3, 12, 0); init_pair(4, 13, 0);
    init_pair(5, 14, 0); init_pair(6, 15, 0);
}

void rotate(float *x, float *y, float *z, float rx, float ry, float rz) {
    float tx, ty, tz;
    // X
    ty = *y * cos(rx) - *z * sin(rx);
    tz = *y * sin(rx) + *z * cos(rx);
    *y = ty; *z = tz;
    // Y
    tx = *x * cos(ry) + *z * sin(ry);
    tz = -*x * sin(ry) + *z * cos(ry);
    *x = tx; *z = tz;
    // Z
    tx = *x * cos(rz) - *y * sin(rz);
    ty = *x * sin(rz) + *y * cos(rz);
    *x = tx; *y = ty;
}

int main() {
    setlocale(LC_ALL, "");
    initscr(); noecho(); curs_set(0); timeout(0);
    setup_colors();
    generate_model();
    srand(time(NULL));

    Banana bananas[MAX_BANANAS];
    for (int i = 0; i < MAX_BANANAS; i++) {
        bananas[i].x = rand() % 80;
        bananas[i].y = -(rand() % 40);
        bananas[i].rx = (rand() % 100) / 10.0f;
        bananas[i].ry = (rand() % 100) / 10.0f;
        bananas[i].rz = (rand() % 100) / 10.0f;
        bananas[i].drx = (rand() % 10) / 100.0f + 0.02;
        bananas[i].dry = (rand() % 10) / 100.0f + 0.02;
        bananas[i].drz = (rand() % 10) / 100.0f + 0.02;
        bananas[i].speed = (rand() % 5) / 10.0f + 0.2;
    }

    while (getch() == ERR) {
        erase();
        int row, col; getmaxyx(stdscr, row, col);

        for (int i = 0; i < MAX_BANANAS; i++) {
            bananas[i].y += bananas[i].speed;
            bananas[i].rx += bananas[i].drx;
            bananas[i].ry += bananas[i].dry;
            bananas[i].rz += bananas[i].drz;
            if (bananas[i].y > row) {
                bananas[i].y = -5;
                bananas[i].x = rand() % col;
            }

            for (int j = 0; j < POINTS_PER_BANANA; j++) {
                float px = model[j].x, py = model[j].y, pz = model[j].z;
                rotate(&px, &py, &pz, bananas[i].rx, bananas[i].ry, bananas[i].rz);
                
                int screen_x = (int)(bananas[i].x + px * 6);
                int screen_y = (int)(bananas[i].y + py * 3);

                if (screen_x > 0 && screen_x < col - 2 && screen_y > 0 && screen_y < row) {
                    // Color based on t (0.0 to 1.0) and a bit of lighting (pz)
                    int color = 1 + (int)(model[j].t * 5);
                    if (pz < -0.1) color++; 
                    attron(COLOR_PAIR(color > 6 ? 6 : color));
                    mvprintw(screen_y, screen_x, "██"); // Double block for square pixels
                    attroff(COLOR_PAIR(color));
                }
            }
        }
        refresh();
        usleep(30000);
    }
    endwin();
    return 0;
}
