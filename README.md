# alphaMatting
Intelligent scissors are a tool a cut an object out of an image by using a live wire that automatically wraps around that object. It still requires a user to guide the wire, but does much of the work and might create a better result than a user would. This produces an alpha mask that can then be used to add images together. However, the alpha mask is either one or zero; there is just a hard edge and no mix of alpha values. If the cut is imperfect or if it contains a mixture of foreground and background colors (such as hair), then this hard cut will be noticeable to the viewer. A solution to this is to implement an alpha matting algorithm to solve for the best alpha value for the pixel. I initially tried the Bayesian algorithm to find the alpha value but stopped when I couldn’t find any information on how to compute the Orchard-Bouman clusters that are essential to the algorithm. The algorithm that I settled on was the one described in "A Closed Form Solution to Natural Image Matting”. This algorithm creates a matting laplacian and then solves a sparse linear system in order to compute the alpha values.   
The equation for an image I given the alpha map α the foreground F and background B is   
I = α*F+(1-α)*B  
This is severely under-constrained as I is the only known value. This can be rewritten as  
α = a*I+b  
a = 1/(F-B)  
b=-b/(F-B)  
assuming that F and B are smooth over a window w. The goal is then to find values to minimize the equation
![eqn1](https://raw.githubusercontent.com/nathanbain314/alphaMatting/master/equations/eqn1.jpg)  
with ε as a regularization term.
This can be reduced to J(α) = α^T * L * α, where L(i,j) = 
![eqn2](https://raw.githubusercontent.com/nathanbain314/alphaMatting/master/equations/eqn2.jpg)  
with |w| being the size of the window around k, mean μ, and variance σ^2
Finally this can be extended to an RGB image by setting L(i,j) to   
![eqn3](https://raw.githubusercontent.com/nathanbain314/alphaMatting/master/equations/eqn3.jpg)  
with covariance matrix Σ. 

When solving for alpha, some alpha values are already specified as known foreground or known background. The equation can be modified to become  
![eq1](https://raw.githubusercontent.com/nathanbain314/alphaMatting/master/equations/eq1.png)  
With D being a diagonal matrix where D(i,i) is 1 when the pixel is known foreground or background, vector β is 1 when it is known foreground, and λ being a large number to force the system to find a solution. If this is differentiated it becomes a sparse linear system.  
![eq2](https://raw.githubusercontent.com/nathanbain314/alphaMatting/master/equations/eq2.png)  
This system is solved with the Eigen library for sparse matrices so that α can be found. 
The intelligent scissors executable in the scissors library is the provided solution from [this project](http://courses.cs.washington.edu/courses/cse455/03wi/projects/project1/web/project1.htm). This can be used to trace out the known background and then trace the known foreground. These can then be combined into a trimap or sent directly into the RunLaplacian program. 
###Examples
A picture of a woman taken from natural background and added onto an ocean background.  
![Woman next to Ocean](https://raw.githubusercontent.com/nathanbain314/alphaMatting/master/examples/womanOcean.jpg)  
A squirrel with fuzzy edges rendered onto a different background  
![Squirrel on Enchanted Rock](https://raw.githubusercontent.com/nathanbain314/alphaMatting/master/examples/squirrelRock.jpg)  
The lighthouse on the left is the original cut, and the lighthouse on the right with my alpha mask.  
![Lighthouse on Enchanted Rock](https://raw.githubusercontent.com/nathanbain314/alphaMatting/master/examples/twoTowers.jpg)  

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

##Example
```
./RunLaplacian -i examples/squirrel.jpg -o out.jpg -t examples/squirrelTrimap.png
./RunLaplacian -i examples/squirrel.jpg -o out.jpg -f examples/squirrelForeground.png -b examples/squirrelBackground.png -g
```

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
