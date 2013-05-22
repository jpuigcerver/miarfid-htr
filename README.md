miarfid-htr
===========

Handwriting Text Recognition exercises for Daniel Keyser's lectures.

  - Author: Joan Puigcerver i Pérez <joapuipe@upv.es>
  - Source code: https://github.com/jpuigcerver/miarfid-htr

Requirements
------------

  - C++ Compiler (tested with g++-4.6 and g++-4.8)
  - Magick++ (included in ImageMagick) (tested with version 6.8.5)
  - libgflags-2.0 (https://code.google.com/p/gflags/)
  - libglog-0.3.3 (https://code.google.com/p/google-glog/)

Exercise 1.4 (Connected Components)
-----------------------------------

This exercise is implemented by conncomp.cc. It uses a MFSet to build the
the different sets of connected pixels. By default it uses 4-connectivity but
it also accepts 8-connectivity. The statement of the exercise asked to output
a single image where the background pixels have a value of 0 and each of the
foreground components is painted with its id number. This approach has several
disadvantages:
  - It may happen that there are more than 255 foreground components, so we
    would need more than the default 1-byte per pixel.
  - Even if there is only one foreground component, the color difference
    between the background (0) and the component (1) is indistinguishable to the
    human eye.
Thus, I changed the output so that it uses color 0 for the background and only
a desired maximum number of colors for the foreground components (10 by default).
Anyway, if you want to get the output as the statement specified, you can pass
the -s option to the program.

Here is the list of relevant options that can be passed to the program:
```
$ ./conncomp --helpshort
conncomp: Computes the connected components of an image.

  Flags from conncomp.cc:
    -b (Background color) type: double default: 1
    -c8 (Use eight-neighbors connectivity) type: bool default: false
    -h (Output the components map in a human-readable way) type: bool
      default: false
    -i (Input image. Use '-' for standard input) type: string default: "-"
    -m (Output one image for each component. Use -o as preffix.) type: bool
      default: false
    -nc (Number of colors to use to paint the connected components)
      type: uint64 default: 10
    -o (Output image. Use '-' for standard output) type: string default: "-"
    -s (Output painted using the strict definition on the statement) type: bool
      default: false
```

Exercise 1.5 (Features of Connected Components)
-----------------------------------------------

This exercise is implemented by connfeat.cc. It extracts the features indicated
in the exercise statement. You can use the -m flag of the previous solution to
generate a set of suitable foreground component images. This program assumes that
the input image is formed by a single connected component.

Here is the list of relevant options:
```
$ ./connfeat --helpshort
connfeat: Computes the features of a connected component image.

  Flags from connfeat.cc:
    -b (Background color) type: double default: 1
    -c8 (Use eight-neighbors connectivity) type: bool default: false
    -i (Input image. Use '-' for standard input) type: string default: "-"
```
