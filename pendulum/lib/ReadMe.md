# Guideline for Libraries Installation
## Content 

This file contains instructions to setup the libraries in order to compile the
rendering part of the pendulum project. 

Instructions have been tested for:

* Windows 7 
  * Code::Blocks (MinGW)
  * Visual Studio 2019
  
The project compilation requires the following libraries:

* SDL2
* SDL2_TTF
* SDL2_IMAGE

## SDL2
1. Download the SDL2 Development libraries corresponding to your IDE following [our guide](https://preesm.github.io/tutos/intro/#sdl2-and-sdl2_ttf)
2. This step differs depending on the used IDE.
   * **For MinGW based IDEs (Codeblocks, Makefile, Eclipse CDT, ...)**
     1. Decompress the dowloaded file in a temporary location. 
     2. In the decompressed file, copy the content of the following directory  
        ```i686-w64-mingw32```  
        into a folder named exactly as follows:  
        ```/<project-path>/lib/SDL-2.0.<xx>/```  
        where <xx> is replaced with your version number).
     3. Copy the content of  
        ```/<project-path>/lib/SDL-2.0.<xx>/include/SDL2/```  
        into its parent directory  
        ```/<project-path>/lib/SDL-2.0.<xx>/include/```
   * **For Visual Studio Users Only**  
     1. Decompress the dowloaded file in
	    ```/<project-path>/lib/SDL-2.0.<xx>/```
	 2. Copy the content of 
	    ```/<project-path>/lib/SDL-2.0.<xx>/lib/x86```  
		into its parent directory  
		```/<project-path>/lib/SDL-2.0.<xx>/lib```  

## SDL2_TTF
1. Download the SDL2 Development libraries corresponding to your IDE following [our guide](https://preesm.github.io/tutos/intro/#sdl2-and-sdl2_ttf)
2. Follow the same procedure as for the SDL2 library. The folder containing the library must be named as follows:  
   ```SDL2_ttf-2.0.<xx>```  
   where `<xx>` is replaced with your version number.

## SDL2_IMAGE
1. Download the SDL2 Image library corresponding to your IDE from [there](https://www.libsdl.org/projects/SDL_image/)
2. Follow the same procedure as for the SDL2 library. The folder containing the library must be named as follows:  
   ```SDL2_image-2.0.<xx>```  
   where `<xx>` is replaced with your version number.