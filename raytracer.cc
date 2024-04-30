#include "raytracer.h"
#include "spinlock.h"
#include <random>
#include <atomic>

//------------------------------------------------------------------------------
/**
*	funtion to check if a atomic bool is true or false
*/
static bool testAndSet(atomic<int>& flag)
{
	int result = flag.fetch_add(0, std::memory_order_acquire);
	//check if the old value was 1 which means the bool is true
	return result == 1;
}
//------------------------------------------------------------------------------
/**
*/
Raytracer::Raytracer(unsigned w, unsigned h, std::vector<Color>& frameBuffer, unsigned rpp, unsigned bounces) :
    frameBuffer(frameBuffer),
    rpp(rpp),
    bounces(bounces),
    width(w),
    height(h),
	threads(threadCount)
{
	widthPerThread = width / threadCount;
	leftOverPixels = width % threadCount;
	cout << "Width per thread" << widthPerThread << endl;
	cout << "Left overs: " << leftOverPixels << endl;
	cout << threadCount << endl;
}
//------------------------------------------------------------------------------
/**
*/
Raytracer::~Raytracer()
{
    for (int i = 0; i < objects.size(); i++)
    {
		delete objects[i];
    }
	objects.clear();
	for (int i = 0; i < materials.size(); i++)
	{
		delete materials[i];
	}
	materials.clear();
}

//------------------------------------------------------------------------------
/**
*/
void
Raytracer::Raytrace()
{
    static int leet = 1337;
    std::mt19937 generator (leet++);
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
	atomic<int> renderCompleted(0);
	Spinlock lock;

	for (int i = 0; i < threadCount; ++i)
	{
		cout << i << endl;
		int startX, endX, startY = 0, endY = height;
		if (i < threadCount -1 || leftOverPixels == 0)
		{
			startX = i * (widthPerThread + 1);
			endX = startX + widthPerThread;
		}
		else if (leftOverPixels > 0 && i == threadCount - 1)
		{
			startX = i * widthPerThread;
			endX = startX * widthPerThread + leftOverPixels;
		}
		threads[i] = std::thread([this, &dis, &generator, &renderCompleted, startX, startY, endX, endY, &lock, &i]()
		{
			while (!testAndSet(renderCompleted))
			{
				for (int y = startY; y <= endY; ++y)
				{
					for (int x = startX; x <= endX; ++x)
					{
						Color color;
						for (int i = 0; i < this->rpp; ++i)
						{
							float u = ((float(x + dis(generator)) * (1.0f / this->width)) * 2.0f) - 1.0f;
							float v = ((float(y + dis(generator)) * (1.0f / this->height)) * 2.0f) - 1.0f;

							vec3 direction = vec3(u, v, -1.0f);
							direction = transform(direction, this->frustum);

							Ray ray = Ray(get_position(this->view), direction);
							color += this->TracePath(ray, 0);
							// delete ray;
						}
						// divide by number of samples per pixel, to get the average of the distribution
						color.r /= this->rpp;
						color.g /= this->rpp;
						color.b /= this->rpp;
						{
							cout << "Here: " << i << endl;
							lock.lock();
							frameBuffer[y * this->width + x] += color;
							lock.unlock();
						}
					}
				}
			}
		});
	}
	renderCompleted.store(1, std::memory_order_release);
	for (auto& thread : threads)
	{
		thread.join();
	}
  
}

//------------------------------------------------------------------------------
/**
 * @parameter n - the current bounce level
*/
Color
Raytracer::TracePath(Ray ray, unsigned n)
{
    vec3 hitPoint;
    vec3 hitNormal;
    Object* hitObject = nullptr;
    float distance = FLT_MAX;

    if (Raycast(ray, hitPoint, hitNormal, hitObject, distance, this->objects))
    {
        Ray scatteredRay = Ray(hitObject->ScatterRay(ray, hitPoint, hitNormal));
        if (n < this->bounces)
        {
            return hitObject->GetColor() * this->TracePath(scatteredRay, n + 1);
        }

        if (n == this->bounces)
        {
            return {0,0,0};
        }
    }

    return this->Skybox(ray.m);
}

//------------------------------------------------------------------------------
/**
*/
bool
Raytracer::Raycast(Ray ray, vec3& hitPoint, vec3& hitNormal, Object*& hitObject, float& distance, std::vector<Object*> world)
{
    bool isHit = false;
    HitResult closestHit;
    int numHits = 0;
    HitResult hit;

    //// First, sort the world objects
    //std::sort(world.begin(), world.end());

    //// then add all objects into a remaining objects set of unique objects, so that we don't trace against the same object twice
    //std::vector<Object*> uniqueObjects;
    //for (size_t i = 0; i < world.size(); ++i)
    //{
    //    Object* obj = world[i];
    //    std::vector<Object*>::iterator it = std::find_if(uniqueObjects.begin(), uniqueObjects.end(), 
    //            [obj](const auto& val)
    //            {
    //                return (obj->GetName() == val->GetName() && obj->GetId() == val->GetId());
    //            }
    //        );

    //    if (it == uniqueObjects.end())
    //    {
    //        uniqueObjects.push_back(obj);
    //    }
    //}

    for (auto object : world)
    {
        /*auto objectIt = uniqueObjects.begin();
        Object* object = *objectIt;*/

        auto opt = object->Intersect(ray, closestHit.t);
        if (opt.HasValue())
        {
            hit = opt.Get();
            assert(hit.t < closestHit.t);
            closestHit = hit;
            closestHit.object = object;
            isHit = true;
            numHits++;
        }
       // uniqueObjects.erase(objectIt);
    }

    hitPoint = closestHit.p;
    hitNormal = closestHit.normal;
    hitObject = closestHit.object;
    distance = closestHit.t;
    
    return isHit;
}


//------------------------------------------------------------------------------
/**
*/
void
Raytracer::Clear()
{
    for (auto& color : this->frameBuffer)
    {
        color.r = 0.0f;
        color.g = 0.0f;
        color.b = 0.0f;
    }
}

//------------------------------------------------------------------------------
/**
*/
void
Raytracer::UpdateMatrices()
{
    mat4 inverseView = inverse(this->view); 
    mat4 basis = transpose(inverseView);
    this->frustum = basis;
}

//------------------------------------------------------------------------------
/**
*/
Color
Raytracer::Skybox(vec3 direction)
{
    float t = 0.5*(direction.y + 1.0);
    vec3 vec = vec3(1.0, 1.0, 1.0) * (1.0 - t) + vec3(0.5, 0.7, 1.0) * t;
    return {(float)vec.x, (float)vec.y, (float)vec.z};
}
