#include <stdio.h>
#include <chrono>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <fstream>
#include "window.h"
#include "vec3.h"
#include "raytracer.h"
#include "sphere.h"
#include "flags.h"

#define degtorad(angle) angle * MPI / 180
using std::cout;
using std::cin;
using std::endl;

static void SaveImage(Raytracer& rt, std::vector<Color>& framebuffer)
{
	// Write to PGM file
	std::ofstream saveimage("SavedFrame.pgm", std::ios::binary);
	assert(!saveimage.is_open && "<main> savefile could not be opened");
	saveimage << "P5\n"; // Magic number for grayscale PGM
	saveimage << rt.width << " " << rt.height << "\n"; // Image dimensions
	saveimage << 255 << "\n"; // Maximum grayscale value
	std::vector<Color> reverseFramebuffer = framebuffer;
	std::reverse(reverseFramebuffer.begin(), reverseFramebuffer.end());
	for (const auto& pixel : reverseFramebuffer) {
		unsigned char r = static_cast<unsigned char>(std::clamp(pixel.r, 0.0f, 1.0f) * 255.0f);
		unsigned char g = static_cast<unsigned char>(std::clamp(pixel.g, 0.0f, 1.0f) * 255.0f);
		unsigned char b = static_cast<unsigned char>(std::clamp(pixel.b, 0.0f, 1.0f) * 255.0f);
		unsigned char gray = (r + g + b) / 3;
		//saveimage.write(reinterpret_cast<char*>(&gray), sizeof(gray));
		saveimage << gray;
	}
	saveimage.close();
}
int main()
{ 
    Display::Window wnd;
    
    wnd.SetTitle("TrayRacer");
    
    if (!wnd.Open())
        return 1;
    
    std::vector<Color> framebuffer; 

    const unsigned w = 500;
    const unsigned h = 300;
    framebuffer.resize(w * h);
    
    int raysPerPixel = 1;
    int maxBounces = 5;

    Raytracer rt = Raytracer(w, h, framebuffer, raysPerPixel, maxBounces);

    // Create some objects
    Material* mat = new Material();
    mat->type = Lambertian;
    mat->color = { 0.5,0.5,0.5 };
    mat->roughness = 0.3;
    Sphere* ground = new Sphere(1000, { 0,-1000, -1 }, mat);
    rt.AddObject(ground);

    for (int it = 0; it < 12; it++)
    {
        {
            Material* mat = new Material();
                mat->type = Lambertian;
                float r = RandomFloat();
                float g = RandomFloat();
                float b = RandomFloat();
                mat->color = { r,g,b };
                mat->roughness = RandomFloat();
                const float span = 10.0f;
                Sphere* ground = new Sphere(
                    RandomFloat() * 0.7f + 0.2f,
                    {
                        RandomFloatNTP() * span,
                        RandomFloat() * span + 0.2f,
                        RandomFloatNTP() * span
                    },
                    mat);
            rt.AddObject(ground);
        }{
            Material* mat = new Material();
            mat->type = Conductor;
            float r = RandomFloat();
            float g = RandomFloat();
            float b = RandomFloat();
            mat->color = { r,g,b };
            mat->roughness = RandomFloat();
            const float span = 30.0f;
            Sphere* ground = new Sphere(
                RandomFloat() * 0.7f + 0.2f,
                {
                    RandomFloatNTP() * span,
                    RandomFloat() * span + 0.2f,
                    RandomFloatNTP() * span
                },
                mat);
            rt.AddObject(ground);
        }{
            Material* mat = new Material();
            mat->type = Dielectric;
            float r = RandomFloat();
            float g = RandomFloat();
            float b = RandomFloat();
            mat->color = { r,g,b };
            mat->roughness = RandomFloat();
            mat->refractionIndex = 1.65;
            const float span = 25.0f;
            Sphere* ground = new Sphere(
                RandomFloat() * 0.7f + 0.2f,
                {
                    RandomFloatNTP() * span,
                    RandomFloat() * span + 0.2f,
                    RandomFloatNTP() * span
                },
                mat);
            rt.AddObject(ground);
        }
    }
    
    bool exit = false;
	bool saveFrame = false;
    // camera
    bool resetFramebuffer = false;
    vec3 camPos = { 0,1.0f,10.0f };
    vec3 moveDir = { 0,0,0 };

    wnd.SetKeyPressFunction([&exit, &moveDir, &resetFramebuffer, &saveFrame](int key, int scancode, int action, int mods)
    {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            exit = true;
            break;
        case GLFW_KEY_W:
            moveDir.z -= 1.0f;
            resetFramebuffer |= true;
            break;
        case GLFW_KEY_S:
            moveDir.z += 1.0f;
            resetFramebuffer |= true;
            break;
        case GLFW_KEY_A:
            moveDir.x -= 1.0f;
            resetFramebuffer |= true;
            break;
        case GLFW_KEY_D:
            moveDir.x += 1.0f;
            resetFramebuffer |= true;
            break;
        case GLFW_KEY_SPACE:
            moveDir.y += 1.0f;
            resetFramebuffer |= true;
            break;
        case GLFW_KEY_LEFT_CONTROL:
            moveDir.y -= 1.0f;
            resetFramebuffer |= true;
            break;
		case GLFW_KEY_P:
			saveFrame = true;
			break;
        default:
            break;
        }
    });

    float pitch = 0;
    float yaw = 0;
    float oldx = 0;
    float oldy = 0;

    wnd.SetMouseMoveFunction([&pitch, &yaw, &oldx, &oldy, &resetFramebuffer](double x, double y)
    {
        x *= -0.1;
        y *= -0.1;
        yaw = x - oldx;
        pitch = y - oldy;
        resetFramebuffer |= true;
        oldx = x;
        oldy = y;
    });

    float rotx = 0;
    float roty = 0;

    // number of accumulated frames
    int frameIndex = 0;

    std::vector<Color> framebufferCopy;
    framebufferCopy.resize(w * h);

    // rendering loop
    while (wnd.IsOpen() && !exit)
    {
        resetFramebuffer = false;
        moveDir = {0,0,0};
        pitch = 0;
        yaw = 0;

        // poll input
        wnd.Update();

        rotx -= pitch;
        roty -= yaw;

        moveDir = normalize(moveDir);

        mat4 xMat = (rotationx(rotx));
        mat4 yMat = (rotationy(roty));
        mat4 cameraTransform = multiply(yMat, xMat);

        camPos = camPos + transform(moveDir * 0.2f, cameraTransform);
        
        cameraTransform.m30 = camPos.x;
        cameraTransform.m31 = camPos.y;
        cameraTransform.m32 = camPos.z;

        rt.SetViewMatrix(cameraTransform);
        
        if (resetFramebuffer)
        {
            rt.Clear();
            frameIndex = 0;
        }

        rt.Raytrace();
		if (saveFrame)
		{
			SaveImage(rt, framebuffer);
			saveFrame = false;
		}
        frameIndex++;

        // Get the average distribution of all samples
        {
            size_t p = 0;
            for (Color const& pixel : framebuffer)
            {
                framebufferCopy[p] = pixel;
                framebufferCopy[p].r /= frameIndex;
                framebufferCopy[p].g /= frameIndex;
                framebufferCopy[p].b /= frameIndex;
                p++;
            }
        }

        glClearColor(0, 0, 0, 1.0);
        glClear( GL_COLOR_BUFFER_BIT );

        wnd.Blit((float*)&framebufferCopy[0], w, h);
        wnd.SwapBuffers();
    }

    if (wnd.IsOpen())
        wnd.Close();

    return 0;
} 

