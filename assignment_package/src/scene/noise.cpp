#include "noise.h"
#include <iostream>

float Noise::random1(float p) {
    return glm::fract(glm::sin(p * 127.1) * 43758.5453);
}

float Noise::random1(glm::vec2 p) {
    return glm::fract(glm::sin(glm::dot(p, glm::vec2(127.1, 311.7))) * 43758.5453);
}

glm::vec2 Noise::random2(glm::vec2 p) {
    glm::vec2 v = glm::fract(glm::sin(glm::vec2(glm::dot(p, glm::vec2(127.1, 311.7)),
                                 glm::dot(p, glm::vec2(269.5,183.3))))
                                 * 43758.5453f);
    return v;
}

float Noise::surflet(glm::vec2 p, glm::vec2 gridPoint) {
    // Compute the distance between p and the grid point along each axis, and warp it with a
    // quintic function so we can smooth our cells
    glm::vec2 t2 = glm::abs(p - gridPoint);
    glm::vec2 t = glm::vec2(1.f)
            - 6.f * glm::vec2(pow(t2.x, 5.f), pow(t2.y, 5.f))
            + 15.f * glm::vec2(pow(t2.x, 4.f), pow(t2.y, 4.f))
            - 10.f * glm::vec2(pow(t2.x, 3.f), pow(t2.y, 3.f));
    // Get the random vector for the grid point (assume we wrote a function random2
    // that returns a glm::vec2 in the range [0, 1])
    glm::vec2 gradient = random2(gridPoint) * 2.f - glm::vec2(1,1);
    // Get the vector from the grid point to P
    glm::vec2 diff = p - gridPoint;
    // Get the value of our height field by dotting grid->P with our gradient
    float height = glm::dot(diff, gradient);
    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * t.x * t.y;
}

float Noise::perlinNoise(glm::vec2 uv) {
//    std::cout<< uv.x << ", " << uv.y << std::endl;
    glm::vec2 uvXLYL = glm::vec2(floor(uv.x), floor(uv.y));
    glm::vec2 uvXHYL = uvXLYL + glm::vec2(1, 0);
    glm::vec2 uvXHYH = uvXLYL + glm::vec2(1, 1);
    glm::vec2 uvXLYH = uvXLYL + glm::vec2(0, 1);

    float surflet1 = surflet(uv, uvXLYL);
    float surflet2 = surflet(uv, uvXHYL);
    float surflet3 = surflet(uv, uvXHYH);
    float surflet4 = surflet(uv, uvXLYH);
    float surfletSum = surflet1 + surflet2 + surflet3 + surflet4;
//    std::cout << "perlin: " << surfletSum << std::endl;
    return surfletSum;
}


float Noise::worleyNoise(glm::vec2 uv) {
    uv *= 1.5; // Now the space is 10x10 instead of 1x1. Change this to any number you want.
    uv += fbm(uv / 4.f) * 0.25; // warps mounds
    glm::vec2 uvInt = glm::floor(uv);
    glm::vec2 uvFract = glm::fract(uv);
    float minDist = 1.0; // Minimum distance initialized to max.
    float minDistSecond = 1.0;
    for(int y = -1; y <= 1; ++y) {
        for(int x = -1; x <= 1; ++x) {
            glm::vec2 neighbor = glm::vec2(float(x), float(y)); // Direction in which neighbor cell lies
            glm::vec2 point = random2(uvInt + neighbor); // Get the Voronoi centerpoint for the neighboring cell
            glm::vec2 diff = neighbor + point - uvFract; // Distance between fragment coord and neighbor’s Voronoi point
            float dist = glm::length(diff);
            dist = dist * dist;
//            minDist = min(minDist, dist);
            if (dist < minDist) {
                minDistSecond = minDist;
                minDist = dist;
            } else if (dist < minDistSecond) {
                minDistSecond = dist;
            }
        }
    }
//    return minDist;
    float c1 = -1.f;
    float c2 = 1.f;
    float height = c1 * minDist + c2 * minDistSecond;
    float spread = 0.175f;
    height = glm::max(0.f, height - spread) / (1 - spread);
    float scalar = 0.75;
    float offset = (1.f - scalar) * fbm(uv); // adds noise in between mounds
    return height * scalar + offset;
}

float Noise::mySmoothStep(float a, float b, float t) {
    t = glm::smoothstep(0.f, 1.f, t);
    return glm::mix(a, b, t);
}

float Noise::bilerpNoise(glm::vec2 uv) {
    glm::vec2 uvFract = glm::fract(uv);
    float ll = random1(glm::floor(uv));
    float lr = random1(glm::floor(uv) + glm::vec2(1, 0));
    float ul = random1(glm::floor(uv) + glm::vec2(0, 1));
    float ur = random1(glm::floor(uv) + glm::vec2(1, 1));

    float lerpXL = mySmoothStep(ll, lr, uvFract.x);
    float lerpXU = mySmoothStep(ul, ur, uvFract.x);

    return mySmoothStep(lerpXL, lerpXU, uvFract.y);
}

float Noise::fbm(glm::vec2 uv) {
    float amp = 0.5;
    float freq = 8.0;
    float sum = 0.f;
    for (int i = 0; i < 2; i++) {
        sum += bilerpNoise(uv * freq) * amp;
        amp *= 0.5;
        freq *= 2.0;
    }
    return sum;
}
