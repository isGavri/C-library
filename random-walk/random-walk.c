#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <stdio.h>
#include <stdlib.h>

#define WIDTH 300
#define HEIGHT 400

int main(int argc, char *argv[])
{
  int num_agents = 5;
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
    fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Window *window = SDL_CreateWindow("Random Walk", SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);

  if (!window)
  {
    fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }
  // NEW: Create a renderer so we can draw (and show) the window
  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer)
  {
    fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  int app_running = 1;

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
    // NEW: Draw a black background and "Present" it to Wayland
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Black
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);  // This command tells Hyprland "Hey, show this window now!"
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
