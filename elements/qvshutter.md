# Shutter {#shutter}

The shutter element can control either a single window shutter, a double window shutter, or blinds (i.e. blind position and blade angle).

Depending on the items supplied, the element will automatically decide if to show a single shutter, a double shutter, or a blinds control.

## Syntax

### Single shutter

<pre>
&lt;element type="shutter" position="x,y"&gt;
    &lt;text&gt;...&lt;/text&gt;
    &lt;color&gt;...&lt;/color&gt;
    &lt;icon&gt;...&lt;/icon&gt;
	&lt;sw-icon&gt;...&lt;/sw-icon&gt;
	&lt;item action="up-down"&gt;...&lt;/item&gt;
	&lt;item action="stop"&gt;...&lt;/item&gt;
	&lt;item action="position" max-value="..."&gt;...&lt;/item&gt;
&lt;/element&gt;
</pre>

### Double shutter

<pre>
&lt;element type="shutter" position="x,y"&gt;
    &lt;text&gt;...&lt;/text&gt;
    &lt;color&gt;...&lt;/color&gt;
    &lt;icon&gt;...&lt;/icon&gt;
	&lt;sw-icon&gt;...&lt;/sw-icon&gt;
	&lt;item action="up-down"&gt;...&lt;/item&gt;
	&lt;item action="stop"&gt;...&lt;/item&gt;
	&lt;item action="position" max-value="..."&gt;...&lt;/item&gt;
	&lt;item action="up-down2"&gt;...&lt;/item&gt;
	&lt;item action="stop2"&gt;...&lt;/item&gt;
	&lt;item action="position2" max-value="..."&gt;...&lt;/item&gt;
&lt;/element&gt;
</pre>

### Blinds

<pre>
&lt;element type="shutter" position="x,y"&gt;
    &lt;text&gt;...&lt;/text&gt;
    &lt;color&gt;...&lt;/color&gt;
    &lt;icon&gt;...&lt;/icon&gt;
	&lt;sw-icon&gt;...&lt;/sw-icon&gt;
	&lt;item action="up-down"&gt;...&lt;/item&gt;
	&lt;item action="stop"&gt;...&lt;/item&gt;
	&lt;item action="position" max-value="..."&gt;...&lt;/item&gt;
	&lt;item action="blade-position" max-value="..."&gt;...&lt;/item&gt;
&lt;/element&gt;
</pre>

### Elements and Tags

Elements in **bold** are mandatory.

| element/tag                  | comment                                                                                                   |
|------------------------------|-----------------------------------------------------------------------------------------------------------|
| text                         | Title text                                                                                                |
| color                        | Background color. Default value is cyan.                                                                  |
| icon                         | Icon displayed next to the text.                                                                          |
| **item action="up-down"**    | Item for shutter up/down command (left shutter in case of double shutter)                                 |
| **item action="stop"**       | Item to stop a moving shutter (left shutter in case of double shutter)                                    | 
| item action="position"       | Relative position of the shutter (left shutter in case of double shutter)                                 |
| item action="up-down2"       | Item for shutter up/down command (right shutter)                                                          |
| item action="stop2"          | Item to stop moving right shutter                                                                         | 
| item action="position2"      | Relative position of the right shutter                                                                    |
| item action="blade-position" | Relative blade position                                                                                   |
| max-value                    | Maximum value (equivalent to 100%). Default is 255 (DPT5). Set to 100 if DPT9 is used.                    | 

Element size is 2x2. With a tip on the element, a larger popup window will open. To close it, tap anywhere outside the popup window.

### Examples

The image below shows all three versions: single shutter on top, double shutter in the middle, blinds control on the bottom. The standard element is on the left,
the corresponding popup on the right.

Note that in reality, only one popup can be open at a time, which will cover its element.

![](shutter.png)

### Use

Tap on the element to open a popup. The popup will close immediately once you have either triggered an action or tapped somewhere outside the popup.

#### Single Shutter

The popup will offer up and down arrows, a stop button and a shutter. Tap on the shutter to set the relative position. The buttons should be self-explaining.

#### Double shutter

The popup will offer up and down arrows and a stop button on both sides and a shutter icon. Tapping on either half of the shutter will set the relative position of the left or right shutter.

#### Blade Control

The popup will offer up and down arrows, a stop button, a shutter, and a blade control. Tapping on the shutter will set the relative height, tapping on the blade control will turn the blades.
You can either tap on the blades or drag them. The value will only be updated once you release the finger (or mouse).

### Known bugs

- The display of the shutter position seems not to be working properly.
