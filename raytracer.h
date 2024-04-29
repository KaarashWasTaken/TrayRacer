#pragma once
#include <vector>
#include <atomic>
#include "vec3.h"
#include "mat4.h"
#include "color.h"
#include "ray.h"
#include "object.h"
#include <float.h>

//------------------------------------------------------------------------------
/**
*/
class Raytracer
{
public:
    Raytracer(unsigned w, unsigned h, std::vector<Color>& frameBuffer, unsigned rpp, unsigned bounces);
    ~Raytracer();

    // start raytracing!
    void Raytrace();

    // add object to scene
    void AddObject(Object* obj);

    // single raycast, find object
    static bool Raycast(Ray ray, vec3& hitPoint, vec3& hitNormal, Object*& hitObject, float& distance, std::vector<Object*> objects);

    // set camera matrix
    void SetViewMatrix(mat4 val);

    // clear screen
    void Clear();

    // update matrices. Called automatically after setting view matrix
    void UpdateMatrices();

    // trace a path and return intersection color
    // n is bounce depth
    Color TracePath(Ray ray, unsigned n);

    // get the color of the skybox in a direction
    Color Skybox(vec3 direction);

    std::vector<Color>& frameBuffer;
    
    // rays per pixel
    unsigned rpp;
    // max number of bounces before termination
    unsigned bounces = 5;

    // width of framebuffer
    const unsigned width;
    // height of framebuffer
    const unsigned height;
    
    const vec3 lowerLeftCorner = { -2.0, -1.0, -1.0 };
    const vec3 horizontal = { 4.0, 0.0, 0.0 };
    const vec3 vertical = { 0.0, 2.0, 0.0 };
    const vec3 origin = { 0.0, 2.0, 10.0f };

    bool threadExit;
    const int threadCount = 24;

    // view matrix
    mat4 view;
    // Go from canonical to view frustum
    mat4 frustum;

    std::vector<Object*> objects;
    std::atomic_int workingTilesIndex;
    std::atomic_int completedTiles;
    std::atomic_int temp;
    std::atomic_bool tempb;
    std::atomic_bool tempb2;
};

inline void Raytracer::AddObject(Object* o)
{
    this->objects.push_back(o);
}

inline void Raytracer::SetViewMatrix(mat4 val)
{
    this->view = val;
    this->UpdateMatrices();
}
