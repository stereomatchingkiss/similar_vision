##Synopsis

similar_vision is a free tool aim to offer a gui tools to visualize the 
performance of difference algorithms on finding similar images and videos.

##Dependencies

The dependency of this project are

1. [Qt5.6.x](https://www.qt.io/download-open-source/#section-2)
2. [OpenCV3.x](http://opencv.org/)
3. [img_hash module of opencv_contrib(haven't merged yet)](https://github.com/stereomatchingkiss/opencv_contrib/tree/img_hash/modules/img_hash)
4. [qt_enhance](https://github.com/stereomatchingkiss/qt_enhance/tree/master/compressor)

##How to build them

#Qt5.6.x

Community already build it for us, just download the installer suit your os and compiler.

#OpenCV3.x

Same as Qt5, community build it for us, lucky part of this library is it is very easy to build,
the community do a good job on mantaining their make file.

#img_hash module

This [page]((https://github.com/stereomatchingkiss/opencv_contrib/tree/img_hash/modules/img_hash)) show you the details.

#qt_enhance

Do not need to build them, just clone the folders and place them as the same 
parent directory of similar_vision. The hierarchy looks like

--similar_vision
  --codes
--qt_enhance
  --codes

##Bugs report and features request

If you found any bugs or have any features request, please open the issue at github.
I will try to solve them when I have time.