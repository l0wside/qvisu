# Dimmer {#dimmer}

![Dimmer Example](dimmer.png)

The dimmer can set a percentage value, intended to control a dimmer. It also includes an (optional) switch to turn off/on the corresponding light.

## Syntax

<pre>
&lt;element type="dimmer" position="x,y" width="..."&gt;
    &lt;color&gt;...&lt;/color&gt;
    &lt;icon&gt;...&lt;/icon&gt;
	&lt;sw-icon&gt;...&lt;/sw-icon&gt;
    &lt;text&gt;...&lt;/text&gt;
	&lt;item action="switch"&gt;...&lt;/item&gt;
	&lt;item action="dimmer" max-value="..." type="..."&gt;item name&lt;/item&gt;
&lt;/element&gt;
</pre>

Elements in **bold** are mandatory.

| element/tag               | comment                                                                                                             |
|---------------------------|---------------------------------------------------------------------------------------------------------------------|
| width                     | Element width. Default width is 4.                                                                                  |
| color                     | Background color. Default value is cyan.                                                                            |
| text                      | Title text                                                                                                          |
| icon                      | Icon displayed next to the text. This icon is not recolored.                                                        |
| sw-icon                   | Icon representing the on/off status. See @ref switchicons                                                           |
| **item action="dimmer"**  | Dimmer item                                                                                                         |
| item action="switch"      | Item for on/off.                                                                                                    | 
| max-value                 | Maximum value (equivalent to 100%). Default is 255 (DPT5). Set to 100 if DPT9 is used.                              | 
| type                      | Either "int" (default) for DPT5 or "float" for DPT9                                                                 | 

Element height is 2.