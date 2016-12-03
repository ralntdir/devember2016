#ifndef MYMATH
#define MYMATH_H

union vec3
{
  struct
  {
    real32 x;
    real32 y;
    real32 z;
  };
  real32 e[3];
};

inline vec3 operator-(vec3 a)
{
  vec3 result;

  result.x = -a.x;
  result.y = -a.y;
  result.z = -a.z;

  return(result);
}

inline vec3 operator+(vec3 a, vec3 b)
{
  vec3 result;

  result.x = a.x + b.x;
  result.y = a.y + b.y;
  result.z = a.z + b.z;

  return(result);
}

inline vec3 operator-(vec3 a, vec3 b)
{
  vec3 result;

  result.x = a.x - b.x;
  result.y = a.y - b.y;
  result.z = a.z - b.z;

  return(result);
}

inline vec3 operator*(real32 a, vec3 b)
{
  vec3 result;

  result.x = a*b.x;
  result.y = a*b.y;
  result.z = a*b.z;

  return(result);
}

inline vec3 operator*(vec3 b, real32 a)
{
  vec3 result = a*b;

  return(result);
}

real32 dotProduct(vec3 vector1, vec3 vector2)
{
  real32 result;

  result = vector1.x*vector2.x + vector1.y*vector2.y + vector1.z*vector2.z;

  return(result);
}

void printVector(vec3 vector)
{
  std::cout << "x: " << vector.x << ", y: " << vector.y << ", z:" << vector.z << "\n";
}

#endif