//int main(int argc, char* argv[])
//{
//	//argument flag declaration
//    flags::args arguments = flags::args(argc, argv);
//    
//    std::vector<Color> framebuffer;
//	//argument settings
//    unsigned int w, h;
//    int raysPerPixel, maxBounces, nrOfSpheres;
//
//    w = arguments.get<int>("width", 100);
//    h = arguments.get<int>("height", 100);
//    raysPerPixel = arguments.get<int>("rays", 5);
//    maxBounces = arguments.get<int>("bounces", 5);
//    nrOfSpheres = arguments.get<int>("spheres", 10);
//
//	bool exit = false;
//    while (!exit)
//    {
//		framebuffer.resize(w * h);
//		Raytracer rt = Raytracer(w, h, framebuffer, raysPerPixel, maxBounces);
//		// Create some objects
//		Material* mat = new Material();
//		rt.AddMaterial(mat);
//		mat->type = Lambertian;
//		mat->color = { 0.5,0.5,0.5 };
//		mat->roughness = 0.3;
//		Sphere* ground = new Sphere(1000, { 0,-1000, -1 }, mat);
//		rt.AddObject(ground);
//
//		for (int it = 0; it < nrOfSpheres; it++)
//		{
//		    {
//		        Material* mat = new Material();
//				rt.AddMaterial(mat);
//		        mat->type = Lambertian;
//		        float r = RandomFloat();
//		        float g = RandomFloat();
//		        float b = RandomFloat();
//		        mat->color = { r,g,b };
//		        mat->roughness = RandomFloat();
//		        const float span = 10.0f;
//		        Sphere* ground = new Sphere(
//		            RandomFloat() * 0.7f + 0.2f,
//		            {
//		                RandomFloatNTP() * span,
//		                RandomFloat() * span + 0.2f,
//		                RandomFloatNTP() * span
//		            },
//		            mat);
//		        rt.AddObject(ground);
//		    }
//		}
//		// camera
//		vec3 camPos = { 0,1.0f,10.0f };
//		vec3 moveDir = { 0,0,0 };
//		float rotx = 0;
//		float roty = 0;
//		// rendering loop
//		auto startTime = std::chrono::high_resolution_clock::now();
//        moveDir = { 0,0,0 };
//        moveDir = normalize(moveDir);
//
//        mat4 xMat = (rotationx(rotx));
//        mat4 yMat = (rotationy(roty));
//        mat4 cameraTransform = multiply(yMat, xMat);
//
//        camPos = camPos + transform(moveDir * 0.2f, cameraTransform);
//        cameraTransform.m30 = camPos.x;
//        cameraTransform.m31 = camPos.y;
//        cameraTransform.m32 = camPos.z;
//
//        rt.SetViewMatrix(cameraTransform);
//        rt.Clear();
//        rt.Raytrace();
//		SaveImage(rt, framebuffer);
//		cout << "Would you like to run the program again? [y]es/[n]o " << endl;
//		char input;
//		std::cin >> input;
//		//Input to rerun program
//		switch (input)
//		{
//		case 'y':
//			cout << "Running again" << endl;
//			exit = false;
//			break;
//		case 'n':
//			cout << "Quitting" << endl;
//			exit = true;
//			break;
//		case 'yes':
//			cout << "Running again" << endl;
//			exit = false;
//			break;
//		case 'no':
//			cout << "Quitting" << endl;
//			exit = true;
//			break;
//		case 'Y':
//			cout << "Running again" << endl;
//			exit = false;
//			break;
//		case 'N':
//			cout << "Quitting" << endl;
//			exit = true;
//			break;
//		case 'Yes':
//			cout << "Running again" << endl;
//			exit = false;
//			break;
//		case 'No':
//			cout << "Quitting" << endl;
//			exit = true;
//			break;
//		default:
//			exit = true;
//			cout << "[Input invalid]: Quitting" << endl;
//		}
//		if (exit)
//		{
//			break;
//		}
//		//New setting for rerun
//		else
//		{
//			cout << "Width: ";
//			cin >> w;
//			cout << endl << "Height: ";
//			cin >> h;
//			cout << endl << "Rays Per Pixel: ";
//			cin >> raysPerPixel;
//			cout << endl << "Ray Max Bounces: ";
//			cin >> maxBounces;
//			cout << endl << "Amount of Objects: ";
//			cin >> nrOfSpheres;
//		}
//    }
//}