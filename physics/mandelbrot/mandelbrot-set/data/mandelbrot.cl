/**
 * mandelbrot kernel source
 */
__kernel void mandelbrot(
    __write_only image2d_t image,
    const float2 xrange,
    const float2 yrange,
    const long width,
    const long height,
    const long maxiters)
{
    const long ix = get_global_id(0);    // global pos in x-direction
    const long iy = get_global_id(1);    // global pos in y-direction

    /* Bound checking */
    if (!(ix < width && iy < height)) {
        return;
    }

    /* Compute scaled coordinates of the pixel value */
    float u = (float) ix / width;
    float v = (float) iy / height;

    float x0 = xrange.s0 + u*(xrange.s1 - xrange.s0);
    float y0 = yrange.s0 + v*(yrange.s1 - yrange.s0);

    float x = 0.0;
    float y = 0.0;
    long iter = 0;
    while (x*x + y*y < 4 && iter++ < maxiters) {
        float tmp = x*x - y*y + x0;
        y = 2*x*y + y0;
        x = tmp;
    }

    /* Write color to texture */
    float c = (float) iter / maxiters;
    float4 color = (float4) (c, c, c, 1.0f);
    write_imagef(image, (int2) (ix, iy), color);
}
