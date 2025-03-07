#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <algorithm> // for std::min and std::max

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
}; // 🔴 Missing semicolon fixed!

struct Sphere 
{
    Vec3 center, color;
    float radius;

    Sphere(const Vec3& c, float r, const Vec3& col) : center(c), radius(r), color(col) {}

    bool intersect(const Ray& ray, float& t, Vec3& hitPoint, Vec3& normal) const
    {
        Vec3 oc = ray.origin - center;
        float a = ray.direction.dot(ray.direction);
        float b = 2.0f * oc.dot(ray.direction);
        float c = oc.dot(oc) - (radius * radius);
        float d = b * b - 4 * a * c; // Discriminant

        if (d < 0) return false; // No intersection

        float t0 = (-b - std::sqrt(d)) / (2 * a);
        float t1 = (-b + std::sqrt(d)) / (2 * a);
        t = (t0 > 0) ? t0 : t1;

        if (t < 0) return false; // Intersection behind the camera

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

Vec3 trace(const Ray& ray, const LightSource& light, const Sphere& sphere)
{
    Vec3 hitPoint, normal;
    float t;
    
    if (!sphere.intersect(ray, t, hitPoint, normal)) 
        return Vec3(0, 0, 0); // No intersection, return black

    // Shadow Ray
    Vec3 lightDir = (light.position - hitPoint).normalize();
    Ray shadowRay(hitPoint + lightDir * 0.001f, lightDir); // Offset to avoid self-shadowing

    float tShadow;
    Vec3 tempHit, tempNormal;
    if (sphere.intersect(shadowRay, tShadow, tempHit, tempNormal)) 
        return Vec3(0, 0, 0); // In shadow, return black

    // Diffuse shading
    float diff = std::max(0.0f, normal.dot(lightDir));
    return sphere.color * diff; // Apply shading
}

void render(int width, int height) 
{
    std::ofstream image("output.ppm");
    image << "P3\n" << width << " " << height << "\n255\n";

    Vec3 camera(0, 0, -5);
    Sphere sphere(Vec3(0, 0, 0), 1.0f, Vec3(255, 0, 0)); // Red sphere
    LightSource light(Vec3(-5, 5, -5)); // Light position

    for (int y = height - 1; y >= 0; --y) 
    {
        for (int x = 0; x < width; ++x) 
        {
            float u = (x / (float)width) * 2 - 1;
            float v = (y / (float)height) * 2 - 1;

            Ray ray(camera, Vec3(u, v, 0) - camera);
            Vec3 color = trace(ray, light, sphere);

            int r = std::min(255, (int)color.x);
            int g = std::min(255, (int)color.y);
            int b = std::min(255, (int)color.z);

            image << r << " " << g << " " << b << "\n";
        }
    }

    image.close();
}

int main() 
{
    render(400, 400);
    std::cout << "Rendered output.ppm\n";
    return 0;
}
