#include <SDL3/SDL.h>

#define WIDTH 400
#define HEIGHT 400

int main(int argc, char *argv[])
{
  if (!SDL_Init(SDL_INIT_VIDEO))
  {
    SDL_Log("SDL_Init failed: %s", SDL_GetError());
    return 1;
  }

  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;

  if (!SDL_CreateWindowAndRenderer("Pong", WIDTH, HEIGHT, 0, &window, &renderer))
  {
    SDL_Log("Window/Renderer creation failed: %s", SDL_GetError());
    return 1;
  }

  int running = 1;
  SDL_Event event;

  while (running)
  {
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_EVENT_QUIT)
      {
        running = 0;
      }
    }

    // B. Update Game Logic (Move paddles, ball physics)
    // (Pong logic goes here later)

    // C. Render (Draw to screen)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_FRect paddle = {20, 250, 20, 100};
    SDL_FRect paddleGame = {360, 120, 20, 100};

    SDL_FRect ball = {(WIDTH / 2) - 10, (HEIGHT / 2) - 10, 10, 10};

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &paddle);
    SDL_RenderFillRect(renderer, &paddleGame);
    SDL_RenderFillRect(renderer, &ball);

    SDL_RenderPresent(renderer);
  }

  // 4. Cleanup
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
