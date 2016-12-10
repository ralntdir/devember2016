// TODO(ralntdir):
// Think how to clean SDL if the programs ends with some crash
// Finished the program if there is a problem at SDL/IMG init or
// when creating the window or the renderer
//
// Features to add:
// ray->sphere intersection (check if it's completed)
// shadows
// reflection

// Files manipulation
#include <iostream>
#include <fstream>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// NOTE(ralntdir): For number types
#include <stdint.h>

// NOTE(ralntdir): For rand
#include <random>

// NOTE(ralntdir): For FLT_MAX
#include <float.h>

typedef int32_t int32;

typedef float real32;
typedef double real64;

// TODO(ralntdir): read this from a scene file?
#define WIDTH 500
#define HEIGHT 500
#define MAX_COLOR 255
#define MAX_SAMPLES 100

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
  vec3 camera;

  int32 maxSpheres = 8;
  int32 maxLights = 1;

  int32 numSpheres;
  int32 numLights;
  light lights[1];
  sphere spheres[8];
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

vec3 phongShading(light myLight, sphere mySphere, vec3 camera, vec3 hitPoint)
{
  vec3 result;

  // *N vector (normal at hit point)
  // *L vector (lightPosition - hitPoint)
  vec3 N = normalize(hitPoint - mySphere.center);
  vec3 L = normalize(myLight.position - hitPoint);
  real32 dotProductLN = max(dotProduct(L, N), 0.0);
  real32 filterSpecular = dotProductLN > 0.0 ? 1.0 : 0.0;

  // *R vector (reflection of L -> 2(L·N)N - L)
  // *V vector (camera - hitPoint)
  vec3 R = normalize(2*dotProduct(L, N)*N - L);
  vec3 V = normalize(camera - hitPoint);

  // Only add specular component if you have diffuse,
  // if dotProductLN > 0.0
  result = mySphere.ka*myLight.iAmbient +
           mySphere.kd*myLight.iDiffuse*dotProductLN +
           filterSpecular*mySphere.ks*myLight.iSpecular*pow(max(dotProduct(R, V), 0.0), mySphere.alpha);

  return(result);
}

vec3 color(ray myRay, scene *myScene, vec3 backgroundColor)
{
  // vec3 result = backgroundColor;
  vec3 result = { 0.0, 0.0, 0.0 };

  real32 maxt = FLT_MAX;
  real32 t = -1.0;

  for (int i = 0; i < myScene->numSpheres; i++)
  {
    sphere mySphere = myScene->spheres[i];
    if (hitSphere(mySphere, myRay, &t))
    {
      if ((t >= 0.0) && (t < maxt))
      {
        maxt = t;
        vec3 hitPoint = myRay.origin + t*myRay.direction;
        result = phongShading(myScene->lights[0], mySphere, myScene->camera, hitPoint);
      }
    }
  }

  return(result);
}

void readSceneFile(scene *myScene, char *filename)
{
  std::string line;
  std::ifstream scene(filename);

  if (scene.is_open())
  {
    while (!scene.eof())
    {
      scene >> line;

      // If line is not a comment
      if (line[0] != '#')
      {
        std::cout << line << "\n";

        if (line == "camera")
        {
          scene >> myScene->camera.x;
          scene >> myScene->camera.y;
          scene >> myScene->camera.z;
        }
        else if (line == "sphere")
        {
          sphere mySphere = {};

          scene >> line; // center
          scene >> mySphere.center.x;
          scene >> mySphere.center.y;
          scene >> mySphere.center.z;
          scene >> line; // radius
          scene >> mySphere.radius;
          scene >> line; // ka
          scene >> mySphere.ka.x;
          scene >> mySphere.ka.y;
          scene >> mySphere.ka.z;
          scene >> line; // kd
          scene >> mySphere.kd.x;
          scene >> mySphere.kd.y;
          scene >> mySphere.kd.z;
          scene >> line; // ks
          scene >> mySphere.ks.x;
          scene >> mySphere.ks.y;
          scene >> mySphere.ks.z;
          scene >> line; // alpha
          scene >> mySphere.alpha;

          myScene->numSpheres++;
          if (myScene->numSpheres <= myScene->maxSpheres)
          {
            myScene->spheres[myScene->numSpheres-1] = mySphere;
          }
        }
        else if (line == "light")
        {
          light myLight = {};

          scene >> line; // position
          scene >> myLight.position.x;
          scene >> myLight.position.y;
          scene >> myLight.position.z;
          scene >> line; // iAmbient
          scene >> myLight.iAmbient.x;
          scene >> myLight.iAmbient.y;
          scene >> myLight.iAmbient.z;
          scene >> line; // iDiffuse
          scene >> myLight.iDiffuse.x;
          scene >> myLight.iDiffuse.y;
          scene >> myLight.iDiffuse.z;
          scene >> line; // iSpecular
          scene >> myLight.iSpecular.x;
          scene >> myLight.iSpecular.y;
          scene >> myLight.iSpecular.z;

          myScene->numLights++;
          if (myScene->numLights <= myScene->maxLights)
          {
            myScene->lights[myScene->numLights-1] = myLight;
          }
        }
      }
    }
    scene.close();
  }
  else
  {
    std::cout << "There was a problem opening the scene file\n";
  }
}

int main(int argc, char* argv[])
{
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Surface *surface;
  SDL_Texture *texture;

  if (argc != 2)
  {
    std::cout << "Missing scene file. Usage: ./program sceneFile\n";
    return(1);
  }
  char *sceneFileName = argv[1];

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

  scene myScene = {};
  // Read scene file
  readSceneFile(&myScene, sceneFileName);

  // initializeScene(&myScene);

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
  vec3 lowerLeftCorner = { -1.0, -1.0, -3.0 };
  // vec3 lowerLeftCorner = { -1.0, -1.0, -1.0 };

  // NOTE(ralntdir): generates random unsigned integers
  std::default_random_engine engine;
  // NOTE(ralntdir): generates random floats between [0, 1)
  std::uniform_real_distribution<real32> distribution(0, 1);

  // NOTE(ralntdir): From top to bottom
  for (int32 i = HEIGHT-1; i >= 0 ; i--)
  {
    for (int32 j = 0; j < WIDTH; j++)
    {
      vec3 backgroundColor = { 0.0, ((real32)i/HEIGHT), ((real32)j/WIDTH) };
      vec3 col = {};

      for (int32 samples = 0; samples < MAX_SAMPLES; samples++)
      {
        real32 u = real32(j + distribution(engine))/real32(WIDTH);
        real32 v = real32(i + distribution(engine))/real32(HEIGHT);

        ray cameraRay = {};
        cameraRay.origin = myScene.camera;
        cameraRay.direction = lowerLeftCorner + u*horizontalOffset + v*verticalOffset;

        vec3 tempCol = color(cameraRay, &myScene, backgroundColor);
        clamp(&tempCol);
        col += tempCol;
      }

      col /= (real32)MAX_SAMPLES;

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
