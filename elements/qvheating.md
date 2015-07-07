# Heating Element {#plot}

The heating element controls the set temperature, displays the actual temperature, and in addition can show any number of status values (limited only be screen space)

## Syntax

<pre>
	&lt;element type="heating" position="x,y" width="..."&gt;
		&lt;icon&gt...&lt;/icon&gt;
		&lt;text&gt;...&lt;/text&gt;
		&lt;color&gt;...&lt;/text&gt;
		&lt;item action="temp-value"&gt;...&lt;/item&gt;
		&lt;item action="temp-setpoint" step="..."&gt;...&lt;/item&gt;
		&lt;item action="value" label="..." unit="..." precision="..."&gt;...&lt;/item&gt;
		&lt;item action="switch" icon="..."&gt;...&lt;/item&gt;
		&lt;item action="status" icon="..."&gt;...&lt;/item&gt;
	&lt;/element&gt;
</pre>

Elements in **bold** are mandatory.

| element/tag         | comment                                                                                                                      |
|---------------------|------------------------------------------------------------------------------------------------------------------------------|
| width               | Can be chosen arbitrarily, minimum width is 2. Height is fixed at 2.                                                         |
| text                | Title text                                                                                                                   |
| color               | Background color. Default value is cyan.                                                                                     |
| icon                | Icon displayed next to the text.                                                                                             |

## Item Syntax

### Actual Temperature
```<item action="temp-value">item name</item>```

The name of the item to be displayed. Digits after the decimal point are fixed at 1 (i.e. format is 21.5 &deg;C). The unit &deg;C is fixed.

### Temperature Setpoint
```<item action="temp-setpoint" step="...">item name</item>```

The name of the item which defines the temperature setpoint. *step* defines the step size by which the value increases/decreases with + and -.

### Value items

All items except *actual temperature* and *temperature setpoint* are displayed in one row at the bottom of the element. 

```<item action="value" label="..." unit="..." precision="...">item name</item>```

| tag                 | comment                                                                                                                      |
|---------------------|------------------------------------------------------------------------------------------------------------------------------|
| label               | Text before the displayed value                                                                                              |
| unit                | Text after the displayed value                                                                                               |
| precision           | Number of digits after the decimal point                                                                                     |

To display relative humidity, use ```<item action="value" label="rH" unit="%" precision="0">itemname</item>```

### Switch and status items

```<item action="..." icon="...">item name</item>```

| tag                 | comment                                                                                                                  |
|---------------------|--------------------------------------------------------------------------------------------------------------------------|
| action              | *switch* to actively switch the item, *status* to only display it.                                                       |
| color               | Color in off state, default is white                                                                                     |
| active-color        | Color in on state, default is orange                                                                                     |
| icon                | Icon name. If you specify two icons (separated by comma), they will be used for off and on.                              |
| color-mode          | SVG recoloring mode, see @ref switchicons                                                                                | 

## Known bugs
- The whole element looks rather ugly. Any idea for better formatting is welcome.