# SSIMULACRA 2 - Structural SIMilarity Unveiling Local And Compression Related Artifacts

Perceptual metric developed by Jon Sneyers (Cloudinary) in July-October 2022, updated in April 2023.

## Usage
```
ssimulacra2 original.png distorted.png
```

Returns a score in range -inf..100, which correlates to subjective visual quality scores as follows:

- negative scores: extremely low quality, very strong distortion.
- 10 = very low quality.
  MOS score close to 1 ('bad') in a BT.500 ACR test. Very obvious and annoying artifacts.
  This corresponds to the typical output of `cjxl -d 14` / `-q 12` or libjpeg-turbo 4:2:0 quality 14.
- 30 = low quality.
  MOS score around 2 ('poor') in a BT.500 ACR test. Obvious and annoying artifacts.
  This corresponds to the typical output of `cjxl -d 9` / `-q 20` or libjpeg-turbo 4:2:0 quality 20.
- 50 = medium quality.
  MOS score around 3 ('fair') in a BT.500 ACR test. Slightly annoying artifacts.
  This corresponds to the typical output of `cjxl -d 5` / `-q 45` or libjpeg-turbo 4:2:0 quality 35.
- 70 = high quality.
  MOS score around 4 ('good') in a BT.500 ACR test. Artifacts are perceptible, but not annoying.
  Distortion barely noticeable by an average observer in a side-by-side comparison at 1:1 from a normal viewing distance.
  Without reference to the original image, an average observer does not notice artifacts.
  This corresponds to the typical output of `cjxl -d 2.5` / `-q 73` or libjpeg-turbo 4:2:0 quality 70.
- 80 = very high quality.
  Distortion not noticeable by an average observer in a side-by-side comparison at 1:1 from a normal viewing distance.
  This corresponds to the typical output of `cjxl -d 1.5` / `-q 85` or libjpeg-turbo 4:2:2 quality 85.
- 85 = excellent quality. 
  Distortion not noticeable by an average observer in the condition of in-place comparison at 1:1 from a normal viewing distance.
  This corresponds to the typical output of `cjxl -d 1` / `-q 90` or libjpeg-turbo 4:4:4 quality 90.
- 90 = visually lossless.
  Distortion not noticeable by an average observer in a flicker test at 1:1 from a normal viewing distance.
  This corresponds to the typical output of `cjxl -d 0.5` / `-q 95` or libjpeg-turbo 4:4:4 quality 95.
- 100 = mathematically lossless.



## How it works

SSIMULACRA 2 is based on the concept of the multi-scale structural similarity index measure (MS-SSIM),
computed in a perceptually relevant color space, adding two other (asymmetric) error maps, and
aggregating using two different norms.

- XYB color space (rescaled to a 0..1 range and with B-Y)
- Three error maps:
  - SSIM map (with a corrected SSIM formula that avoids applying gamma correction twice)
  - 'blockiness/ringing' map (error means distorted has edges where original is smooth)
  - 'smoothing/blur' map (error means distorted is smooth where original has edges)
- Each of these error maps is computed at 6 scales (1:1 to 1:32) for each component (X,Y,B)
- Downscaling is done in linear color (i.e. the perceptually correct way)
- For each of these `6*3*3=54` maps, two norms are computed: 1-norm (mean) and 4-norm
- A weighted sum of these `54*2=108` norms leads to the final score
- Weights were tuned based on a large set of subjective scores (CID22, TID2013, Kadid10k, KonFiG-IQA),
  including images compressed with JPEG, JPEG 2000, JPEG XL, WebP, AVIF, HEIC, and various artificial distortions.


Final results after tuning:

SSIMULACRA 2.1:

Dataset | KRCC | SRCC | PCC |
-- | -- | -- | --
CID22|     0.6903 | 0.8805 | 0.8583
TID2013|   0.6590 | 0.8445 | 0.8471
KADID-10k| 0.6175 | 0.8133 | 0.8030
KonFiG(F)| 0.7668 | 0.9194 | 0.9136

SSIMULACRA 2.0:

Dataset | KRCC | SRCC | PCC |
-- | -- | -- | --
CID22|      0.6934 | 0.8820 | 0.8601
TID2013|    0.6322 | 0.8194 | 0.8103
KADID-10k|  0.5870 | 0.7851 | 0.7018
KonFiG(F)|  0.7813 | 0.9280 | 0.8710



The weight tuning was done by running Nelder-Mead simplex search, optimizing to minimize MSE and to
maximize Kendall and Pearson correlation for training data consisting of the CID22 training data,
TID2013, KADID-10k and KonFiG (F boosting).

Changes compared to SSIMULACRA 2.0:

- weights retuned to correlate better with other datasets
- changed the range of the 3 components to ensure they are in 0..1 so the SSIM formula makes sense
- added a polynomial remapping of the error score to allow a better fit to datasets with higher distortions


