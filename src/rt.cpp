// TODO(ralntdir):
// Think how to clean SDL if the programs ends with some crash
// Finished the program if there is a problem at SDL/IMG init or
// when creating the window or the renderer

#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// TODO(ralntdir): read this from a scene file?
#define WIDTH 400
#define HEIGHT 200
#define MAX_COLOR 255

int main(int argc, char* argv[])
{
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Surface *surface;
  SDL_Texture *texture;

  // Init SDL
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
  {
    std::cout << "Error in SDL_Init(): " << SDL_GetError() << "\n";
  }

  // Init SDL_Image
  if (IMG_Init(0) < 0)
  {
    std::cout << "Error in IMG_Init(): " << IMG_GetError() << "\n";
  }

  // Create a Window
  // NOTE(ralntdir): SDL_WINDOW_SHOWN is ignored by SDL_CreateWindow().
  // The SDL_Window is implicitly shown if SDL_WINDOW_HIDDEN is not set.
  window = SDL_CreateWindow("Devember RT", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                            WIDTH, HEIGHT, SDL_WINDOW_SHOWN);

  if (window == 0)
  {
    std::cout << "Error in SDL_CreateWindow(): " << SDL_GetError() << "\n";
  }

  // Create a Renderer
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  if (renderer == 0)
  {
    std::cout << "Error in SDL_CreateRenderer(): " << SDL_GetError() << "\n";
  }

  // Create a .ppm file 
  std::cout << "P3\n";
  std::cout << "# image.ppm\n";
  std::cout << WIDTH << " " << HEIGHT << "\n";
  std::cout << MAX_COLOR << "\n";
  
  // NOTE(ralntdir): From top to bottom
  for (int i = HEIGHT; i > 0 ; i--)
  {
    for (int j = 0; j < WIDTH; j++)
    {
      int r = 0;
      int g = (255.00*i/HEIGHT);
      int b = (255.00*j/WIDTH);

      std::cout << r << " " << g << " " << b << "\n";
    }
  }

  // Load the image
  // TODO(ralntdir): right now I'm loading the image from the previous
  // execution!!!! Change this!!!!!
  surface = IMG_Load("image.ppm");
  if (surface == 0)
  {
    std::cout << "Error in IMG_Load(): " << IMG_GetError() << "\n";
  }

  texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (texture == 0)
  {
    std::cout << "Error in SDL_CreateTextureFromSurface(): " << SDL_GetError() << "\n";
  }
  SDL_FreeSurface(surface);

  // Show the texture
  SDL_RenderCopy(renderer, texture, 0, 0);
  SDL_RenderPresent(renderer);
  SDL_Delay(5000);

  // Free the texture
  SDL_DestroyTexture(texture);
  // Quit IMG
  IMG_Quit();
  // Quit SDL
  SDL_Quit();

  return(0);
}
