# alphaMatting
Alpha matting to use with intelligent scissors

##Building
[Libvips](http://www.vips.ecs.soton.ac.uk/index.php?title=Libvips) and [glm](http://glm.g-truc.net/0.9.8/index.html) are required in order for this to build. Since Eigen and clap are implemented in header files they should work fine.
```
mkdir build
make all
```

#RunLaplacian
This takes in either a trimap or a pair of foreground and background maps and creates an alpha mask. Foreground and background maps can be created with the executable in the scissors directory. These can then either be manually assembled into a trimap, or they can be passed directly into the program.

##Options
- -i,  --input : Input image (required)
- -o, --output : Output image (required)
- -t, --trimap : Trimap
- -f, --foreground : Foreground map
- -b, --background : Background map
- -e, --epsilon : Epsilon value
- -g, --generate : Use foregound and background maps instead of trimap

##Tips
Either a trimap or a pair of foreground and background maps are required. Images are loaded with vipsload which takes in most image types, but it won't accept .tga files, which is the output format of the scissors tool, so these will have to be converted with ```convert```, gimp, or some other tool. The default epsilon is the suggested value of 1e-7. This code was built on a mac and will work on one. The scissors code is from the [this scissors solution](http://courses.cs.washington.edu/courses/cse455/03wi/projects/project1/web/project1.htm) and is a windows executable. It can however be run with [Wine](https://www.winehq.org) for use on a mac.

#Examples
![Woman next to Ocean](https://raw.githubusercontent.com/nathanbain314/alphaMatting/master/examples/womanOcean.jpg)
![Squirrel on Enchanted Rock](https://raw.githubusercontent.com/nathanbain314/alphaMatting/master/examples/squirrelRock.jpg)
![Lighthouse on Enchanted Rock](https://raw.githubusercontent.com/nathanbain314/alphaMatting/master/examples/lighthouseRock.jpg)
#Resources used
##Theory
[A Closed Form Solution to Natural Image Matting](http://www.wisdom.weizmann.ac.il/~levina/papers/Matting-Levin-Lischinski-Weiss-CVPR06.pdf)
[Fast Matting Using Large Kernel Matting Laplacian Matrices](http://kaiminghe.com/publications/cvpr10matting.pdf)
##Code
[Libvips](http://www.vips.ecs.soton.ac.uk/index.php?title=Libvips)
[Eigen](http://eigen.tuxfamily.org/index.php?title=Main_Page)
[glm](http://glm.g-truc.net/0.9.8/index.html)
[tclap](http://tclap.sourceforge.net)
[Scissors](http://courses.cs.washington.edu/courses/cse455/03wi/projects/project1/web/project1.htm)
