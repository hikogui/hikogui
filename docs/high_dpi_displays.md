High DPI Displays
=================

This document describes how HikoGUI scaled the UI when displaying a window on a high DPI display.

Device Independed Pixel
-----------------------

Different manufacturers of GUI systems have defined the DIP (Device Independed Pixel) as:
 - Microsoft: 1/96 Inch (1920 x 1080 @ 23").
 - Apple: 1/72 Inch
 - Android: 1/160 Inch
 - HikoGUI 1/96 Inch

1pt -> 4dp -> 1px


Scaling
-------
Scaling can be done in increments of 25%.

  Scale | 1dp  | 2dp | 3dp | 4dp | 5dp | 6dp | 7dp | 8dp 
 ------ | ----:| ---:| ---:| ---:| ---:| ---:| ---:| ---:
  100%  | 1dp  | 2px | 3px | 4px | 5px | 6px | 7px | 8px
  125%  | 1dp  | 2px | 3px | 4px | 5px | 6px | 7px | 8px


 - At 100%: 1 DIP corrosponds to 1 pixel.
 - At 125%: 1 DIP corrosponds to 1.25 pixel.
 - At 150%: 1 DIP corrosponds to 1.50 pixel.
 - At 150%: 1 DIP corrosponds to 1.50 pixel.

### Desktop computer

  Resolution  | Size | PPI | Scale  | Info
 ------------ |:---- |:--- |:------ |:----------------------
  1920 x 1080 | 24"  | 92  | 1      | Common office display.


### Notebook and tablets




### Phone





### Watch





