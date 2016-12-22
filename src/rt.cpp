// TODO(ralntdir):
// Think how to clean SDL if the programs ends with some crash
// Finished the program if there is a problem at SDL/IMG init or
// when creating the window or the renderer
//
// Features to add:
// ray->sphere intersection (check if it's completed) -> I think so!
// Move width and height to scene?
// Move max_depth to scene?

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
#define MAX_DEPTH 5

#include <math.h>
#include "myMath.h"

struct ray
{
  vec3 origin;
  vec3 direction;
};

struct materialParameters
{
  vec3 ka;
  vec3 kd;
  vec3 ks;

  vec3 kr;

  real32 alpha;
};

enum mesh_type
{
  sphere,
  plane,
  triangle,
};

// NOTE(ralntdir): I don't like this aproximation to handle
// more than one type of mesh, but I also don't want to use
// OOP, so I'll go with this for now.
struct mesh
{
  mesh_type type;

  // Info for a sphere
  vec3 center;
  real32 radius;

  // Info for a plane
  vec3 normal;
  vec3 p0;

  // Info for a triangle
  vec3 a;
  vec3 b;
  vec3 c;

  materialParameters material;
};

enum light_type
{
  point,
  directional,
};

struct light
{
  vec3 position;
  vec3 intensity;
  light_type type;
};

struct scene
{
  vec3 camera;
  vec3 ul;
  vec3 ur;
  vec3 lr;
  vec3 ll;

  int32 maxMeshes = 8;
  int32 maxLights = 2;

  int32 numMeshes;
  int32 numLights;
  light lights[2];
  mesh meshes[8];
};

bool hitSphere(mesh mySphere, ray myRay, real32 *t)
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

bool hitPlane(mesh myPlane, ray myRay, real32 *t)
{
  bool result = false;

  vec3 A = myRay.origin - myPlane.p0;
  real32 dotProductNDirection = dotProduct(myPlane.normal, myRay.direction);
  real32 dotProductNA = dotProduct(myPlane.normal, A);

  real32 tHit = -1.0;

  if (dotProductNDirection != 0.0)
  {
    tHit = -dotProductNA/dotProductNDirection;
  }

  if (tHit > 0.0)
  {
    *t = tHit;
    result = true;
  }

  return(result);
}

// TODO(ralntdir): add attenuation for point lights
// TODO(ralntdir): technically this is not Phong Shading
vec3 phongShading(light myLight, mesh myMesh, vec3 camera, vec3 hitPoint, real32 visible)
{
  vec3 result;

  // *N vector (normal at hit point)
  // *L vector (lightPosition - hitPoint)
  vec3 N = {};
  if (myMesh.type == sphere)
  {
    N = normalize(hitPoint - myMesh.center);
  }
  else if (myMesh.type == plane)
  {
    N = normalize(myMesh.normal);
  }
  else if (myMesh.type == triangle)
  {
    vec3 ab = myMesh.a - myMesh.b;
    vec3 ac = myMesh.a - myMesh.c;
    N = normalize(crossProduct(ab, ac));
  }

  vec3 L = {};
  if (myLight.type == point)
  {
    L = normalize(myLight.position - hitPoint);
  }
  else if (myLight.type == directional)
  {
    L = normalize(-myLight.position);
  }
  real32 dotProductLN = max(dotProduct(L, N), 0.0);
  real32 filterSpecular = dotProductLN > 0.0 ? 1.0 : 0.0;

  // *R vector (reflection of L -> 2(L·N)N - L)
  // *V vector (camera - hitPoint)
  vec3 R = normalize(2*dotProduct(L, N)*N - L);
  vec3 V = normalize(camera - hitPoint);

  // Only add specular component if you have diffuse,
  // if dotProductLN > 0.0
  result = visible*myMesh.material.kd*myLight.intensity*dotProductLN +
           visible*filterSpecular*myMesh.material.ks*myLight.intensity*pow(max(dotProduct(R, V), 0.0), myMesh.material.alpha);

  return(result);
}

