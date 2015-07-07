# Switch and Status {#switch}

![Various switches](switch.png)

The switch/status element displays an on/off value.

With ```<element type="switch">```, the user can modify the specified item. With ```<element type="status">```, it is merely a display of the 
current status. Otherwise, the two elements are identical.

## Syntax

<pre>
&lt;element type="switch" position="x,y" mini="..."&gt;
    &lt;color&gt;...&lt;/color&gt;
    &lt;active-color&gt;...&lt;/active-color&gt;
    &lt;icon&gt;...&lt;/icon&gt;
	&lt;sw-icon&gt;...&lt;/sw-icon&gt;
    &lt;text&gt;...&lt;/text&gt;
	&lt;item action="switch"&gt;...&lt;/item&gt;
&lt;/element&gt;
</pre>

Elements in **bold** are mandatory.

| element/tag      | comment                                                                                                                      |
|------------------|------------------------------------------------------------------------------------------------------------------------------|
| color            | Background color. Default value is cyan.                                                                                     |
| active-color     | Background color when value is "on". Defaults to *color*                                                                     |
| text             | Title text                                                                                                                   |
| icon             | Icon displayed next to the text. This icon is not recolored.                                                                 |
| sw-icon          | Main icon, representing the item status. See @ref switchicons                                                                |
| **item**         | Status item to be switched (or displayed)                                                                                    | 
| mini             | When set to "1", reduces the size to 1x1                                                                                     | 
| item             | The item to be displayed.                                                                                                    | 

The default size is 2x2. When *mini* is set to "1", the size reduces to 1x1.