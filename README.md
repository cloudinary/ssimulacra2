# SSIMULACRA 2 - Structural SIMilarity Unveiling Local And Compression Related Artifacts

Perceptual metric developed by Jon Sneyers (Cloudinary) in July-August 2022.

## Usage
```
ssimulacra2 original.png distorted.png
```

Returns a score in range -inf..100, which correlates to subjective visual quality scores as follows:

- 30 = low quality. This corresponds to the p10 worst output of mozjpeg -quality 30.
- 50 = medium quality. This corresponds to the average output of cjxl -q 40 or mozjpeg -quality 40, or the p10 output of cjxl -q 50 or mozjpeg -quality 60.
- 70 = high quality. This corresponds to the average output of cjxl -q 65 or mozjpeg -quality 70, p10 output of cjxl -q 75 or mozjpeg -quality 80.
- 90 = very high quality. Likely impossible to distinguish from the original when viewed at 1:1 from a normal viewing distance. This corresponds to the average output of mozjpeg -quality 95 or the p10 output of cjxl -q 95.



## How it works

SSIMULACRA 2 is based on the concept of the multi-scale structural similarity index measure (MS-SSIM),
computed in a perceptually relevant color space, adding two other (asymmetric) error maps, and
aggregating using two different norms.

- XYB color space (X+0.5, Y, Y-B+1.0)
- Three error maps:
  - SSIM map
  - 'blockiness/ringing' map (error means distorted has edges where original is smooth)
  - 'smoothing/blur' map (error means distorted is smooth where original has edges)
- Each of these error maps is computed at 6 scales (1:1 to 1:32) for each component (X,Y,B)
- For each of these `6*3*3=54` maps, two norms are computed: 1-norm (mean) and 4-norm
- A weighted sum of these `54*2=108` norms leads to the final score
- Weights were tuned based on a large set of subjective scores for images compressed
  with JPEG, JPEG 2000, JPEG XL, WebP, AVIF, and HEIC.

The weight tuning was done by running Nelder-Mead simplex search, optimizing to minimize MSE and to
maximize Kendall and Pearson correlation for training data consisting of 17611 subjective quality scores,
validated on separate validation data consisting of 4292 scores.


Changes compared to the [original version](https://github.com/cloudinary/ssimulacra):

- works in XYB color space instead of CIE Lab
- uses 1-norm and 4-norm (instead of 1-norm and max-norm-after-downscaling)
- penalizes both smoothing and ringing artifacts (instead of only penalizing ringing but not smoothing)
- removed specific grid-like blockiness detection
- tuned using a much larger set of subjective opinions (and using absolute quality scores, not just relative comparison results)




## Metric performance

Results for just the validation set (4292 subjective scores):

Metric | Kendall correlation | Spearman | Pearson |
-- | -- | -- | --
PSNR | 0.35089 | 0.50092 | 0.50157
SSIM | 0.45002 | 0.62777 | 0.55710
VMAF | 0.58799 | 0.78798 | 0.75031
DSSIM | -0.6777 | -0.8697 | -0.8180
Butteraugli max-norm | -0.5495 | -0.7426 | -0.6870
Butteraugli 2-norm | -0.6226 | -0.8127 | -0.7834
SSIMULACRA | -0.5881 | -0.7845 | -0.7808
SSIMULACRA 2 | 0.72163 | 0.90354 | 0.89504

<img src="metric_correlation-scatterplots-MCOS-validation.svg" width="100%"
alt="2D histograms showing correlation between metrics (PSNR, SSIM, VMAF, DSSIM, Butteraugli (max-norm and 2-norm), SSIMULACRA (v1 and v2) and subjective scores on the validation set (4.3k images from 49 originals)">


Results for the full dataset (almost 22k subjective scores):

Metric | Kendall correlation | Spearman | Pearson |
-- | -- | -- | --
PSNR | 0.34677 | 0.49926 | 0.48178
SSIM | 0.41612 | 0.58939 | 0.52543
VMAF | 0.61720 | 0.81549 | 0.77781
DSSIM | -0.6392 | -0.8363 | -0.7753
Butteraugli max-norm | -0.5852 | -0.7753 | -0.7100
Butteraugli 2-norm | -0.6572 | -0.8461 | -0.8103
SSIMULACRA | -0.5214 | -0.7124 | -0.6891
SSIMULACRA 2 | 0.74185 | 0.91695 | 0.90244

<img src="metric_correlation-scatterplots-MCOS-all.svg" width="100%"
alt="2D histograms showing correlation between metrics (PSNR, SSIM, VMAF, DSSIM, Butteraugli (max-norm and 2-norm), SSIMULACRA (v1 and v2) and subjective scores on the full data (22k images from 250 originals)">



Computing the mean absolute error between opinion scores (on a scale of 0 to 100) and SSIMULACRA 2 results,
on the full set the MAE is 4.274 and on the validation set the MAE is 4.470.


## Building

The source code of SSIMULACRA 2 is part of the `tools` of [libjxl](https://github.com/libjxl/libjxl/blob/main/tools/ssimulacra2.cc).

The bash script `build_ssimulacra2` can be used to fetch the code and compile only what is needed for SSIMULACRA 2.


