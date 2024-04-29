#pragma once
#include "ray.h"
#include "color.h"
#include <float.h>
#include <string>
#include <memory>

class Object;

//------------------------------------------------------------------------------
/**
*/
struct HitResult
{
    // hit point
    vec3 p;
    // normal
    vec3 normal;
    // hit object, or nullptr
    Object* object = nullptr;
    // intersection distance
    float t = FLT_MAX;
};

template<class TYPE>
class Optional
{
public:
    Optional() {}
    Optional(HitResult hit) : hasValue(true)
    {
        this->value = std::make_shared<HitResult>(hit);
    }

    bool HasValue()
    {
        // check if this object has a value.
        if (this->hasValue)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    HitResult Get()
    {
        assert(this->HasValue());
        return *value;
    }

private:
    bool hasValue = false;
    std::shared_ptr<HitResult> value = nullptr;
};

//------------------------------------------------------------------------------
/**
*/
class Object
{
public:
    Object() 
    {
        static unsigned long long idCounter = 0;
        id = idCounter++;
        //// Reserve characters for naming this object something!
        //name = new char[256];
        //name[0] = 'U';
        //name[1] = 'n';
        //name[2] = 'n';
        //name[3] = 'a';
        //name[4] = 'm';
        //name[5] = 'e';
        //name[6] = 'd';
        //name[7] = '\0';

        //purpose = std::string("I don't have a purpose at the moment, but hopefully the programmer that overrides me will give me one. :)");
    }

    virtual ~Object()
    {
        // clean up name!
        //delete[] name;
    }

    virtual Optional<HitResult> Intersect(Ray ray, float maxDist) { return {}; };
    virtual Color GetColor() = 0;
    virtual Ray ScatterRay(Ray ray, vec3 point, vec3 normal) { return Ray({ 0,0,0 }, {1,1,1}); };
    //std::string GetName() { return std::string((const char*)name); }
    unsigned long long GetId() { return this->id; }

private:
    //volatile bool isBigObject = false;
    //volatile char* name;
    unsigned long long id;
    //std::string purpose;
};