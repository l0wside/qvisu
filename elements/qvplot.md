# Plot Element {#plot}

The plot element displays any number of historical data rows on up to two axes.

It is currently the most complex element, therefore prone to have the most bugs. Any feedback is welcome. 
- Rendering time is quite long, which might have to do with Qt&rsquo;s SVG routines and the sometimes immense amount of data. A subsampling algorithm is on my to-do list
- The widget does not allow to zoom in or request actual numeric values as smartvisu does

![](plot.png)

## Syntax

<pre>
	&lt;element type="plot" position="x,y" width="..." height="..." common-zero="..." xgrid="..."&gt;
		&lt;text&gt;...&lt;/text&gt;
		&lt;icon&gt;...&lt;/icon&gt;
		&lt;color&gt;...&lt;/color&gt;
		&lt;background-color&gt;...&lt;/background-color&gt;
		&lt;time-min&gt;...&lt;/time-min&gt;
		&lt;axis1 ymin="..." ymax="..." grid="..."&gt;
			&lt;item action="..." color="..." name="..."&gt;item name&lt;/item&gt;
			&lt;item action="..." color="..." name="..."&gt;item name&lt;/item&gt;
			...
		&lt;/axis1&gt;
		&lt;axis2 ymin="..." ymax="..."&gt;
			&lt;item action="..." color="..." name="..."&gt;item name&lt;/item&gt;
			&lt;item action="..." color="..." name="..."&gt;item name&lt;/item&gt;
			...
		&lt;/axis2&gt;
	&lt;/element&gt;
</pre>

Elements in **bold** are mandatory.

| element/tag         | comment                                                                                                                      |
|---------------------|------------------------------------------------------------------------------------------------------------------------------|
| width, height       | Can be chosen arbitrarily.                                                                                                   |
| common-zero         | Only relevant if two axes are used. If set to "1" or "true", both x axes (y=0) are forced to be on one line.                 |
| xgrid               | Show a background grid at the time intervals                                                                                 |
| text                | Title text                                                                                                                   |
| icon                | Icon displayed next to the text.                                                                                             |
| color               | Background color of the element. Default value is cyan.                                                                      |
| background-color    | Background color of the actual plot area. Defaults to *color*                                                                |
| time-min            | Earliest time to be shown, the value is directly passed to the backend. End time is always the current time.                 |
| axis1               | Parent element for all items to be displayed on the **left** axis                                                            |
| axis2               | Parent element for all items to be displayed on the **right** axis                                                           |
| ymin, ymax          | Maximum and minimum values on the axis. If left out, the element will calculate them automatically                           |
| grid                | When set to "1" or "true", displays a horizontal grid for the axis values                                                    |


## Item Syntax

```<item action="..." color="..." name="..." >item name</item>```

| element/tag         | comment                                                                                                                      |
|---------------------|------------------------------------------------------------------------------------------------------------------------------|
| **action**          | The action is passed on to the backend as the "series" parameter. Only "avg" has been tested so far.                         |
| color               | The color to be used for drawing the item. Each axis is colored based on the color of the first item on the axis.            |
| name                | If given, the name is displayed next to the graph. The placement algorithm is not good, improvements welcome                 |
