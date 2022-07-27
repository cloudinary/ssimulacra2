# SSIMULACRA 2 - Structural SIMilarity Unveiling Local And Compression Related Artifacts

Perceptual metric developed by Jon Sneyers (Cloudinary) in July 2022.
Design:
- XYB color space (X+0.5, Y, Y-B+1.0)
- SSIM map
- 'blockiness/ringing' map (error means distorted has edges where original is smooth)
- 'smoothing' map (error means distorted is smooth where original has edges)
- Each of these error maps is computed at 6 scales (1:1 to 1:32) for each component (X,Y,B)
- For each of these `6*3*3=54` maps, two norms are computed: 2-norm (rmse) and 4-norm
- A weighted sum of these `54*2=108` norms leads to the final score
- Weights were tuned based on a large set of subjective scores for images compressed
  with JPEG, JPEG 2000, JPEG XL, WebP, AVIF, and HEIC.

The weight tuning was done by running Nelder-Mead simplex search, optimizing to minimize MSE and maximize Kendall and Pearson correlation for training data consisting of 17607 subjective quality scores, validated on separate validation data consisting of 4291 scores.

## Metric performance

Results for the full dataset (almost 22k subjective scores):

Metric | Kendall correlation | Spearman | Pearson |
-- | -- | -- |-- 
PSNR | 0.349247  |0.503049 | 0.486496
SSIM | 0.423866  |0.599765 | 0.531878
VMAF | 0.611730  |0.809715 | 0.769437
DSSIM | -0.63776  |-0.83518 | -0.77323
BA-2norm | -0.65846  |-0.84847 | -0.80865
SSIMULACRA | -0.46608  |-0.64622 | -0.64382
SSIMULACRA 2 | 0.730667  |0.908148 | 0.884086

Results for just the validation set (4291 subjective scores):

Metric | Kendall correlation | Spearman | Pearson |
-- | -- | -- |-- 
PSNR | 0.369410  |0.526031 | 0.525934
SSIM | 0.465666  |0.646420 | 0.570981
VMAF | 0.585740  |0.783494 | 0.740504
DSSIM | -0.68036  |-0.86964 | -0.81394
BA-2norm | -0.62880  |-0.82051 | -0.78474
SSIMULACRA | -0.52301  |-0.71024 | -0.72711
SSIMULACRA 2 | 0.729409  |0.905287 | 0.893949


## Building

The source code of SSIMULACRA 2 is part of the `tools` of [libjxl](https://github.com/libjxl/libjxl/blob/main/tools/ssimulacra2.cc).

The bash script `build_ssimulacra2` can be used to fetch the code and compile only what is needed for SSIMULACRA 2.

