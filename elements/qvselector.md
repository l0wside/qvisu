# Selector {#md_selector}

As its name says, the selector element selects the container to be displayed.

In addition, it can display any number of status elements, which can contain numbers, text, or icons. 
If there is only one such status element, it will be displayed inline. If there is more than one, they will be displayed in two rows.

![Selector Examples](selector.png)

## Syntax

<pre>
&lt;element type="selector" position="x,y" width="w"&gt;
    &lt;color&gt;...&lt;/color&gt;
    &lt;active-color&gt;...&lt;/active-color&gt;
    &lt;icon&gt;...&lt;/icon&gt;
    &lt;text&gt;...&lt;/text&gt;
    &lt;target&gt;...&lt;/target&gt;
    &lt;item action="..." icon="..."&gt;...&lt;/item&gt;
&lt;/element&gt;
</pre>

Elements in **bold** are mandatory.

| element/tag      | comment                                                                                                                      |
|------------------|------------------------------------------------------------------------------------------------------------------------------|
| color            | Background color. Default value is cyan.                                                                                     |
| active-color     | Background color when the selector is active, i.e. the corresponding container is selected. Defaults to *color*              |
| icon             | Icon displayed next to the text. This icon is not recolored.                                                                 |
| text             | Title text displayed on the selector element, should describe the target container                                           |
| **target**       | Container to be selected when the selector is pressed                                                                        |
| item             | Status item to be displayed, see below                                                                                       | 

The height is always 1. The width can be specified, minimum (enforced) width is 2.

## Item Syntax

Any number of items can be specified (at least until you run out of screen space). 

- ```<item action="status" icon="..." color="..." active-color="..." color-mode="...">item name</item>```
  
  Displays a status icon for off/on. 
	| tag              | comment                                                                                                                  |
	|------------------|--------------------------------------------------------------------------------------------------------------------------|
	| color            | Color in off state, default is white                                                                                     |
	| active-color     | Color in on state, default is orange                                                                                     |
	| icon             | Icon name. If you specify two icons (separated by comma), they will be used for off and on.                              |
	| color-mode       | SVG recoloring mode, see @ref switchicons                                                                                | 

- ```<item action="value" label="..." unit="..." precision="...">item name</item>```

  Displays a value (like a temperature).
	| tag              | comment                                                                                                                  |
	|------------------|--------------------------------------------------------------------------------------------------------------------------|
	| label            | Text displayed before the value                                                                                          |
	| unit             | Text displayed after the value                                                                                           |
	| precision        | Digits after the decimal point                                                                                           |
  If the item value received is a text, it will be displayed as a text. *unit* and *precision* are ignored in this case.                          |
