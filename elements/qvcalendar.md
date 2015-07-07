# Google Calendar Element {#calendar}

Displays a Google calendar.

As Google uses OAuth 2.0, you do not need to provide username and password. Rather, QVisu will display a code in the format AAAA-BBBB when first used,
which you must enter on [the Google site](http://www.google.com/device). You then need to log in and authorize the use of QVisu.

**NOTE:** If you derive your own version of QVisu and either modify the calendar element or use the OAuth 2 code in your own element, use your own app credentials. 
Using my credentials for your work is not covered by the copyright.

## Syntax

<pre>
&lt;element type="calendar" position="x,y" height="h"&gt;
	&lt;color&gt;...&lt;/color&gt;
	&lt;text&gt;...&lt;/text&gt;
	&lt;max-days&gt;...&lt;/max-days&gt;
	&lt;calendar&gt;...&lt;/calendar&gt;
&lt;/element&gt;
</pre>

Elements in **bold** are mandatory.

| element/tag           | comment                                                                                                                       |
|-----------------------|-------------------------------------------------------------------------------------------------------------------------------|
| height                | Can be chosen arbitrarily, default is 4. Width is fixed at 4.                                                                 |
| color                 | Background color. Default value is cyan.                                                                                      |
| max-days              | Maximum number of days (from today) for which entries are retrieved.                                                          |
| **calendar**          | The calendar ID (see below)                                                                                                   |

To retrieve you calendar ID:
- Go to your [Google Calendar](http://calendar.google.com). 
- On the left side, below "My Calendars", click on the down arrow next to you calendar name.
- Click on "Calendar Settings"
- In the line "Calendar Address", look for an entry "Calendar ID". The value after the colon is what you need.

## Known bugs
- On startup, all future entries are retrieved. Later, only updates are requested from Google. For some reason, Google then sends the full list.
- The update interval is fixed at 30s.

