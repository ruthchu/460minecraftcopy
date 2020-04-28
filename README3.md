Sky: Benjamin Sei
First I changed the Quad definition so it's z coord was further away.

I began by making a sky shader with the vert shader simply setting gl position and the frag shader doing all the heavy lifting. The foundation of the sky is based on raycasting using the vector defined by the player's eye (really the camera) to the projection of a pixel to a plane in the viewing frustum. In the frag shader we know the position of the fragment in pixel space, so we simply need to apply a series of transformations to get the fragment to world space. However, because the goal is to simulate a sky, we want to project the fragment some distance away, this distance defined by the position of an imaginary plane. This works because each slice in the frustum will follow geometric rules of similarity, and a ray drawn from the eye through the screen and into any other slice in the frustum will hit the same fragment. With the position of the projected fragment in world space in hand, we find the direction of the ray by computing a normalized difference between it and the coordinates of the player's eye. To model the sky, I have chosen to convert the ray to spherical coordinates, with phi defined as rotation around the y-axis and theta as the second degree of freedom. Using hard coded values for sunset and dusk colors as well as piecewise functions to return blended/unblended colors. Now onto the sun. The shader is given the direction of the sun. To get the angle between the sun and the player's view, we take the dot product of the ray and the vector of light, then take the arccos of the dot product to get the angle which occupies [0,pi]. We get it to occupy [0,two_pi] and choose to convert to degrees. Since we have the size of the sun defined already by degrees, we simply check if we are within the sun. If we are, we can do one of two things. If we are within the defined size of the sun's core, then we solidly color the sun's color. Othwerise, color will be a linear interpolation of the sunset and sun. If we are not within the sun, then there are three scenarios. If we should draw sunset, then the final color will be sunset. We could also be within the area transitioning from sunset to dusk in which case we linearly interpolate with dusk color. The final case is when we're completely in dusk in which case we draw dusk.

To animate/perturb the sky, I chose to experiment with simplex noise and varying FBM technqiues. One was provided by the sample, the other was found through researching the topic.
Various code samples for noise functions including simplex was found here: https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
An article discussing FBM as well as interesting warping was found here: https://thebookofshaders.com/13/
Further reading on noise was done in Inigo Quilez's website including this one: http://www.iquilezles.org/www/articles/warp/warp.htm

Fog:
I first made a uniform vec3 representing the coords of the camera as well as a uniform mat4 for the view matrix. In the frag shader,
I get the coord of the fragment in camera coordinates in order to obtain the depth value in the form of the z component. I then applied
a simple effect using near and far fog distances as boundaries for a smooth step for linearly interpolating  with the fog color and fog amount.

Biomes: (Ruth Chung)
I added two biomes. One consists of many tall spires, with water appearing beneath a certain level, and the other consists of mostly flat terrain with sharply protruding hills. In the hills biome, the hills are made of dirt and the ground is made of grass.

The spire biome is primarily Perlin noise, floored at a certain threshold. I subtracted floored Worley noise from the top to flatten the spires in a gradiated way. Then, to round the tops again, I subtracted some Perlin noise from the top.
The hill biome is a modified Worley noise, also floored at a certain threshold. 
I also made the mountains slightly shorter.

In order to have smooth height transitions between the 4 biomes (2 existing, 2 new), I used an interpolation function as described here:
https://parzivail.com/procedural-terrain-generaion/
the master function being a Perlin function.
I used that same Perlin function (1D output) to designate terrain zones. Based on terrain zone, I set the blocktype.
For the spire biome, I added 3 new textures to the texture atlas, in order to give the spires a lighter stone color and a darker grass color. This also meant I had to add two new blocks, called SPIRE and SPIRE_TOP.


Sky: (Ruth Chung)
After Ben had set up the shaders, I tweaked the sky fragment shader to make the sky look more natural. Based on the height of the sun, the sky is first shaded with a base day or night color, and if the sun is rising or setting, the base color is an interpolation of the two. If the sun is setting or rising, then I bring in the dusk and the sunset palette. When the sun rises, I first blend the dusk palette into the base sky color, as well as color only the area around the sun with the palette. As the sun rises, the color of the palette grows to fill the whole sky, as well as turns more opaque. Then, I blend that palette into the sunrise palette and do the reverse to get to a fully blue, daytime sky.

I also added in stars during the night. I used 3D Worley noise (provided by Ben) to procedurally add stars to the base night sky color.

Fog: (Ruth Chung)
After Ben had set up the shaders, I tweaked the color of the fog so that it would match the color of the sky better. Rather than pass the lambert frag shader (which draws the fog) the sky color at that point, I decided to bring over the math I used to adjust the sky color, and use that to determine the color of the fog. I also made the fog color slightly transparent so it would blend into the sky better.

Shadow Mapping : (Kaiying Guo)

First I created a post processing pipeline correctly using a frame buffer. I used the frame buffer our professor provided to buffer the scene into a texture occupying a texture slot that was different from the texture slot used for minecraft textures. Then, I displayed the texture using serveral post processing shaders depending on the enviroment the player is on using the quad class provided in homework 4. The shaders took in a passthough vertex shader and did varaious or no modifications to the colors sampled from the screen UV texture. For shadow mapping, I created another class called DepthFrameBuffer which is farily similar to the frame buffer class except in the create function I do not bind a RenderBuffer to the FBO. Instead, I bind a 2d texture image that takes GL_DEPTH_COMPONENT16 as its format instead of color. I set this depthFBO to store no colors. I create a new shader called depthThrough which will be used to render the first pass of the scene from the perspective of light onto the texture which I have bound to the FBO. I create some unifroms for a light projection matrix, the sun direction (which I originally had fixed and later refactored to be in tune with the moving sun), the eye vector and caculate a lightMVP matrix in my depthThrough shader's vertex shader. I pass to the fragment shader lightMVP * vs_Pos which transforms our pixel in world to light spcae. For the fragment shader, I do not do anything since I read from the tutorials that the depth will be automatically saved onto the texture. After doing the first light pass, I set the framebuffer to the deafult frame buffer. In order to do the first pass i had to refactor the render terrain method in myGL to be more generic and take in a Shader program pointer. To test this, I made my post processing shader render the depth map by sampling the texture and taking the r coordinate. 

For the second pass, I bind the regular FBO. I created another sampler2D for the depth texture and sent that to the GPU for the lambert shader to use by making it the active texure and binding it in the a texture slot unsued by the minecraft textures and the post procesing shaders. I call render terrain using the lambert shader as per usual. In the lambert shader fragment shader I created yet again the lightMVP. I created a new out variable called fs_LightPos which will give us the coordinates of a fragment from world space to light space which is computed by lightMVP * u_Model * vs_Pos. Then, in the fragment shader I find the coordinates to smaple in my fragment in lightSpace NDC by doing perspective divide on fs_LightPos. I remap these shadow coordinates to 0,1 in order to sample the depth texture correctly. With the coordinates remapped, I sample the depth txture we made to get the stored depth z. I compare the stored depth to my shadow coordinate's z value. If the z value stored is less than the z coordinate, then the fragment is in shadow and I make it darker in color, efefctively adding shadow.

After that pass is, I render the screen on a quad using my post processing shaders. 

I was stuck on this for so long. I couldn't generate a good depth map because my lightprojection matrix was bad. I had to set the far and near clip plane to match the camera ones and I also increased the size of the viewing box from the one detailed in the tutorials. The tutoirals I used were the following 
https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/