bool hitMesh(mesh myMesh, ray myRay, real32 *t)
{
  bool result = false;

  if (myMesh.type == sphere)
  {
    result = hitSphere(myMesh, myRay, t);
  }
  else if (myMesh.type == plane)
  {
    result = hitPlane(myMesh, myRay, t);

// NOTE(ralntdir): This works only for a plane where
// one of the coordinates is fixed (a "2D" plane), not
// one where the 3 coordinates vary in the 3D space.
#if 0
    vec3 hitPoint = myRay.origin + *t*myRay.direction;

    if ((hitPoint.x < myMesh.p0.x - 1) || (hitPoint.x > myMesh.p0.x + 2) ||
        (hitPoint.y < myMesh.p0.y - 2) || (hitPoint.y > myMesh.p0.y + 1))
    {
      result = false;
    }
#endif
  }
  else if (myMesh.type == triangle)
  {
    vec3 ab = myMesh.a - myMesh.b;
    vec3 ac = myMesh.a - myMesh.c;
    vec3 planeNormal = normalize(crossProduct(ab, ac));
    // printVector(planeNormal);
    mesh plane = {};
    plane.normal = planeNormal;
    plane.p0 = myMesh.a;
    result = hitPlane(plane, myRay, t);

    if (result == true)
    {
      // std::cout << "Result True\n";

      // TODO(ralntdir): Check if the point is inside the triangle
      // using barycentric coordinates (?)

      // Using barycentric coordinates...
      // We know that the hit point is defined with the following equation
      // P = wA + uB + vC;
      // and that w = 1 - u - v
      // So P can be put in the following way
      // P = A - uA - vA + uB + vC
      // or
      // P = A + u(B - A) + v(C - B); (I)

      // Using the ray equation
      // We also know that the hit point is defined with the following equation
      // P = O + tD; (II)

      // If we substitute (II) into (I) we get
      // O + tD = A + u(B - A) + v(C - B)
      // and rearranging it we get
      // O - A = -tD + u(B - A) + v(C - B) (III)

      //                         [ t
      // [ -D, B - A , C - B ] *   u   = O - A
      //                           v ]

      // Using Cramer's rule and the determinants, we can get the values for
      // u, v and t, and check if we have hit the triangle based in the
      // sign of those values

      // Think how to apply the Cramer's rule and how to calculate the
      // determinants.
      vec3 E1 = myMesh.b - myMesh.a;
      vec3 E2 = myMesh.c - myMesh.a;
      vec3 T = myRay.origin - myMesh.a;
      vec3 minusD = -myRay.direction;

      real32 invDetM = 1.0 / scalarTripleProduct(minusD, E1, E2);

      real32 detM2 = scalarTripleProduct(minusD, T, E2);
      real32 u = detM2*invDetM;

      if (u < 0.0 || u > 1)
      {
        result = false;
      }
      else
      {
        real32 detM3 = scalarTripleProduct(minusD, E1, T);

        real32 v = detM3*invDetM;

        if (v < 0.0 ||  u + v > 1.0)
        {
          result = false;
        }
        else
        {
          real32 detM1 = scalarTripleProduct(T, E1, E2);
          *t = detM1*invDetM;

          result = true;
        }
      }
    }
  }

  return(result);
}


vec3 blinnPhongShading(light myLight, mesh mySphere, vec3 camera, vec3 hitPoint, real32 visible)
{
  vec3 result;

  // *N vector (normal at hit point)
  // *L vector (lightPosition - hitPoint)
  vec3 N = normalize(hitPoint - mySphere.center);
  vec3 L = {};
  if (myLight.type == point)
  {
    L = normalize(myLight.position - hitPoint);
  }
  else if (myLight.type == directional)
  {
    L = normalize(-myLight.position);
  }
  real32 dotProductLN = max(dotProduct(L, N), 0.0);
  real32 filterSpecular = dotProductLN > 0.0 ? 1.0 : 0.0;

  // *N vector (normal at hit point)
  // *H half vector (normalize(L+V)), L and V have to be normalized
  // *V vector (camera - hitPoint)
  vec3 V = normalize(camera - hitPoint);
  vec3 H = normalize(L + V);

  // Only add specular component if you have diffuse,
  // if dotProductLN > 0.0
  result = visible*mySphere.material.kd*myLight.intensity*dotProductLN +
           visible*filterSpecular*mySphere.material.ks*myLight.intensity*pow(max(dotProduct(N, H), 0.0), mySphere.material.alpha);

  return(result);
}

ray getShadowRay(light myLight, vec3 hitPoint, vec3 normalAtHitPoint)
{
  ray result = {};

  // NOTE(ralntdir): delta to avoid shadow acne.
  real32 bias = 0.01;

  result.origin = hitPoint + normalAtHitPoint*bias;

  if (myLight.type == directional)
  {
    result.direction = normalize(-myLight.position);
  }
  else if (myLight.type == point)
  {
    result.direction = normalize(myLight.position - hitPoint);
  }

  return(result);
}

