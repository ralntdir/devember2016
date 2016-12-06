// TODO(ralntdir):
// Think how to clean SDL if the programs ends with some crash
// Finished the program if there is a problem at SDL/IMG init or
// when creating the window or the renderer
//
// Features to add:
// ray->sphere intersection
//   -phong shading
//      *Add diffuse component if dotProduct >= 0.0

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
#define HEIGHT 400
#define MAX_COLOR 255

#include <math.h>
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

  vec3 ka;
  vec3 kd;
  vec3 ks;

  real32 alpha;
};

struct light
{
  vec3 position;
  vec3 iAmbient;
  vec3 iDiffuse;
  vec3 iSpecular;
};

struct scene
{
  light lights[1];
  sphere spheres[1];
};

// TODO(ralntdir): Think if I should add this to the scene
vec3 camera = { 0.0, 0.0, 0.0 };

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

    real32 root1 = (-b + sqrt(discriminant))/(2*a);
    real32 root2 = (-b - sqrt(discriminant))/(2*a);

    if ((root1 < root2) && (root1 > 0.0))
    {
      *t = root1;
    }
    else if ((root2 < root1) && (root2 > 0.0))
    {
      *t = root2;
    }
    else if ((root1 < 0) && (root2 < 0))
    {
      result = false;
    }
  }

  return(result);
}

// TODO(ralntdir): Implement real phong shading here!
vec3 phongShading(light myLight, sphere mySphere, vec3 hitPoint)
{
  vec3 result;

  // *N vector (normal at hit point)
  // *L vector (lightPosition - hitPoint)
  vec3 N = normalize(hitPoint - mySphere.center);
  vec3 L = normalize(myLight.position - hitPoint);

  // *R vector (reflection of L -> 2(L·N)N - L)
  // *V vector (camera - hitPoint)
  vec3 R = normalize(2*dotProduct(L, N)*N - L);
  vec3 V = normalize(camera - hitPoint);

  result = mySphere.ka*myLight.iAmbient +
           mySphere.kd*myLight.iDiffuse*dotProduct(L, N) +
           mySphere.ks*myLight.iSpecular*pow(max(dotProduct(R, V), 0.0), mySphere.alpha);

  return(result);
}

vec3 color(ray myRay, scene *myScene, vec3 backgroundColor)
{
  // vec3 result = backgroundColor;
  vec3 result = { 0.0, 0.0, 0.0 };

  real32 t = -1.0;

  if (hitSphere(myScene->spheres[0], myRay, &t))
  {
    if (t >= 0.0)
    {
#if 1
      vec3 hitPoint = myRay.origin + t*myRay.direction;
      result = phongShading(myScene->lights[0], myScene->spheres[0], hitPoint);
#else
      result = myScene->spheres[0].kd;
#endif
    }
  }

  return(result);
}

void initializeLight(light *myLight)
{
  myLight->position = { -0.57735027, 0.57735027, -0.57735027 };
  //myLight->position = { 2.0, 5.0, 1.0 };
  //myLight->iAmbient = { 0.7, 0.7, 0.7 };
  myLight->iAmbient = { 1.0, 1.0, 1.0 };
  myLight->iDiffuse = { 1.0, 1.0, 1.0 };
  //myLight->iSpecular = { 0.5, 0.5, 0.5 };
  myLight->iSpecular = { 1.0, 1.0, 1.0 };
}

void initializeScene(scene *myScene)
{
  sphere mySphere =  {};
#if 1
  mySphere.center = { 0.0, 0.0, -2.0 };
  mySphere.radius = 1.0;
#else
  mySphere.center = { 0.0, 0.0, -1.0 };
  mySphere.radius = 0.5;
#endif

  mySphere.ka = { 0.1, 0.1, 0.1 };
  //mySphere.ka = { 0.1, 0.1, 0.2 };
  mySphere.kd = { 1.0, 0.0, 0.0 };
  // mySphere.kd = { 0.5, 0.5, 0.5 };
  mySphere.ks = { 1.0, 1.0, 1.0 };
  //mySphere.ks = { 0.5, 0.5, 0.5 };

  mySphere.alpha = 100.0;

  myScene->spheres[0] = mySphere;

  light myLight;
  initializeLight(&myLight);

  myScene->lights[0] = myLight;
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

  // vec3 horizontalOffset = { 4.0, 0.0, 0.0 };
  // vec3 verticalOffset = { 0.0, 2.0, 0.0 };
  // vec3 lowerLeftCorner = { -2.0, -1.0, -1.0 };

  vec3 horizontalOffset = { 2.0, 0.0, 0.0 };
  vec3 verticalOffset = { 0.0, 2.0, 0.0 };
  vec3 lowerLeftCorner = { -1.0, -1.0, -1.0 };
  // vec3 lowerLeftCorner = { -1.0, -1.0, -1.0 };

  scene myScene = {};
  initializeScene(&myScene);
  
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

      vec3 backgroundColor = { 0.0, ((real32)i/HEIGHT), ((real32)j/WIDTH) };
      vec3 col = color(cameraRay, &myScene, backgroundColor);
      clamp(&col);

      int32 r = int32(255.0*col.e[0]);
      int32 g = int32(255.0*col.e[1]);
      int32 b = int32(255.0*col.e[2]);

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

  SDL_Event event;

  bool running = true;

  while (running)
  {
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
      {
        running = false;
      }
      else if (event.type == SDL_KEYDOWN)
      {
        if (event.key.keysym.sym == SDLK_ESCAPE)
        {
          running = false;
        }
      }
    }

    // Show the texture
    SDL_RenderCopy(renderer, texture, 0, 0);
    SDL_RenderPresent(renderer);
  }

  // Free the texture
  SDL_DestroyTexture(texture);
  // Quit IMG
  IMG_Quit();
  // Quit SDL
  SDL_Quit();

  return(0);
}
