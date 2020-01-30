#ifndef TEPHRA_TEST_VERTEX_HPP_INCLUDED
#define TEPHRA_TEST_VERTEX_HPP_INCLUDED

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace utils
{

struct vertex
{
    glm::vec2 position{};
    glm::vec2 texture_coord{};
    glm::vec4 color{};
};

struct uniform_buffer_object
{
    glm::mat4 model{};
    glm::mat4 view{};
    glm::mat4 proj{};
};

}

#endif
