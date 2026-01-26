/* Compile gcc -Wall -Wextra -o random-walk random-walk.c -lSDL2 */
#include <SDL2/SDL.h>
#include <SDL2/SDL_mutex.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_stdinc.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 800
#define HEIGHT 800
#define PADDING 10

#define WALKER_SIZE 5
#define WALKERS 20

#define JUMP 5

Uint32 colors[30] = {
    // --- Reds & Pinks ---
    0xFF0000,  // Pure Red
    0xFF5733,  // Persimmon (Red-Orange)
    0xC70039,  // Crimson
    0x900C3F,  // Dark Berry
    0xFF69B4,  // Hot Pink
    0xE91E63,  // Deep Pink

    // --- Oranges & Yellows ---
    0xFF8C00,  // Dark Orange
    0xFFC300,  // Vivid Yellow
    0xF1C40F,  // Sunflower
    0xD35400,  // Pumpkin
    0xFFD700,  // Gold

    // --- Greens ---
    0x00FF00,  // Lime Green
    0x2ECC71,  // Emerald Green
    0x008000,  // Dark Green
    0xADFF2F,  // Green Yellow
    0x16A085,  // Sea Green
    0x27AE60,  // Nephritis Green

    // --- Blues & Cyans ---
    0x0000FF,  // Pure Blue
    0x3498DB,  // Peter River (Soft Blue)
    0x00FFFF,  // Cyan
    0x1ABC9C,  // Turquoise
    0x2980B9,  // Belize Hole (Dark Blue)
    0x2C3E50,  // Midnight Blue (Very Dark)

    // --- Purples & Violets ---
    0x8E44AD,  // Wisteria Purple
    0x800080,  // Classic Purple
    0x9B59B6,  // Amethyst
    0x4B0082,  // Indigo

    // --- Earth & Neutrals ---
    0x8B4513,  // Saddle Brown
    0xA0522D,  // Sienna
    0x95A5A6   // Concrete Grey
};

typedef struct coord
{
    int x;
    int y;
} coord_t;

coord_t get_quadrantI()
{
    coord_t direction = {};
    int rand_direction = (rand() % 2) + 1;
    switch (rand_direction)
    {
        case 1:
            direction.y = -JUMP;
            return direction;
        case 2:
            direction.x = JUMP;
            return direction;
    }
    return direction;
}

coord_t get_quadrantII()
{
    coord_t direction = {};
    int rand_direction = (rand() % 2) + 1;
    switch (rand_direction)
    {
        case 1:
            direction.y = -JUMP;
            return direction;
        case 2:
            direction.x = -JUMP;
            return direction;
    }
    return direction;
}

coord_t get_quadrantIII()
{
    coord_t direction = {};
    int rand_direction = (rand() % 2) + 1;
    switch (rand_direction)
    {
        case 1:
            direction.y = JUMP;
            return direction;
        case 2:
            direction.x = -JUMP;
            return direction;
    }
    return direction;
}

coord_t get_quadrantIV()
{
    coord_t direction = {};
    int rand_direction = (rand() % 2) + 1;
    switch (rand_direction)
    {
        case 1:
            direction.y = JUMP;
            return direction;
        case 2:
            direction.x = JUMP;
            return direction;
    }
    return direction;
}

coord_t get_random_directon()
{
    coord_t direction = {};

    int rand_direction = (rand() % 4) + 1;
    switch (rand_direction)
    {
        case 1:
            direction.y = -JUMP;
            return direction;
        case 2:
            direction.x = JUMP;
            return direction;
        case 3:
            direction.y = JUMP;
            return direction;
        case 4:
            direction.x = -JUMP;
            return direction;
    }
    return direction;
}

int main(int argc, char* argv[])
{
    int num_agents = 4;
    if (argc == 2)
    {
        num_agents = atoi(argv[1]);
    }
    else if (argc > 2)
    {
        printf("Usage: %s <num-of-agents>\n", argv[0]);
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n",
                SDL_GetError());
        return 1;
    }

    SDL_Window* window =
        SDL_CreateWindow("Random Walk", SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    if (!window)
    {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n",
                SDL_GetError());
        return 1;
    }

    SDL_Surface* surface = SDL_GetWindowSurface(window);

    // SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
    // SDL_RENDERER_ACCELERATED); if (!renderer)
    // {
    //   fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n",
    //   SDL_GetError()); return 1;
    // }

    int app_running = 1;

    srand(time(NULL));

    SDL_Rect walkers[WALKERS] = {[0 ... WALKERS - 1] = {WIDTH / 2, HEIGHT / 2,
                                                        WALKER_SIZE,
                                                        WALKER_SIZE}};

    while (app_running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                app_running = 0;
            }
        }

        for (int i = 0; i < WALKERS; i++)
        {
            coord_t coords;
            // switch (i % 4)
            // {
            //   case 0:
            //     coords = get_quadrantI();
            //     break;
            //   case 1:
            //     coords = get_quadrantII();
            //     break;
            //   case 2:
            //     coords = get_quadrantIII();
            //     break;
            //   case 3:
            //     coords = get_quadrantIV();
            //     break;
            // }
            coords = get_random_directon();
            walkers[i].x += coords.x;
            walkers[i].y += coords.y;

            if (walkers[i].x < PADDING)
            {
                walkers[i].x = PADDING;
            }

            if (walkers[i].x > WIDTH - PADDING - WALKER_SIZE)
            {
                walkers[i].x = WIDTH - PADDING - WALKER_SIZE;
            }

            if (walkers[i].y < PADDING)
            {
                walkers[i].y = PADDING;
            }

            if (walkers[i].y > HEIGHT - PADDING - WALKER_SIZE)
            {
                walkers[i].y = HEIGHT - PADDING - WALKER_SIZE;
            }

            Uint8 r = (walkers[i].x * 255) / WIDTH;

            Uint8 g = (walkers[i].y * 255) / HEIGHT;

            Uint8 b = 128;

            Uint32 dynamic_color = (r << 16) | (g << 8) | b;

            // From array
            // SDL_FillRect(surface, &walkers[i], colors[(i   % 12 ) + 11]);
            // Gradient
            SDL_FillRect(surface, &walkers[i], dynamic_color);
        }
        SDL_UpdateWindowSurface(window);
        SDL_Delay(20);

        // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        // SDL_RenderClear(renderer);
        // SDL_RenderPresent(renderer);
    }

    // SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
