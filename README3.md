Sky: Benjamin Sei
First I changed the Quad definition so it's z coord was further away.

I began by making a sky shader with the vert shader simply setting gl position and the frag shader doing all the heavy lifting. The foundation of the sky is based on raycasting using the vector defined by the player's eye (really the camera) to the projection of a pixel to a plane in the viewing frustum. In the frag shader we know the position of the fragment in pixel space, so we simply need to apply a series of transformations to get the fragment to world space. However, because the goal is to simulate a sky, we want to project the fragment some distance away, this distance defined by the position of an imaginary plane. This works because each slice in the frustum will follow geometric rules of similarity, and a ray drawn from the eye through the screen and into any other slice in the frustum will hit the same fragment. With the position of the projected fragment in world space in hand, we find the direction of the ray by computing a normalized difference between it and the coordinates of the player's eye. To model the sky, I have chosen to convert the ray to spherical coordinates, with phi defined as rotation around the y-axis and theta as the second degree of freedom. Using hard coded values for sunset and dusk colors as well as piecewise functions to return blended/unblended colors. Now onto the sun. The shader is given the direction of the sun. To get the angle between the sun and the player's view, we take the dot product of the ray and the vector of light, then take the arccos of the dot product to get the angle which occupies [0,pi]. We get it to occupy [0,two_pi] and choose to convert to degrees. Since we have the size of the sun defined already by degrees, we simply check if we are within the sun. If we are, we can do one of two things. If we are within the defined size of the sun's core, then we solidly color the sun's color. Othwerise, color will be a linear interpolation of the sunset and sun. If we are not within the sun, then there are three scenarios. If we should draw sunset, then the final color will be sunset. We could also be within the area transitioning from sunset to dusk in which case we linearly interpolate with dusk color. The final case is when we're completely in dusk in which case we draw dusk.

To animate/perturb the sky, I chose to experiment with simplex noise and varying FBM technqiues. One was provided by the sample, the other was found through researching the topic.
Various code samples for noise functions including simplex was found here: https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
An article discussing FBM as well as interesting warping was found here: https://thebookofshaders.com/13/
Further reading on noise was done in Inigo Quilez's website including this one: http://www.iquilezles.org/www/articles/warp/warp.htm

Fog:
https://webglfundamentals.org/webgl/lessons/webgl-fog.html

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