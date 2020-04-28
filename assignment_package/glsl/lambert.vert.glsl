#version 150
// ^ Change this to version 130 if you have compatibility issues

//This is a vertex shader. While it is called a "shader" due to outdated conventions, this file
//is used to apply matrix transformations to the arrays of vertex data passed to it.
//Since this code is run on your GPU, each vertex is transformed simultaneously.
//If it were run on your CPU, each vertex would have to be processed in a FOR loop, one at a time.
//This simultaneous transformation allows your program to run much faster, especially when rendering
//geometry with millions of vertices.

uniform mat4 u_Model;       // The matrix that defines the transformation of the
                            // object we're rendering. In this assignment,
                            // this will be the result of traversing your scene graph.

uniform mat4 u_ModelInvTr;  // The inverse transpose of the model matrix.
                            // This allows us to transform the object's normals properly
                            // if the object has been non-uniformly scaled.

uniform mat4 u_ViewProj;    // The matrix that defines the camera's transformation.
                            // We've written a static matrix for you to use for HW2,
                            // but in HW3 you'll have to generate one yourself
uniform mat4 u_depthMVP;

uniform mat4 u_View;

uniform ivec2 u_Dimensions; // screen u_Dimensions

uniform mat4 u_LightProj;

uniform vec3 u_Eye; // Camera pos

uniform int u_Time;

uniform vec4 u_Color;       // When drawing the cube instance, we'll set our uniform color to represent different block types.

in vec4 vs_Pos;             // The array of vertex positions passed to the shader

in vec4 vs_Nor;             // The array of vertex normals passed to the shader

in vec4 vs_Col;             // The array of vertex colors passed to the shader.

out vec4 fs_Pos;
out vec4 fs_Nor;            // The array of normals that has been transformed by u_ModelInvTr. This is implicitly passed to the fragment shader.
out vec4 fs_LightVec;       // The direction in which our virtual light lies, relative to each vertex. This is implicitly passed to the fragment shader.
out vec4 fs_UV;            // The color of each vertex. This is implicitly passed to the fragment shader.

out vec4 fs_PosLight;

const vec4 lightDir = normalize(vec4(0.5, 1, 0.75, 0));  // The direction of our virtual light, which is used to compute the shading of
                                        // the geometry in the fragment shader.

const vec3 sunDir = normalize(vec3(0, 0.1, 1.0));

mat4 lookAt(vec3 eye, vec3 center, vec3 up)
{
    vec3  f = normalize(center - eye);
    vec3  u = normalize(up);
    vec3  s = normalize(cross(f, u));
    u = cross(s, f);
    mat4 result = mat4(1.0);
    result[0][0] = s.x;
    result[1][0] = s.y;
    result[2][0] = s.z;
    result[0][1] = u.x;
    result[1][1] = u.y;
    result[2][1] = u.z;
    result[0][2] =-f.x;
    result[1][2] =-f.y;
    result[2][2] =-f.z;
    result[3][0] =-dot(s, eye);
    result[3][1] =-dot(u, eye);
    result[3][2] = dot(f, eye);
    return result;
}

vec3 rotateX(vec3 p, float a) {
    mat4 rot = mat4(1, 0, 0, 0,
                    0, cos(a), sin(a), 0,
                    0, -sin(a), cos(a), 0,
                    0, 0, 0, 1);
    vec4 v = rot * vec4(p, 1);
    return v.xyz;
}

void main()
{
    fs_Pos = vs_Pos;
    fs_UV = vs_Col;                         // Pass the vertex colors to the fragment shader for interpolation

    mat3 invTranspose = mat3(u_ModelInvTr);
    fs_Nor = vec4(invTranspose * vec3(vs_Nor), 0);          // Pass the vertex normals to the fragment shader for interpolation.
                                                            // Transform the geometry's normals by the inverse transpose of the
                                                            // model matrix. This is necessary to ensure the normals remain
                                                            // perpendicular to the surface after the surface is transformed by
                                                            // the model matrix.

    vec4 modelposition = u_Model * vs_Pos;   // Temporarily store the transformed vertex positions for use below

    fs_LightVec = (lightDir);  // Compute the direction in which the light source lies

    // frag world pos -> unhomogenized screen pos -> light pos
//    fs_PosLight =  u_depthMVP * u_View * modelposition;


//    glm::vec3 lightPos = glm::vec3(40.f, 180.f, -20.f);
//    glm::vec3 lightDir = glm::normalize(glm::vec3(0.5f, -0.6f, 0.75f));
//    glm::mat4 lightView = glm::lookAt(lightPos, lightDir, glm::vec3(0, 1, 0));
//    mat4 lightMVP = mat4();

    vec3 newSunDir = rotateX(sunDir, u_Time * 0.01);
    newSunDir = normalize(newSunDir);
    vec3 lightPos = -1000 * newSunDir + u_Eye;
    mat4 lightMVP = u_LightProj * lookAt(lightPos, newSunDir, vec3(0, 1, 0));

    fs_PosLight =  lightMVP * modelposition;
//    fs_PosLight =  u_depthMVP * modelposition;

    //((gl_FragCoord.xy / vec2(u_Dimensions)) - 0.5) * 2.0;

//    fs_PosLight =  u_depthMVP * u_ViewProj * vec2(gl_FragCoord.x );

    gl_Position = u_ViewProj * modelposition;// gl_Position is a built-in variable of OpenGL which is
                                             // used to render the final positions of the geometry's vertices
}
