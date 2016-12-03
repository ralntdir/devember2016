// TODO(ralntdir):
// Think how to clean SDL if the programs ends with some crash
// Finished the program if there is a problem at SDL/IMG init or
// when creating the window or the renderer
//
// Features to add:
// sphere "class"
// ray "class"
// ray->sphere intersection

#include <iostream>
#include <fstream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// NOTE(ralntdir): For number types
#include <stdint.h>

typedef int32_t int32;

typedef float real32;
typedef double real64;

// TODO(ralntdir): read this from a scene file?
#define WIDTH 400
#define HEIGHT 200
#define MAX_COLOR 255

struct vec3
{
  real32 x;
  real32 y;
  real32 z;
};

struct ray
{
  vec3 origin;
  vec3 direction;
};

struct sphere
{
  vec3 center;
  real32 radius;
};

real32 dotProduct(vec3 vector1, vec3 vector2)
{
  real32 result;

  result = vector1.x * vector2.x + vector1.y * vector2.y + vector1.z * vector2.z;

  return(result);
}

bool hitSphere(sphere mySphere, ray myRay, real32 *t)
{
  bool result = false;

  real32 a = dotProduct(myRay.direction, myRay.direction);
  real32 b = 2 * dotProduct(myRay.origin, myRay.direction);
  real32 c = dotProduct(myRay.origin, myRay.origin) - mySphere.radius*mySphere.radius;

  real32 discriminant = 4*a*c;

  if (discriminant < 0)
  {
    return(result);
  }
  else
  {
    real32 root1 = (-b + sqrt(discriminant))/2*a;
    if (discriminant > 0)
    {
      real32 root2 = (-b - sqrt(discriminant))/2*a;

      if ((root1 > 0) && (root2 > 0) && (root1 < root2))
      {
        *t = root1;
        result = true;
        return(result);
      }
      else if ((root1 < 0) && (root2 > 0) && (root1 < root2))
      {
        *t = root2;
        result = true;
        return(result);
      }
      else
      {
        return(result);
      }
    }
    else
    {
      *t = root1;
      result = true;
      return(result);
    }
  }

  return(result);
}

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
  std::ofstream ofs("image.ppm", std::ofstream::out | std::ofstream::binary);

  ofs << "P3\n";
  ofs << "# image.ppm\n";
  ofs << WIDTH << " " << HEIGHT << "\n";
  ofs << MAX_COLOR << "\n";
  
  // NOTE(ralntdir): From top to bottom
  for (int i = HEIGHT; i > 0 ; i--)
  {
    for (int j = 0; j < WIDTH; j++)
    {
      int r = 0;
      int g = (255.00*i/HEIGHT);
      int b = (255.00*j/WIDTH);

      ofs << r << " " << g << " " << b << "\n";
    }
  }

  ofs.close();

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
