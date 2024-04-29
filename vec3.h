#pragma once
#include <cmath>
#include <initializer_list>
#include <assert.h>

#define MPI 3.14159265358979323846

class vec3
{
public:
    vec3() : x(0), y(0), z(0){}

    vec3(double x, double y, double z) : x(x), y(y), z(z){}

    vec3(std::initializer_list<double> const il)
    {
        assert(il.size() == 3);

        int i = 0;
        for (auto v : il)
        {
            double* d = reinterpret_cast<double*>(this);
            d += i;
            *d = v;
            i++;
        }

    }

    ~vec3()
    {
    }

    vec3(vec3 const& rhs)
    {
        this->x = rhs.x;
        this->y = rhs.y;
        this->z = rhs.z;

    }

    inline vec3 operator+(vec3 const& rhs) { return {x + rhs.x, y + rhs.y, z + rhs.z};}
    inline vec3 operator-(vec3 const& rhs) { return {x - rhs.x, y - rhs.y, z - rhs.z};}
    inline vec3 operator-() { return {-x, -y, -z};}
    inline vec3 operator*(float const c) { return {x * c, y * c, z * c};}

    double x, y, z;

};

// Get length of 3D vector
inline double len(vec3 const& v)
{
    double a = v.x * v.x;
    a = a + v.y * v.y;
    a = a + v.z * v.z;
    double l = sqrt(a);
    return l;
}

// Get normalized version of v
inline vec3 normalize(vec3 v)
{
    double l = len(v);
    if (l == 0)
        return vec3(0,0,0);

    vec3 ret = vec3(v.x / l, v.y / l, v.z / l);
    return ret;
}

// piecewise multiplication between two vectors
inline vec3 mul(vec3 a, vec3 b)
{
    return {a.x * b.x, a.y * b.y, a.z * b.z};
}

// piecewise add between two vectors
inline vec3 add(vec3 a, vec3 b)
{
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

inline float dot(vec3 a, vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline vec3 reflect(vec3 v, vec3 n)
{
    return v - n * (2 * dot(v,n));
}

inline vec3 cross(vec3 a, vec3 b)
{
    return { a.y * b.z - a.z * b.y,
             a.z * b.x - a.x * b.z,
             a.x * b.y - a.y * b.x, };
}