# Widgets

## Constraints

### Widget constraint variables

 * left
 * bottom
 * width
 * height
 * right (derived)
 * top (derived)
 * base

### Widget constraints

 * minimumWidth
    width >= Theme::smallSize (HIGH)
 * minimumHeight
    height >= calculated_label_size (HIGH)
 * preferedWidth
    width >= recommended_label_size (MID)
 * preferedHeight
    height >= recommended_label_size (MID)
 * maximumWidth
    width <= recommended_label_size (LOW)
 * maximumHeight
    height <= recommended_label_size (LOW)

 * baseHeight - Based on the visual middle of a widget.