Changes compared to the [original version (SSIMULACRA 1)](https://github.com/cloudinary/ssimulacra):

- works in XYB color space instead of CIE Lab
- linear downscaling
- fixed SSIM formula
- uses 1-norm and 4-norm (instead of 1-norm and max-norm-after-downscaling)
- penalizes both smoothing and ringing artifacts (instead of only penalizing ringing but not smoothing)
- removed specific grid-like blockiness detection
- tuned using a much larger set of subjective opinions (and using absolute quality scores, not just relative comparison results)




## Metric performance


These tables show the correlation of various metrics in terms of
Kendall Rank Correlation Coefficient (KRCC),
Spearman Rank Correlation Coefficient (SRCC),
and Pearson Correlation Coefficient (PCC), for various IQA datasets.

TID2013:

Metric | KRCC | SRCC | PCC |
-- | -- | -- | --
PSNR-Y | 0.4699 | 0.6394 | 0.428
PSNR-HVS | 0.5464 | 0.698 | 0.6846
SSIM | 0.5707 | 0.7552 | 0.764
MS-SSIM | 0.6068 | 0.7868 | 0.7802
VMAF | 0.5608 | 0.7439 | 0.7728
SSIMULACRA 2 | 0.6322 | 0.8194 | 0.8103
SSIMULACRA 2.1 | 0.659 | 0.8445 | 0.8471
DSSIM | -0.6984 | -0.871 | -0.8021
Butteraugli (3-norm) | -0.4935 | -0.6639 | -0.4878
PSNR (ImageMagick) | 0.4958 | 0.6869 | 0.6601

KADID-10k:

Metric | KRCC | SRCC | PCC |
-- | -- | -- | --
PSNR-Y | 0.4555 | 0.6319 | 0.5932
PSNR-HVS | 0.4229 | 0.5927 | 0.5949
SSIM | 0.5889 | 0.7806 | 0.6576
MS-SSIM | 0.6466 | 0.8359 | 0.6836
VMAF | 0.5343 | 0.7253 | 0.7185
SSIMULACRA 2 | 0.587 | 0.7851 | 0.7018
SSIMULACRA 2.1 | 0.6175 | 0.8133 | 0.803
DSSIM | -0.6679 | -0.8561 | -0.6544
Butteraugli (3-norm) | -0.3846 | -0.543 | -0.4424
PSNR (ImageMagick) | 0.4876 | 0.6757 | 0.6214


KonFiG-IQA: (Experiment I, F boosting, clamping negative JND (better than reference) to zero)

Metric | KRCC | SRCC | PCC |
-- | -- | -- | --
PSNR-Y | 0.5871 | 0.7598 | 0.6968 |
PSNR-HVS | 0.7798 | 0.9277 | 0.8453 |
SSIM | 0.6156 | 0.7795 | 0.7052 |
MS-SSIM | 0.6635 | 0.8299 | 0.6834 |
VMAF | 0.3866 | 0.4906 | 0.463 |
SSIMULACRA 2 | 0.7813 | 0.928 | 0.871 |
SSIMULACRA 2.1 | 0.7668 | 0.9194 | 0.9136 |
DSSIM | -0.7595 | -0.9147 | -0.673 |
Butteraugli (3-norm) | -0.771 | -0.9238 | -0.7587 |
PSNR (ImageMagick) | 0.6531 | 0.8248 | 0.7218 |


CID22 full set: (22k subjective scores)

Metric | KRCC | SRCC | PCC |
-- | -- | -- | --
PSNR-Y | 0.4452 | 0.6246 | 0.5901
PSNR-HVS | 0.6076 | 0.81 | 0.7559
SSIM | 0.5628 | 0.7577 | 0.7005
MS-SSIM | 0.5596 | 0.7551 | 0.7035
VMAF | 0.6176 | 0.8163 | 0.7799
SSIMULACRA 2 | 0.6934 | 0.882 | 0.8601
SSIMULACRA 2.1 | 0.6903 | 0.8805 | 0.8583
DSSIM | -0.6428 | -0.8399 | -0.7813
Butteraugli 3-norm | -0.6547 | -0.8387 | -0.7903
PSNR (ImageMagick) | 0.3472 | 0.5002 | 0.4817


CID22 validation set: (4292 subjective scores, not used for tuning)

Metric | KRCC | SRCC | PCC |
-- | -- | -- | --
PSNR-Y | 0.4734 | 0.6577 | 0.6354
PSNR-HVS | 0.6199 | 0.8224 | 0.7848
SSIM | 0.6028 | 0.7871 | 0.7647
MS-SSIM | 0.5915 | 0.7781 | 0.7601
VMAF | 0.588 | 0.7884 | 0.7502
SSIMULACRA 2 | 0.7033 | 0.8854 | 0.8745
SSIMULACRA 2.1 | 0.7077 | 0.8904 | 0.8787
DSSIM | -0.6807 | -0.8722 | -0.822
Butteraugli 3-norm | -0.6102 | -0.7938 | -0.745
PSNR (ImageMagick) | 0.3491 | 0.4995 | 0.5013



## Building

Building instructions for Debian:
```
sudo apt install build-essential git libhwy-dev liblcms2-dev libjpeg62-turbo-dev libpng-dev cmake ninja-build
mkdir build
cd build
cmake ../src -G Ninja
ninja ssimulacra2
```

or simply execute `build_ssimulacra2`. Other distributions should be similar;
you may need to use `libjpeg-turbo8-dev` instead of `libjpeg62-turbo-dev`.
Version 2.13 of lcms2 is needed.

The source code of SSIMULACRA 2 is also part of the `tools` of [libjxl](https://github.com/libjxl/libjxl/blob/main/tools/ssimulacra2.cc).

The bash script `build_ssimulacra2_from_libjxl_repo` can be used to fetch the code and compile only what is needed for SSIMULACRA 2.


