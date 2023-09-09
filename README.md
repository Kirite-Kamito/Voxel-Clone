# GraphicsFinalProject

ReadMe for Voxel Engine
*******************************************
Originally build using Microsoft Visual Studio; sln file can be found at the github:

This is a CLONE of the original project which was created as a Final Project. 

This is unlikely to be updated outside of any severe bugs found.
*******************************************
Since this project uses Vulkan, it’s required for it to run. A guide to it’s installation and dependencies can be found here:
https://vulkan-tutorial.com/Development_environment

External libraries required to be linked are: vulkan-1.lib; glfw3.lib; glfw3_mt.lib; glfw3dll.lib

*******************************************
Controls:

WASD - Move directionally\
Space - Move up\
Left Shift - Move down\
E - Make a voxel (attaches to closest voxel to camera)\
Q - Remove a voxel (removes closest voxel to camera)\
Arrow Keys - Look\
F1 - Change voxel color (uses console to get user input)\
Left Control - Reset Camera

********************************************
Part 1: Environment
Much of the environmental set up follows Brendan Galea’s tutorial on Vulkan:
https://www.youtube.com/@BrendanGalea/videos \
Which includes lighting, controls, setting up buffers, pipelines, etc. This was followed and then modified to make it more catered to a vulkan environment.

Pretty confident with everything aside from Make/Remove Voxel in first_app.cpp. Trying to implement inserting/deletion based on what voxel the camera was looking at failed and ultimately functioned worse than the current implementation of it.

Part 2: Culling
Implemented frustum culling. Compares the center of the voxel to the position relative to the camera and decides whether or not to cull. Not terribly optimized, but does the math on one point rather than all 8 vertices. Implementation can be found in first_app.cpp under the culling function.

Part 3: Model Importing
This part was completed alongside the YouTube tutorials, but attempted to use:
https://github.com/karimnaaji/voxelizer \
To attempt to have imported objects probably be immersed in the voxel environment. This failed as I ran out of time to bug fix (Vulkan was a lot more than I thought), but still included as an import option when I is pressed as a relic.
