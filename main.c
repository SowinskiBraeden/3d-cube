#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502884 /* pi */
#endif
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
    // y: v.x * sin(angle) + v.y * cos(angle),
    // z: v.z,
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

void draw_point(SDL_Renderer *renderer, VEC2 v)
{
    const float size = 2.0;

    SDL_Rect point_rect;
    // Adjust x and y to center the point visually, if desired
    point_rect.x = v.x - size / 2;
    point_rect.y = v.y - size / 2;
    point_rect.w = size;
    point_rect.h = size;

    // Draw the filled rectangle
    SDL_RenderFillRect(renderer, &point_rect);
}

void draw_circle(SDL_Renderer *renderer, VEC2 v, float r)
{
    float x = r, y = 0.0;

    // Initialising the value of P
    int P = 1 - r;
    while (x > y)
    {
        y++;

        // Mid-point is inside or on the perimeter
        if (P <= 0)
            P = P + 2*y + 1;

        // Mid-point is outside the perimeter
        else
        {
            x--;
            P = P + 2*y - 2*x + 1;
        }

        // All the perimeter points have already been printed
        if (x < y)
            break;

        // Printing the generated point and its reflection
        // in the other octants after translation
        draw_point(renderer, (VEC2){x + v.x, y + v.y});
        draw_point(renderer, (VEC2){-x + v.x, y + v.y});
        draw_point(renderer, (VEC2){x + v.x, -y + v.y});
        draw_point(renderer, (VEC2){-x + v.x, -y + v.y});

        // If the generated point is on the line x = y then
        // the perimeter points have already been printed
        if (x != y)
        {
            draw_point(renderer, (VEC2){y + v.x,  x + v.y});
            draw_point(renderer, (VEC2){-y + v.x,  x + v.y});
            draw_point(renderer, (VEC2){y + v.x, -x + v.y});
            draw_point(renderer, (VEC2){-y + v.x, -x + v.y});
        }
    }
}

void draw_ellipse_points(SDL_Renderer *renderer, VEC2 *points, size_t num_points)
{
    for (size_t i = 0; i < num_points; ++i)
    {
        VEC2 a = points[i];
        VEC2 b = points[(i + (size_t) sqrt(num_points)) % num_points];

        SDL_RenderDrawLine(renderer, a.x, a.y, b.x, b.y);
    }
}

void draw_2d_ellipse(SDL_Renderer *renderer, int x, int y, int rx, int ry) {
    // Number of points to approximate the ellipse
    const int num_points = 100;
    VEC2 points[num_points];

    for (int i = 0; i < num_points; ++i)
    {
        float angle = 2.0f * M_PI * i / num_points;
        VEC2 p = (VEC2){
            x + (int)(rx * cosf(angle)),
            y + (int)(ry * sinf(angle)),
        };
        // printf("(%f, %f)\n" ,p.x, p.y);
        // draw_point(renderer, p);
        points[i] = p;
    }

    draw_ellipse_points(renderer, points, num_points);
    for (int i = 0; i < num_points; ++i)
    {
        VEC2 a = points[i];
        VEC2 b = points[(i + 1) % (num_points)];

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

    int stacks = 20;
    int slices = 20;
    float r = 0.175;

    VEC3 sphere[(stacks) * (slices)];

    for (int i = 0; i < stacks; i++) {
        // Phi (angle of latitude, ranges from 0 to PI)
        float phi = M_PI * (float)i / (float)stacks;
        float sinPhi = sin(phi);
        float cosPhi = cos(phi);

        for (int j = 0; j < slices; j++) {
            // Theta (angle of longitude, ranges from 0 to 2*PI)
            float theta = 2.f * M_PI * (float)j / (float)slices;
            float sinTheta = sin(theta);
            float cosTheta = cos(theta);

            // Convert spherical coordinates to Cartesian (x, y, z)
            // The choice of axis mapping may vary. Here Y is vertical.
            sphere[i * stacks + j] = (VEC3){
                x: r * cosTheta * sinPhi,
                y: r * cosPhi,
                z: r * sinTheta * sinPhi,
            };
        }
    }

    size_t num_points = sizeof(sphere) / sizeof(sphere[0]);

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
        // draw_circle(renderer, (VEC2){WINDOW_W/2, WINDOW_H/2}, 150);
        // draw_2d_ellipse(renderer, WINDOW_W/2, WINDOW_H/2, 150, 150);

        VEC2 ellipse[num_points];

        for (size_t i = 0; i < num_points; i++)
        {
            VEC3 v = sphere[i];
            v = rotate(v, angle);
            v = translate_z(v);
            // v.y = v.y - 0.125;
            VEC2 v2 = project(v);
            v2 = toScreenCoord(v2);
            ellipse[i] = v2;
        }
        draw_ellipse_points(renderer, ellipse, num_points);

        // Draw faces of cube
        // SDL_RenderDrawLine(renderer, x1, y1, x2, y2)
        for (int i = 0; i < 6; i++)
        {
          for (int j = 0; j < 4; j++)
          {
            VEC3 ao = vertecies[faces[i][j]];
            VEC3 bo = vertecies[faces[i][(j + 1) % 4]];

            ao = rotate(ao, -angle);
            bo = rotate(bo, -angle);

            ao = translate_z(ao);
            bo = translate_z(bo);

            VEC2 a = project(ao);
            VEC2 b = project(bo);

            a = toScreenCoord(a);
            b = toScreenCoord(b);

            SDL_RenderDrawLine(renderer, a.x, a.y, b.x, b.y);
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
