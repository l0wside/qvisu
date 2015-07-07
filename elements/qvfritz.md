# Fritz!Box Element {#fritz}

The Fritz!Box element displays the last calls on your Fritz!Box.
The Fritz!Box must support username/password authentication. The old authentication (password only) is not supported.

## Syntax

<pre>
&lt;element type="fritzbox" position="x,y" height="h"&gt;
	&lt;color&gt;...&lt;/color&gt;
	&lt;text&gt;...&lt;/text&gt;
	&lt;server&gt;...&lt;/color&gt;
	&lt;user&gt;...&lt;/user&gt;
	&lt;password&gt;..&lt;/password&gt;
&lt;/element&gt;
</pre>

Elements in **bold** are mandatory.

| element/tag           | comment                                                                                                                       |
|-----------------------|-------------------------------------------------------------------------------------------------------------------------------|
| height                | Can be chosen arbitrarily, default is 4. Width is fixed at 4.                                                                 |
| show                  | Any combination of *incoming*, *outgoing*, and *missed*, separated by commas. Default is to show all calls.                   |
| text                  | Title text                                                                                                                    |
| color                 | Background color. Default value is cyan.                                                                                      |
| server                | Name or IP address of the Fritz!Box                                                                                           |
| user, password        | Your credentials for the Fritz!Box. Creating a read-only account for this purpose is recommended.                             |

## Known bugs
- The display will be completely reworked, to show every caller/callee only once

