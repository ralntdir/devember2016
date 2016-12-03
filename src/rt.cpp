// TODO(ralntdir):
// Think how to clean SDL if the programs ends with some crash
// Finished the program if there is a problem at SDL/IMG init or
// when creating the window or the renderer
//
// Features to add:
// ray->sphere intersection
// ray creation
// handle sdl events to be able to close the window whenever I want

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

#include "myMath.h"

struct ray
{
  vec3 origin;
  vec3 direction;
};

struct sphere
{
  vec3 center;
  real32 radius;
  vec3 color;
};

bool hitSphere(sphere mySphere, ray myRay, real32 *t)
{
  bool result = false;

  vec3 originCenter = myRay.origin - mySphere.center;

  real32 a = dotProduct(myRay.direction, myRay.direction);
  real32 b = 2 * dotProduct(originCenter, myRay.direction);
  real32 c = dotProduct(originCenter, originCenter) - mySphere.radius*mySphere.radius;

  real32 discriminant = b*b - 4*a*c;

  if (discriminant < 0)
  {
    return(result);
  }
  else
  {
    result = true;
    /*real32 root1 = (-b + sqrt(discriminant))/(2*a);
    if (discriminant > 0)
    {
      real32 root2 = (-b - sqrt(discriminant))/(2*a);

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
    }*/
  }

  return(result);
}

vec3 color(ray myRay, sphere mySphere, vec3 backgroundColor)
{
  vec3 result = backgroundColor;

  real32 t;

  if (hitSphere(mySphere, myRay, &t))
  {
    result = mySphere.color;
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

  vec3 horizontalOffset = { 4.0, 0.0, 0.0 };
  vec3 verticalOffset = { 0.0, 2.0, 0.0 };
  vec3 lowerLeftCorner = { -2.0, -1.0, -1.0 };

  sphere mySphere =  {};
  mySphere.center = { 0.0, 0.0, -1.0 };
  mySphere.radius = 0.5;
  mySphere.color = { 0.9, 0.9, 0.9 };
  
  // NOTE(ralntdir): From top to bottom
  for (int32 i = HEIGHT-1; i >= 0 ; i--)
  {
    for (int32 j = 0; j < WIDTH; j++)
    {
      real32 u = real32(j)/real32(WIDTH);
      real32 v = real32(i)/real32(HEIGHT);
      ray cameraRay = {};
      cameraRay.origin = { 0.0, 0.0, 0.0 };
      cameraRay.direction = lowerLeftCorner + u*horizontalOffset + v*verticalOffset;
      // printVector(cameraRay.origin);
      // printVector(cameraRay.direction);

      vec3 backgroundColor = { 0.0, ((real32)i/HEIGHT), ((real32)j/WIDTH) };
      vec3 col = color(cameraRay, mySphere, backgroundColor);

      int32 r = (255.0*col.e[0]);
      int32 g = (255.0*col.e[1]);
      int32 b = (255.0*col.e[2]);

      ofs << r << " " << g << " " << b << "\n";
    }
  }

  ofs.close();

  // Load the image
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
