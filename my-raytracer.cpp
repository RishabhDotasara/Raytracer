#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <omp.h>

struct Vec3 
{
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator*(float t) const { return Vec3(x * t, y * t, z * t); }
    Vec3 operator/(float t) const { return Vec3(x / t, y / t, z / t); }

    float dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }

    Vec3 normalize() const 
    {
        float len = std::sqrt(x*x + y*y + z*z);
        return (len > 0) ? (*this / len) : *this;
    }
};

struct Ray 
{
    Vec3 origin, direction;
    Ray(const Vec3& o, const Vec3& d) : origin(o), direction(d.normalize()) {}
};

struct Sphere 
{
    Vec3 center, color;
    float radius;
    float reflectivity=0.2f;

    Sphere(const Vec3& c, float r, const Vec3& col, float ref) : center(c), radius(r), color(col), reflectivity(ref) {}

    bool intersect(const Ray& ray, float& t, Vec3& hitPoint, Vec3& normal) const
    {
        Vec3 oc = ray.origin - center;
        float a = ray.direction.dot(ray.direction);
        float b = 2.0f * oc.dot(ray.direction);
        float c = oc.dot(oc) - (radius * radius);
        float d = b * b - 4 * a * c; // Discriminant

        if (d < 0) return false;

        float t0 = (-b - std::sqrt(d)) / (2 * a);
        float t1 = (-b + std::sqrt(d)) / (2 * a);
        t = (t0 > 0) ? t0 : t1;

        if (t < 0) return false;

        hitPoint = ray.origin + ray.direction * t;
        normal = (hitPoint - center).normalize();
        return true;
    }
};

struct LightSource 
{
    Vec3 position;
    LightSource(const Vec3& p) : position(p) {}
};

Vec3 trace(const Ray& ray, std::vector<LightSource>& lights, std::vector<Sphere>& spheres, int depth=3)
{
    Vec3 hitPoint, normal;
    float t, tClosest = 1e6;
    Sphere* hitSphere = nullptr;

    for (auto& s : spheres) 
    {
        if (s.intersect(ray, t, hitPoint, normal) && t < tClosest) 
        {
            tClosest = t;
            hitSphere = &s;
        }
    }

    if (!hitSphere) return Vec3(0, 0, 0);

    hitPoint = ray.origin + ray.direction * tClosest;
    normal = (hitPoint - hitSphere->center).normalize();
    Vec3 finalColor(0, 0, 0);
    
    for (auto& ls : lights)
    {
        Vec3 lightDir = (ls.position - hitPoint).normalize();
        Ray shadowRay(hitPoint + lightDir * 0.01f, lightDir);
        bool isShadow = false;

        for (auto& s : spheres) 
        {
            float tShadow;
            Vec3 tempHit, tempNormal;
            if (&s != hitSphere && s.intersect(shadowRay, tShadow, tempHit, tempNormal) && tShadow > 0.001f) 
            {
                isShadow = true;
                break;
            }
        }

        if (depth > 0 && hitSphere->reflectivity > 0.01f)
        {
            Vec3 RDirn =  ray.direction - normal * 2 *  ray.direction.dot(normal);
            // std::cout<<depth;
            Ray reflectedRay(hitPoint, RDirn.normalize());
            Vec3 reflectedColor = trace(reflectedRay, lights, spheres, depth -1);
            finalColor = finalColor * (1 - hitSphere->reflectivity) + reflectedColor * hitSphere->reflectivity;
        }

        if (!isShadow) 
        {
            float diff = std::max(0.0f, normal.dot(lightDir));
            finalColor = finalColor + (hitSphere->color * diff);
        }
    }

    return finalColor;
}

void render(int width, int height) 
{
    std::ofstream image("output.ppm");
    image << "P3\n" << width << " " << height << "\n255\n";

    float aspectRatio = (float)width / height;
    Vec3 camera(0, 0, -10);
    std::vector<Sphere> spheres = {
        Sphere(Vec3( 2, -0.5, 3), 1.0f, Vec3(0, 255, 0),0.2),  // Green sphere
        Sphere(Vec3( 0, -0.5, 5), 1.0f, Vec3(255, 0, 0),0.7),  // Red sphere
        Sphere(Vec3(-2, -0.5, 3), 1.0f, Vec3(0, 0, 255),0.2)   // Blue sphere
    };

    std::vector<LightSource> lights = {
        LightSource(Vec3( 5, 5, -2)),  // Top right
        LightSource(Vec3(-5, 5, -2)),  // Top left
        LightSource(Vec3( 0, 3, 5))    // Front middle
    };  
    #pragma omp parallel for schedule(dynamic) collapse(2)
    for (int y = height - 1; y >= 0; --y) 
    {
        for (int x = 0; x < width; ++x) 
        {
            std::cout<<"Pixel:"<<x<<" "<<y<<std::endl;
            float u = (2 * (x + 0.5f) / (float)width  - 1) * aspectRatio;
            float v = 1 - 2 * (y + 0.5f) / (float)height;

            Ray ray(camera, Vec3(u, v, 0) - camera);
            Vec3 color = trace(ray, lights, spheres);

            int r = std::max(0, std::min(255, (int)color.x));
            int g = std::max(0, std::min(255, (int)color.y));
            int b = std::max(0, std::min(255, (int)color.z));
            #pragma omp critical
            image << r << " " << g << " " << b << "\n";
        }
    }

    image.close();
}

int main() 
{
    render(3840,2160);
    std::cout << "Rendered output.ppm\n";
    return 0;
}