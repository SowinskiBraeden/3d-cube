#include <stdlib.h>
#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502884 /* pi */
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <math.h>
#include <stdbool.h>

#define WINDOW_W 800
#define WINDOW_H 800

#define BACKGROUND_R 20
#define BACKGROUND_G 20
#define BACKGROUND_B 20
#define BACKGROUND_A 255
#define RAINDROP_R   156
#define RAINDROP_G   174
#define RAINDROP_B   255
#define RAINDROP_A   255

typedef struct {
  float x; // -1 .. 1
  float y; // -1 .. 1
  float z; // -1 .. 1
} VEC3;

typedef struct {
  float x; // 0 .. WINDOW_W
  float y; // 0 .. WINDOW_H
} VEC2;

VEC2 project(VEC3 v)
{
  return (VEC2)
  {
    x: v.x / v.z,
    y: v.y / v.z,
  };
}

void toScreenCoord(VEC2 *v)
{
  // -1..1 -> 0..2 -> 0..1 -> 0..WINDOW_W/H
  v->x = (v->x + 1) / 2 * WINDOW_W;
  v->y = (1 - (v->y + 1) / 2) * WINDOW_H;
}

void rotate_x(VEC3 *v, double angle)
{
  float y = v->y;
  float z = v->z;
  v->y = y * cos(angle) - z * sin(angle);
  v->z = y * sin(angle) + z * cos(angle);
}

void rotate_y(VEC3 *v, double angle)
{
  float x = v->x;
  float z = v->z;
  v->x = x * cos(angle) - z * sin(angle);
  v->z = x * sin(angle) + z * cos(angle);
}

void rotate_z(VEC3 *v, double angle)
{
  float x = v->x;
  float y = v->y;
  v->x = x * cos(angle) - y * sin(angle);
  v->y = x * sin(angle) + y * cos(angle);
}


void translate_z(VEC3 *v)
{
  v->z = v->z + 1;
}

void draw_point(SDL_Renderer *renderer, VEC2 v)
{
    const float size = 2.0;

    SDL_Rect point_rect;

    point_rect.x = v.x - size / 2;
    point_rect.y = v.y - size / 2;
    point_rect.w = size;
    point_rect.h = size;

    SDL_RenderFillRect(renderer, &point_rect);
}

void draw_ellipse_points(SDL_Renderer *renderer, VEC2 *points, size_t num_points, bool open)
{
    for (size_t i = 0; i < (open ? num_points - 1 : num_points); ++i)
    {
        VEC2 a = points[i];
        VEC2 b = open ? points[i + 1] : points[(i + 1) % num_points];

        SDL_RenderDrawLine(renderer, a.x, a.y, b.x, b.y);
    }
}

void draw_ellipse(SDL_Renderer *renderer, int x, int y, int rx, int ry, int detail)
{
    VEC2 points[detail];

    for (int i = 0; i < detail; ++i)
    {
        float angle = 2.0f * M_PI * i / detail;
        VEC2 p = (VEC2){
            x + (int)(rx * cosf(angle)),
            y + (int)(ry * sinf(angle)),
        };
        // printf("(%f, %f)\n" ,p.x, p.y);
        // draw_point(renderer, p);
        points[i] = p;
    }

    draw_ellipse_points(renderer, points, detail, false);
    for (int i = 0; i < detail; ++i)
    {
        VEC2 a = points[i];
        VEC2 b = points[(i + 1) % (detail)];

        SDL_RenderDrawLine(renderer, a.x, a.y, b.x, b.y);
    }
}

