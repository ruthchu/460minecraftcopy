#ifndef NOISE_H
#define NOISE_H
#include <glm/glm.hpp>
class Noise {
private:
    static float surflet(glm::vec2 p, glm::vec2 gridPoint);
    static float bilerpNoise(glm::vec2 uv);
    static float mySmoothStep(float a, float b, float t);
public:
    // (0, 1]
    static float random1(float p);
    static float random1(glm::vec2 p);
    static glm::vec2 random2(glm::vec2 p);
    static float perlinNoise(glm::vec2 uv);
    static float worleyNoise(glm::vec2 uv);
    static float fbm(glm::vec2 uv);
};


#endif // NOISE_H
