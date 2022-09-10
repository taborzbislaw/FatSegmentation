# FatSegmentation


The procedure for visceral and abdominal fat segmentation is carried out in two steps.

In the first step, we start from the original 2D axial section of the abdominal cavity (e.g. 1004244319.dcm.raw) and find the body segmentation using the  code analysis_1.cpp. 
The code can be compiled into an executable program "analiza_1" under linux using the attached script buildAanaliza.sh
Launch: ./analiza_1 1004244319.dcm.raw

The code returns a masked file raw mask_1004244319.dcm.raw

In the second step, we apply an algorithm for subcutaneous fat detection to the mask_1004244319.dcm.raw image, saved in the attached file sciezki_13.11.2018.cpp (compilation for linux to an executable "sciezki" in the buildSciezki.sh script). 
Launch:
./sciezki mask_1004244319.dcm.raw 100. 1. 0.5

As a result, I get a skinFat_maska_1004244319.dcm.raw.bmp bitmap with subcutaneous fat, visceral fat, background and the rest. The last three arguments of calling the ./sciezki program were unchanged for me, independent of the image in the dataset. 

The raw files can be opened in ImageJ - see Settings.png for options which should be set when opening. To open mask_1004244319.dcm.raw you must switch on the option "Little-endian bit order"


g++ -o analiza_1 analiza_1.cpp -lm -lpthread -lX11 -O3

g++ -o sciezki sciezki_13.11.2018.cpp -lm -lpthread -lX11 -O3
