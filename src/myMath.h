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
  struct
  {
    real32 r;
    real32 g;
    real32 b;
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

inline vec3 operator*(vec3 a, vec3 b)
{
  vec3 result;

  result.x = a.x * b.x;
  result.y = a.y * b.y;
  result.z = a.z * b.z;

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

inline vec3 operator/(vec3 b, real32 a)
{
  real32 k = 1.0/a;

  vec3 result = b*k;

  return(result);
}

inline vec3 operator+=(vec3 &a, vec3 b)
{
  a = a + b;

  return(a);
}

inline vec3 operator/=(vec3 &b, real32 a)
{
  b = b/a;

  return(b);
}

real32 dotProduct(vec3 vector1, vec3 vector2)
{
  real32 result;

  result = vector1.x*vector2.x + vector1.y*vector2.y + vector1.z*vector2.z;

  return(result);
}

vec3 crossProduct(vec3 a, vec3 b)
{
  vec3 result = {};

  result.x = a.y*b.z - a.z*b.y;
  result.y = a.z*b.x - a.x*b.z;
  result.z = a.x*b.y - a.y*b.x;

  return(result);
}

void printVector(vec3 vector)
{
  std::cout << "x: " << vector.x << ", y: " << vector.y << ", z:" << vector.z << "\n";
}

real32 clamp(real32 value)
{
  real32 result = value;

  if (result < 0.0)
  {
    result = 0.0;
  }
  else if (result > 1.0)
  {
    result = 1.0;
  }

  return(result);
}

void clamp(vec3 *vector)
{
  vector->x = clamp(vector->x);
  vector->y = clamp(vector->y);
  vector->z = clamp(vector->z);
}

real32 length(vec3 vector)
{
  real32 result = sqrt(vector.x*vector.x + vector.y*vector.y + vector.z*vector.z);

  return(result);
}

vec3 normalize(vec3 vector)
{
  vec3 result = {};

  real32 k = 1/length(vector);

  result = k*vector;

  return(result);
}

real32 max(real32 a, real32 b)
{
  real32 result = b;

  if (a > b)
  {
    result = a;
  }

  return(result);
}

#endif