int main(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Error initializing: %s", SDL_GetError());
        return 1;
    }

    SDL_Window   *window;
    SDL_Renderer *renderer;

    window = SDL_CreateWindow(
        "Cube",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_W,
        WINDOW_H,
        SDL_WINDOW_SHOWN
    );

    if (!window)
    {
        SDL_Log("Error creating window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!renderer)
    {
        SDL_Log("Error creating renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int       running;
    SDL_Event event;
    Uint32    last_time;

    double angle = 0;

    int stacks = 10;
    int slices = 12;
    float r = 0.175;

    // Generate sphere vectors
    VEC3 sphere_vectors_3d[stacks * slices];

    for (int i = 0; i < stacks; i++)
    {
        // Phi (angle of latitude, ranges from 0 to PI)
        float phi = M_PI * (float)i / (float)(stacks - 1);
        float sinPhi = sin(phi);
        float cosPhi = cos(phi);

        for (int j = 0; j < slices; j++)
        {
            // Theta (angle of longitude, ranges from 0 to 2*PI)
            float theta = 2.f * M_PI * (float)j / (float)slices;
            float sinTheta = sin(theta);
            float cosTheta = cos(theta);

            // Convert spherical coordinates to Cartesian (x, y, z)
            // The choice of axis mapping may vary. Here Y is vertical.
            sphere_vectors_3d[i * slices + j] = (VEC3){
                x: r * cosTheta * sinPhi,
                y: r * cosPhi,
                z: r * sinTheta * sinPhi,
            };
        }
    }

    // size_t num_sphere_vectors = sizeof(sphere_vectors_3d) / sizeof(sphere_vectors_3d[0]);

    // CUBE
    VEC3 vertecies[8] = {
      {x:  0.3, y:  0.3, z: 0.3},
      {x: -0.3, y:  0.3, z: 0.3},
      {x: -0.3, y: -0.3, z: 0.3},
      {x:  0.3, y: -0.3, z: 0.3},

      {x:  0.3, y:  0.3, z: -0.3},
      {x: -0.3, y:  0.3, z: -0.3},
      {x: -0.3, y: -0.3, z: -0.3},
      {x:  0.3, y: -0.3, z: -0.3},
    };

    int faces[4][4] = {
      {0, 1, 2, 3},  // front
      {4, 5, 6, 7},  // back
      {0, 1, 5, 4},  // top
      {2, 3, 7, 6},  // bottom
    };

    last_time = SDL_GetTicks();
    running   = 1;

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = 0;
            }
        }

        // delta time
        Uint32 now;
        float delta;

        now = SDL_GetTicks();
        delta = (now - last_time) / 1000.0f; // seconds
        last_time = now;

        angle += M_PI * delta * 0.5;

        // clear scren
        SDL_SetRenderDrawColor(
            renderer,
            BACKGROUND_R,
            BACKGROUND_G,
            BACKGROUND_B,
            BACKGROUND_A
        );
        SDL_RenderClear(renderer);

        // render

        SDL_SetRenderDrawColor(
          renderer,
          RAINDROP_R,
          RAINDROP_G,
          RAINDROP_B,
          RAINDROP_A
        );

        // draw sphere
        // draw_ellipse(renderer, WINDOW_W/2, WINDOW_H/2, 150, 150, 500);

        // render latitude ellipses
        for (int i = 0; i < stacks; i++)
        {
            VEC2 ellipse[slices];
            for (int j = 0; j < slices; j++)
            {
                VEC3 v3d = sphere_vectors_3d[i * slices + j];
                rotate_y(&v3d, angle);
                rotate_x(&v3d, angle);
                rotate_z(&v3d, angle);
                translate_z(&v3d);

                VEC2 v2d = project(v3d);
                toScreenCoord(&v2d);

                ellipse[j] = v2d;
            }

            draw_ellipse_points(renderer, ellipse, slices, false);
        }

        // render verticle ellipses
        for (int j = 0; j < slices; j++)
        {
          VEC2 ellipse[stacks];
          for (int i = 0; i < stacks; i++)
          {
            VEC3 v3d = sphere_vectors_3d[i * slices + j];
            rotate_y(&v3d, angle);
            rotate_x(&v3d, angle);
            rotate_z(&v3d, angle);
            translate_z(&v3d);

            VEC2 v2d = project(v3d);
            toScreenCoord(&v2d);

            ellipse[i] = v2d;
          }

          draw_ellipse_points(renderer, ellipse, stacks, true);
        }

        // Draw faces of cube
        // SDL_RenderDrawLine(renderer, x1, y1, x2, y2)
        for (size_t i = 0; i < sizeof(faces) / sizeof(faces[0]); i++)
        {
          for (size_t j = 0; j < sizeof(faces[0]) / sizeof(faces[0][0]); j++)
          {
            VEC3 vec3A = vertecies[faces[i][j]];
            VEC3 vec3B = vertecies[faces[i][(j + 1) % 4]];

            rotate_y(&vec3A, -angle);
            rotate_y(&vec3B, -angle);
            rotate_x(&vec3A, -angle);
            rotate_x(&vec3B, -angle);
            // rotate_z(&vec3A, -angle);
            // rotate_z(&vec3B, -angle);

            translate_z(&vec3A);
            translate_z(&vec3B);

            VEC2 vec2A = project(vec3A);
            VEC2 vec2B = project(vec3B);

            toScreenCoord(&vec2A);
            toScreenCoord(&vec2B);

            SDL_RenderDrawLine(renderer, vec2A.x, vec2A.y, vec2B.x, vec2B.y);
          }
        }

        // draw vertecies of cube
        // for (int i = 0; i < sizeof(vertecies) / sizeof(vertecies[0]); i++)
        // {
        //     VEC3 v3 = vertecies[i];
        //     v3 = rotate(v3, angle);
        //     v3 = translate_z(v3);

        //     VEC2 v2 = project(v3);
        //     v2 = toScreenCoord(v2);

        //     SDL_Rect point_rect;
        //     // Adjust x and y to center the point visually, if desired
        //     point_rect.x = v2.x - 10 / 2;
        //     point_rect.y = v2.y - 10 / 2;
        //     point_rect.w = 10;
        //     point_rect.h = 10;

        //     // Draw the filled rectangle
        //     SDL_RenderFillRect(renderer, &point_rect);
        // }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
