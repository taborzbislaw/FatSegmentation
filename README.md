# FatSegmentation


The procedure for visceral and abdominal fat segmentation is carried out in two steps.

In the first step, we start from the original 2D axial section of the abdominal cavity (e.g. 1004244319.dcm.raw) and find the body segmentation using the  code Step1_maskCTImage.cpp. 

The code can be compiled into an executable program "Step1_maskCTImage" under linux as follows (Cimg.h header file must be present in the project directory: https://cimg.eu/):

g++ -o Step1_maskCTImage Step1_maskCTImage.cpp -lm -lpthread -lX11 -O3

Launch: ./Step1_maskCTImage 1004244319.dcm.raw

The executable returns a masked file raw mask_1004244319.dcm.raw


In the second step, we apply an algorithm (described in I. Kucybała, Z. Tabor, S. Ciuk, R. Chrzan, A. Urbanik, W. Wojciechowski: A fast graph-based algorithm for automated segmentation of subcutaneous and visceral adipose tissue in 3D abdominal computed tomography images. Biocybernetics and Biomedical Engineering 2020, 40: 729-739) for subcutaneous fat detection to the mask_1004244319.dcm.raw image.

The code for the algorithm can be found in Step2_findFatComponents.cpp. Compilation for linux to an executable "Step2_findFatComponents":
g++ -o Step2_findFatComponents Step2_findFatComponents.cpp -lm -lpthread -lX11 -O3

Launch:
./Step2_findFatComponents mask_1004244319.dcm.raw 100. 1. 0.5

As a result, one gets a skinFat_maska_1004244319.dcm.raw.bmp bitmap with subcutaneous fat, visceral fat, background and the remaining regions. The last three arguments of calling the ./Step2_findFatComponents were independent of the image in the dataset described in the article. 

The raw files can be opened in ImageJ - see Settings.png for options which should be set when opening. To open mask_1004244319.dcm.raw one must switch on the option "Little-endian bit order"





