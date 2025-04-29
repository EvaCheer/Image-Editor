# Image-Editor
A simple image editor built with C++ and OpenCV, 
featuring manual Gaussian blur, brightness/contrast control, and image sharpening.

![Lenna_edited](https://github.com/user-attachments/assets/5ad13598-b22a-4d84-968b-6d4cfd563003)

### Brightness and contrast adjustments
To adjust brigtness and contrast we use the linear expression:

$$ g(x) = \alpha f(x) + \beta, \alpha > 0 $$

where the parameters α and β control the contrast and brightness respectively.
We apply this formula to each channel of each pixel of an image (a matrix with dimensions i and j). We can also write the formula like:

$$ g(i,j) = \alpha f(i,j) + \beta $$


### Gaussian Blur
To create the effect of a Gaussian Blur first need to create a kernel/convolution matrix. After that we need to convolute our image (add each element of the image to its local neighbors, weighted by the kernel.)

#### The kernel/Convolution Matrix
To generate a kernel we fill in a matrix with the values received from using the formula:

$$ g(x,y) = \exp{- \frac{x^2 + y^2}{2 \sigma ^2}} $$

where x is the distance from the origin in the horizontal axis, y is the distance from the origin in the vertical axis, and σ is the standard deviation of the Gaussian distribution. 
After filling in the convolution matrix, we need to normalize it so that the sum of all weights equals 1. This preserves the brightness of the image after applying the blur.


#### Convolution

![2D_Convolution_Animation](https://github.com/user-attachments/assets/b311e983-d16d-4e2e-a44d-2d08d33bb919) <br/>
<sub>By Michael Plotke - Own work, CC BY-SA 3.0, https://commons.wikimedia.org/w/index.php?curid=24288958  </sub>


To do the convolution we must iterate through every pixel, then for each pixel, we iterate through the whole kernel to make the calculations needed for the output pixel’s value. 

A challenge in the implementation is the way we traverse the image’s neighbouring pixels, while keeping track of the relevant kernel’s indices. We solve this by keeping track of the offset (ky,kx) of the current multiplication taking place at (y+ky,x+kx), to the current image pixel (y,x) that is being convoluted.

There are cases where the convolution matrix goes out of the image’s edges. To take care of that, we clamp the values of y+ky and x+ky and take a pixel in the image with the clamped values for indices (lets say the new clamped indices are (yy,xx)). Now if we go out of the image’s range, we take the values of a pixel on the edge of the image. Throughout this, the indices of the kernel’s current weight stay the same.

At this time, we are ready to take the image’s pixel at (yy, xx) and the kernel’s weight at (ky + half_k, kx + half_k) (ky and kx were offset counters from –half_k to half_k, so to get the current kernel’s indices that go from 0 to 2*half_k=kernel_size, we need to add +half_k).

Now that we have both the pixel and the kernel’s weight, we simply multiply each channel of the pixel (in our case, three channels) by the weight and stuff each multiplication into a sum[i], i=0,1,2.

After doing this for each value in the kernel, while we keep adding the sums at the same place, we are left with a sum that holds our output pixel values. We can now fill our output image at (y,x) with the output pixel.
Repeat for each pixel.


### Sharpness
The formula used for creating a shrening effect is:

$$ \text{sharpened} = \text{origial} + (\text{original} - \text{blurred}) \text{amount}  $$ 

,or

$$ \text{sharpened} = \alpha * \text{original} + \beta * \text{blurred} $$

where α = 1 + sharpness_modifier and β = sharpness_modifier
This formula then is applied to each channel of each pixel of the image.