vec3 color(ray myRay, scene *myScene, vec3 backgroundColor, int32 depth)
{
  // vec3 result = backgroundColor;
  vec3 result = { 0.0, 0.0, 0.0 };

  if (depth <= MAX_DEPTH)
  {
    real32 mint = FLT_MAX;
    real32 t = -1.0;

    for (int i = 0; i < myScene->numMeshes; i++)
    {
      mesh myMesh = myScene->meshes[i];
      if (hitMesh(myMesh, myRay, &t))
      {
        if ((t >= 0.0) && (t < mint))
        {
          result = {};
          // NOTE(ralntdir): Let's suppose that ia is (1.0, 1.0, 1.0)
          if (depth == 1)
          {
            result += myMesh.material.ka;
          }
          mint = t;
          vec3 hitPoint = myRay.origin + t*myRay.direction;

          vec3 N = {};

          if (myMesh.type == sphere)
          {
            N = normalize(hitPoint - myMesh.center);
          }
          else if (myMesh.type == plane)
          {
            N = normalize(myMesh.normal);
          }
          else if (myMesh.type == triangle)
          {
            vec3 ab = myMesh.a - myMesh.b;
            vec3 ac = myMesh.a - myMesh.c;
            N = normalize(crossProduct(ab, ac));
          }

          for (int j = 0; j < myScene->numLights; j++)
          {
            light myLight = myScene->lights[j];

            ray shadowRay = getShadowRay(myLight, hitPoint, N);

            real32 visible = 1.0;

            for (int k = 0; k < myScene->numMeshes; k++)
            {
              // TODO(ralntdir): check if this filtering is right
              if (i != k)
              {
                mesh myMesh1 = myScene->meshes[k];
                real32 t1 = -1.0;
                if (hitMesh(myMesh1, shadowRay, &t1))
                {
                  visible = 0.0;
                  break;
                }
              }
            }
            result += phongShading(myLight, myMesh, myScene->camera, hitPoint, visible);
          }

          // Add reflection
          ray reflectedRay = {};
          reflectedRay.origin = hitPoint + N*0.01;
          reflectedRay.direction = 2*dotProduct(-myRay.direction, N)*N + myRay.direction;

          result += myMesh.material.kr*color(reflectedRay, myScene, backgroundColor, depth+1);
        }
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
      if (line[0] == '#')
      {
        std::getline(scene, line);
        std::cout << line << "\n";
      }
      else
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
          mesh mySphere = {};
          mySphere.type = sphere;

          scene >> line; // center
          scene >> mySphere.center.x;
          scene >> mySphere.center.y;
          scene >> mySphere.center.z;
          scene >> line; // radius
          scene >> mySphere.radius;
          scene >> line; // ka
          scene >> mySphere.material.ka.r;
          scene >> mySphere.material.ka.g;
          scene >> mySphere.material.ka.b;
          scene >> line; // kd
          scene >> mySphere.material.kd.r;
          scene >> mySphere.material.kd.g;
          scene >> mySphere.material.kd.b;
          scene >> line; // ks
          scene >> mySphere.material.ks.r;
          scene >> mySphere.material.ks.g;
          scene >> mySphere.material.ks.b;
          scene >> line; // kr || alpha
          if (line == "kr")
          {
            scene >> mySphere.material.kr.r;
            scene >> mySphere.material.kr.g;
            scene >> mySphere.material.kr.b;
            scene >> line; // alpha
            scene >> mySphere.material.alpha;
          }
          else if (line == "alpha")
          {
            scene >> mySphere.material.alpha;
          }

          myScene->numMeshes++;
          if (myScene->numMeshes <= myScene->maxMeshes)
          {
            myScene->meshes[myScene->numMeshes-1] = mySphere;
          }
        }
        else if (line == "plane")
        {
          mesh myPlane = {};
          myPlane.type = plane;

          scene >> line; // normal
          scene >> myPlane.normal.x;
          scene >> myPlane.normal.y;
          scene >> myPlane.normal.z;
          scene >> line; // p0
          scene >> myPlane.p0.x;
          scene >> myPlane.p0.y;
          scene >> myPlane.p0.z;
          scene >> line; // ka
          scene >> myPlane.material.ka.r;
          scene >> myPlane.material.ka.g;
          scene >> myPlane.material.ka.b;
          scene >> line; // kd
          scene >> myPlane.material.kd.r;
          scene >> myPlane.material.kd.g;
          scene >> myPlane.material.kd.b;
          scene >> line; // ks
          scene >> myPlane.material.ks.r;
          scene >> myPlane.material.ks.g;
          scene >> myPlane.material.ks.b;
          scene >> line; // kr || alpha
          if (line == "kr")
          {
            scene >> myPlane.material.kr.r;
            scene >> myPlane.material.kr.g;
            scene >> myPlane.material.kr.b;
            scene >> line; // alpha
            scene >> myPlane.material.alpha;
          }
          else if (line == "alpha")
          {
            scene >> myPlane.material.alpha;
          }

          myScene->numMeshes++;
          if (myScene->numMeshes <= myScene->maxMeshes)
          {
            myScene->meshes[myScene->numMeshes-1] = myPlane;
          }
        }
        else if (line == "triangle")
        {
          mesh myTriangle = {};
          myTriangle.type = triangle;

          scene >> line; // a
          scene >> myTriangle.a.x;
          scene >> myTriangle.a.y;
          scene >> myTriangle.a.z;
          scene >> line; // b
          scene >> myTriangle.b.x;
          scene >> myTriangle.b.y;
          scene >> myTriangle.b.z;
          scene >> line; // c
          scene >> myTriangle.c.x;
          scene >> myTriangle.c.y;
          scene >> myTriangle.c.z;
          scene >> line; // ka
          scene >> myTriangle.material.ka.r;
          scene >> myTriangle.material.ka.g;
          scene >> myTriangle.material.ka.b;
          scene >> line; // kd
          scene >> myTriangle.material.kd.r;
          scene >> myTriangle.material.kd.g;
          scene >> myTriangle.material.kd.b;
          scene >> line; // ks
          scene >> myTriangle.material.ks.r;
          scene >> myTriangle.material.ks.g;
          scene >> myTriangle.material.ks.b;
          scene >> line; // kr || alpha
          if (line == "kr")
          {
            scene >> myTriangle.material.kr.r;
            scene >> myTriangle.material.kr.g;
            scene >> myTriangle.material.kr.b;
            scene >> line; // alpha
            scene >> myTriangle.material.alpha;
          }
          else if (line == "alpha")
          {
            scene >> myTriangle.material.alpha;
          }

          myScene->numMeshes++;
          if (myScene->numMeshes <= myScene->maxMeshes)
          {
            myScene->meshes[myScene->numMeshes-1] = myTriangle;
          }
        }
        else if (line == "light")
        {
          light myLight = {};

          scene >> line; // position
          scene >> myLight.position.x;
          scene >> myLight.position.y;
          scene >> myLight.position.z;
          scene >> line; // intensity
          scene >> myLight.intensity.r;
          scene >> myLight.intensity.g;
          scene >> myLight.intensity.b;
          scene >> line; // type
          scene >> line;

          if (line.compare("point") == 0)
          {
            myLight.type = point;
          }
          else if (line.compare("directional") == 0)
          {
            myLight.type = directional;
          }

          myScene->numLights++;
          if (myScene->numLights <= myScene->maxLights)
          {
            myScene->lights[myScene->numLights-1] = myLight;
          }
        }
        else if (line == "ul")
        {
          vec3 ul = {};
          scene >> ul.x;
          scene >> ul.y;
          scene >> ul.z;

          myScene->ul = ul;
        }
        else if (line == "ur")
        {
          vec3 ur = {};
          scene >> ur.x;
          scene >> ur.y;
          scene >> ur.z;

          myScene->ur = ur;
        }
        else if (line == "lr")
        {
          vec3 lr = {};
          scene >> lr.x;
          scene >> lr.y;
          scene >> lr.z;

          myScene->lr = lr;
        }
        else if (line == "ll")
        {
          vec3 ll = {};
          scene >> ll.x;
          scene >> ll.y;
          scene >> ll.z;

          myScene->ll = ll;
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

  if (argc < 2)
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

  // Create a .ppm file 
  std::ofstream ofs("image.ppm", std::ofstream::out | std::ofstream::binary);

  ofs << "P3\n";
  ofs << "# image.ppm\n";
  ofs << WIDTH << " " << HEIGHT << "\n";
  ofs << MAX_COLOR << "\n";

  vec3 horizontalOffset = myScene.ur - myScene.ul;
  vec3 verticalOffset = myScene.ul - myScene.ll;
  vec3 lowerLeftCorner = myScene.ll;

  // NOTE(ralntdir): generates random unsigned integers
  std::default_random_engine engine;
  // NOTE(ralntdir): generates random floats between [0, 1)
  std::uniform_real_distribution<real32> distribution(0, 1);

  int32 depth = 1;

  // NOTE(ralntdir): From top to bottom
  for (int32 i = HEIGHT-1; i >= 0 ; i--)
  {
    for (int32 j = 0; j < WIDTH; j++)
    {
      vec3 backgroundColor = { 0.0, ((real32)i/HEIGHT), ((real32)j/WIDTH) };
      vec3 col = {};

#if 0
      #pragma omp declare reduction(vectorReduction:vec3: \
                                    omp_out += omp_in)

      #pragma omp parallel for reduction(vectorReduction:col)
#endif
      for (int32 samples = 0; samples < MAX_SAMPLES; samples++)
      {
        real32 u = real32(j + distribution(engine))/real32(WIDTH);
        real32 v = real32(i + distribution(engine))/real32(HEIGHT);

        ray cameraRay = {};
        cameraRay.origin = myScene.camera;
        cameraRay.direction = lowerLeftCorner + u*horizontalOffset + v*verticalOffset;

        vec3 tempCol = color(cameraRay, &myScene, backgroundColor, depth);
        clamp(&tempCol);
        col += tempCol;
      }

      col /= (real32)MAX_SAMPLES;

      int32 r = int32(255.0*col.r);
      int32 g = int32(255.0*col.g);
      int32 b = int32(255.0*col.b);

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
