Credit to Brian Sharpe's Wombat project for efficient implementation of the base noise functions. (https://github.com/BrianSharpe/Wombat)

We note that the 3D world have the following features:
It uses a hybrid multifractal noise with a Perlin2D basis to create the terrain on the GPU.
We thus get an infinite world.

It use a Fractional Brownian motion noise on a Perlin4D to texture the skybox to add clouds that depend on the time. (animated clouds)

It uses Blinn-Phong lighting and several textures blended together based on height for the colors. We mix a grass and a snow texture to get grass and snow deposits. The deposits depend on the slope at that point

It uses reflection and refraction framebuffers to create reflection and refraction textures for the water surface. 

Waves are added to the water using weighted contributions of sin waves to sample to the water texture at different times creating a water effect.

The reflection texture is sampled after some Perlin2D perturbations to create the effect of moving water


Pictures of 3D Virtual World:

![Picture1](https://github.com/humaid123/3D-Virtual-World/blob/main/capture/Picture1.png)

The mountain in the above pictures show the color blendings that were used fram the san texture, the rock texture and the snow and grass texture deposits.

The following pictures show the water reflection and refraction as well as a smooth/beach-like terrain (Picture2) and a noisy/mountainous terrain (Picture3).

![Picture2](https://github.com/humaid123/3D-Virtual-World/blob/main/capture/Picture2.png)

![Picture3](https://github.com/humaid123/3D-Virtual-World/blob/main/capture/Picture3.png)

Find a video of the 3D Virtual World here to see the water and cloud animations:

https://github.com/humaid123/3D-Virtual-World/blob/main/capture/3D-Virtual-World.mp4
