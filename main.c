#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <math.h>

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

VEC2 toScreenCoord(VEC2 v)
{
  // -1..1 -> 0..2 -> 0..1 -> 0..WINDOW_W/H

  return (VEC2) {
    x: (v.x + 1) / 2 * WINDOW_W,
    y: (1 - (v.y + 1) / 2) * WINDOW_H,
  };
}

VEC3 rotate(VEC3 v, double angle)
{
  return (VEC3) {
    x: v.x * cos(angle) - v.z * sin(angle),
    // y: v.y * cos(angle) - v.x * sin(angle),
    y: v.y,
    z: v.x * sin(angle) + v.z * cos(angle),
  };
}

VEC3 translate_z(VEC3 v)
{
  return (VEC3) {
    v.x,
    v.y,
    v.z + 1,
  };
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

    VEC3 vertecies[8] = {
      {x:  0.25, y:  0.25, z: 0.25},
      {x: -0.25, y:  0.25, z: 0.25},
      {x: -0.25, y: -0.25, z: 0.25},
      {x:  0.25, y: -0.25, z: 0.25},

      {x:  0.25, y:  0.25, z: -0.25},
      {x: -0.25, y:  0.25, z: -0.25},
      {x: -0.25, y: -0.25, z: -0.25},
      {x:  0.25, y: -0.25, z: -0.25},
    };

    int faces[6][4] = {
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

        angle += 3.141592654 * delta;

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

        // Draw faces
        // SDL_RenderDrawLine(renderer, x1, y1, x2, y2)
        for (int i = 0; i < 6; i++)
        {
          for (int j = 0; j < 4; j++)
          {
            VEC3 ao = vertecies[faces[i][j]];
            VEC3 bo = vertecies[faces[i][(j + 1) % 4]];

            ao = rotate(ao, angle);
            bo = rotate(bo, angle);

            ao = translate_z(ao);
            bo = translate_z(bo);

            VEC2 a = project(ao);
            VEC2 b = project(bo);

            a = toScreenCoord(a);
            b = toScreenCoord(b);

            SDL_RenderDrawLine(renderer, a.x, a.y, b.x, b.y);
          }
        }

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
