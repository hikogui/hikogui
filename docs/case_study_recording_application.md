# Case Study: Recording Application

The application will need to run on:

 * Windows 10
 * Mac OS
 * iOS
 * Android

The user interface needs to be silky smooth with
real time updates for the audio related user interface elements like
audio meters, spectrum analyser, waveform viewer.

## GUI Library
### Compositing
Use Vulkan for compositing pre-drawn user interface elements at variable
positions and rotation. Share pre-drawn layers between elements that are
equal to composite them multiple times at different positions and rotations.

All compositing commands should be properly sorted based on type, to improve
rendering/compositing speed, in the same way as game graphic engines. Use Z buffering
for compositing to sort by type even for overlapping user interface elements.

In principle the user interface is redrawn completely every buffer-swap, i.e. 60 fps.
A background cached texture map can be used to draw all fixed elements in a single
draw with just clearing the Z buffer. Changed opaque elements can draw directly on top
of cached texture map without clearing the texture map, however all sub elements need
to be redrawn above that (how to handle clearing of the Z buffer in this case).

### Realtime Drawing
Drawing of simple shapes like lines, boxes, circles can be done directly with
triangles and simple fragment shaders including anti-aliasing.

Drawing of real time text can be done using glyph cached texture map on top of
triangles. Use libfreetype to render directly into a high resolution bitmap,
and pass it to Vulkan using `VK_FORMAT_R16_UINT` 4x4 pixel per texel then use
super-sampling when compositing.

### Path Rendering
For comercial exploitation of the GUI Toolkit I want to implement 2D path rendering
in the toolkit itself. TrueType fonts are pretty easy to read and render needing only
quadratic-bezier-paths. However doing scanline rendering is more computational
efficient than fragment rendering, since ray-casting over the y-axis allows for a
lot of pre computation. The rest of the path rendering can be build on top of the
font render engine, including pixel hinting.

For the first version use Cairo and Pango to draw anti-aliased black-and-white
textures that are then composited using my own engine.

## API/Classes

### Image
 * width, height
 * origin-x,y (compared to left top of image).
 * Reference to Vulkan Image Handle
 * Reference to data mapping.

An image can be loaded from disk using the .png format.
The memory format is equal to Vulkan's image format and
can be mapped directly from the GPU.

### ImageEffects
Effects that can be executed on a full color or gray-scale image,

 * Gaussion blur
 * Matte Composit (Select between two source from the list)
   * Image
   * Fill solid colour
   * Fill linear gradiant
   * Fill radial gradiant

### Path
A path is build up from quadratic-bezier curves which can
be filled or stroked into an gray-scale `Image` for matte purpose.
Specification according to TrueType Fonts, which includes hinting.

Store all bezier curves in a 1D table. Use fragment shader to pre-
compute the y-axis parameters into a 2D table. Use a second fragment
shader to compute inside/distance test for each pixel from this
2D table. How many y-axis parameters are needed? How well does
Vulkan handle non-square non-power-of-two images?

### Font
A font is a collection of `Paths`.

### ImageCopyCommand
 * x,y vector from origin of destination to origin of `Image`.
 * rotation around z-axis at origin of `Image`.
 * z-depth for depth test.
 * Image
 * Super sampling required when doing non-integer moves or rotations.

### ImageCopyCommandQueue
Queue of image-copy-commands to execute on each frame.
All ImageCopyCommand share the shader, but some commands
need super-sampling, there should be two physical queues
to seperate the two different Vulkan pipelines that are needed.

### RealTimeDrawCommand
 * Triangle Data, including colour.
 * Use super sampler for anti-aliased lines.

### RealTimeDrawCommandQueue
All draw commands share the same vulkan pipeline for
drawing of solid shapes.

### Widget
 * static data (incl width, height).
 * implements key for HashMap based on static-data.
 * shared between multiple `WidgetCell`.
 * Manages Images

The Widget draws the images needed by a WidgetCell.
Draws images lazilly, cheaper for WidgetCache.

A Widget's static data is immutable after creation.
Images are created purely based on its static data.

### WidgetCache
 * Tracks sharing of Widgets with equal static-data.
 * wantToUseWidget(Widget) -> Widget
 * finishedUsingWidget(Widget)
Every Widget needs to be moved through the WidgetCache, which
may return a prefiously created Widget in return.
If a WidgetCell needs a change a Widget it needs to create
a new Widget and return the old one to the Cache.

### WidgetCell
 * References Widget for drawing of shared static images.
 * Creates and manages ImageCopyCommands using variable data.
 * Creates and manages RealTimeDrawCommands using variable data.
 * variable data (incl x,y coords)
 * Every WidgetCell can be a container for other WidgetCells.
 * Animate between values of variable data, such as the X-Y coord.
 * Track mouse events into bounding shapes (rectangle, circle).
 * Track Tap, shift-tap keyboard-focus switching between children
   and pass back to parent.

### Window
An application can open one or more windows.
An application can have one or more of the windows in fullscreen-borderless.

