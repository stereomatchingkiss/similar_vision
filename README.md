# Synopsis

similar_vision is a free tool inspired by [Awesome Duplicate Photo Finder](http://www.duplicate-finder.com/photo.html), 
this app can help you find and remove duplicate photos(and videos in the future) on your PC. 

Same as [Awesome Duplicate Photo Finder](http://www.duplicate-finder.com/photo.html), similar_vision is a completely FREE Software. It contains absolutely 
NO ADWARE, NO SPYWARE, NO REGISTRATION, NO POPUPS, NO MALWARE or other unwanted software.

Although this app is inspired by [Awesome Duplicate Photo Finder](http://www.duplicate-finder.com/photo.html), it is not a 
copy and paste of it. To make it 

# ScreenShot

![Found image](https://a.fsdn.com/con/app/proj/similar-vision/screenshots/similar_00.JPG)
![Accuracy chart](https://a.fsdn.com/con/app/proj/similar-vision/screenshots/similar_01.JPG)

# Features
- Simple interface
- Finds similar or exact duplicate photos
- Search through the following image types: BMP, JPG, PNG, PBM, PGM, PPM, TIFF, WEBP
- Support 4 algorithms
    - Average hash
	- PHash
	- Marr Hildreth Hash
	- Block Mean Hash(mode 0 and mode 1)
- Able to compare pictures that was resized, blurring, different contrast, 
blurring, noise(gaussian), embedded with watermark, different format, jpeg compression 
- Add multiple folders or drives for scanning 
- Works with removable devices (USB etc.)
- Support moving and deleting of duplicate photos
- Saving settings, including geometry
- Self explain chart of the pros and cons of different algorithms
- You can update this app with one click
- This is an open source software, you can tweak it as you like
- Cross platform, support windows XP~windows10 and Linux

# Motivation

- Show the power of [img_hash module](https://github.com/stereomatchingkiss/opencv_contrib/tree/img_hash/modules/img_hash)
- Create a platform to demonstrate the performance of different algorithms
- It is fun

# Installation

This module only depend on opencv_core and opencv_imgproc, by now it is not merged into the opencv_contrib yet, to build it, you can try following solution(please tell me a better one if you know, thanks)

1. Unzip the file you download from [SourceForge](https://sourceforge.net/projects/similar-vision/files/?source=navbar)
2. Double click the program--similar_vision, this is all you need to do

# Email

If you have any questions, like bugs, desired features, nice algorithms want me to 
implement, please send it to my email--thamngapwei@gmail.com.

# For programmers

Please go to [details](https://github.com/stereomatchingkiss/similar_vision/blob/master/DETAILS.md)