An application wants to open small always-ontop-of-application windows for panels
which can embed as a widget inside another window.

Resize in Vulkan is expensive requiring building of a new surface, swap-buffers
queues, etc. After resize Widgets may need to be re-created and re-drawn based
on the new size.

Interact with Mac OS's top-of-screen main menu.

May need to open operating system supplied dialogue-windows, such as open-file dialogue to
interact with the sandbox environment. Other operating system supplied dialogues:
Colour-palette, Font-selector, Media-selector, Address-book.

The operating system may also open modal permission dialogue windows, for example when using
the audio input, address book, camera.

## External Libraries needed

 | Library       | License                              | Commercial | Dependencies                                  |
 | ------------- | ------------------------------------ | ---------- | --------------------------------------------- |
 | Vulkan        | Apache License 2.0                   | Y          |                                               |
 | VulkanSamples | Apache License 2.0                   | Y          |                                               |
 | cereal        | BSD                                  | Y          | -                                             |
 | glslang       | Google (BSD-like), NVIDIA (MIT-like) | Y          | -                                             |
 | SPIRV-Cross   | Apache License 2.0                   | Y          |                                               |
 | SPIRV-Tools   | Apache License 2.0                   | Y          |                                               |
 | SPIRV-Headers | Kronos (BSD Like)                    | Y          |                                               |
 | MoltenVK      | Apache License 2.0                   | Y          | cereal, SPIRV, glslang, Vulkan, VulkanSamples |
 | libfreetype   | FTL (BSD Like) / GPL2                | Y          | -                                             |
 | Pango         | LGPL 2                               | Y          | Cairo, Fontconfig                             |
 | Fontconfig    | MIT                                  | Y          | Freetype                                      |
 | Cairo         | LGPL 2.1                             | Y          | libpng, pixman, Fontconfig                    |
 | libpng        | zlib/libpng                          | Y          | -                                             |
 | pixman        | MIT                                  | Y          | -                                             |

## Data Controller
A data-controller maintains a set of values that can be simultaneous controlled and queried by
user interface elements, physical controls, or other devices.

User interface elements may need to change visually when data changes. And the user interface
element should be able to change data when the user controls the user interface element.

Some values may need to be locked, such as the sample rate, the value will try to force
the locked value or give and error if it is not possible to force the value. The user interface
element or physical feedback should display an error (possibly using an error sound).

The 'configure' (right click) menu of a user interface element will show information about the
data-controller that is linked to it, including other user interface elements or physical controls.
This way it is possible to link other user interface elements, physical controls and devices directly
from the user interface element.

## Needed user interface elements

### Window / Panel
The application may be one or more windows, full screen, on multiple screens.

### Text Label
Drawing ASCII text with real time update with for example time code with
display characters using a mono-font.

Non-cached rendering.

 * Also used as tumbler for setting values.

### Text Input Field
User editable text.

 * Unicode with right-to-left support.
 * Single or multiple line.
 * Template place holders which can be dragged and configured.
 * Turning editing off can be used as static label.
 * Horizontal, Vertical, Diagonal.
 * Spell check.

### Pull Down Menu
Menu items are widgets, including the pull down menu itself.
Normal items are Buttons with text.

### Button
Custom Drawing, or fixed images/text.
May show multiple states at once.

 * On/Off
 * Check Box
 * Radio Button
 * "Recording"

### Slider / Knob
Real time update of slider position, or knob rotation on top
of a static background.

### 2D Table

 * Scrolling.
 * Add row / delete row buttons.
 * Widgets in each cell.

### Assignment Grid
A grid to assign inputs to channels, channels to outputs, channels to files, files to folders.
Temporary, I don't like this user interface in Boom Recorder, replace with drag/drop grid?

### Tabs
Container that switches between tabs.

### Ring Buffer viewer
Show the status of the ring buffer and audio I/O.

### Audio Meter
Show audio levels:

 * True-peak
 * Loudness level
 * Peak and hold
 * Maximum peak (resettable).
 * Enabled
 * Armed
 * Multiple channels
 * Configurable red, yellow, green areas

### Spectrum Analyser

 * Frequency decades or Octave grid
 * Configurable lowest and highest level.

### Timeline/Waveform viewer
Show recordings throughout the day on a single timeline.

 * Current recording point
 * Current listening point
 * Peak/Loudness level of multi/channels
 * Start / Stop points of recordings.
 * Collapse between Stop and Start.
 * Shows recording metadata above Start/Stop points
 * Shows clipping
 * Shows missing samples

### Drag/Drop Grid
The user can place user interface elements on a grid.
Other user interface elements may move when space is needed to
drop an element in its place.

Used for placing channel faders, gain, meters on a grid custom made
by the user.

 * Snap on grid.
 * Elements should be a multiple of grid distance in height and width.


### Plug-in Element
Vendors should be able to supply elements.

 * Plug-ins may be code for Mac OS and Window, but not iOS or Android
 * Plug-ins may be a description of a combination of existing user interface elements.
   * Custom Background image
   * Custom colours
 * Link to networked/MIDI/etc parameters.




