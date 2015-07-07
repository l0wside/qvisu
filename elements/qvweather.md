# Weather Element {#weather}

The weather element downloads the weather forecast from yr.no and displays it.
For the next 24 hours, it shows max/min temperatures (in 6h intervals), clouds, precipitation (rain/snow), and wind.
For the following days, it shows max/min temperatures, clouds, and precipitation.

![](weather.png)

## Syntax

<pre>
<element type="weather" position="x,y" width="w" height="h">
	<latitude>...</latitude>
	<longitude>...</longitude>
	<color>...</color>
	<text>...</text>
	<days>...</days>
</element>
</pre>

Elements in **bold** are mandatory.

| element/tag             | comment                                                                                                                               |
|-------------------------|---------------------------------------------------------------------------------------------------------------------------------------|
| width, height           | Can be chosen arbitrarily, minimum width is 3, minimum height is 2. Default is 6x3                                                    |
| **latitude, longitude** | Geographical position of your location **in decimal notation**. Stuttgart (48&deg; 46' 39'' N, 9&deg; 10' 43'' E) is 48.7775 / 9.178611 |
| text                    | Title text                                                                                                                            |
| color                   | Background color. Default value is cyan.                                                                                              |
| days                    | Number of days after today for which the forecast is displayed. yr.no seems to have a ten-day forecast. Default is 3.                 |

## Known bugs
- The algorithm to convert the yr.no data to something that can be displayed is not optimal (and barely documented).
- Thunderstorms are not displayed, snowfall is shown as rain. The data from yr.no simply does not have the info.
- Non-transparent proxies are not supported, i.e. if you have something between you and the yr.no server that requires authentication, the element will fail.

Changing the data source to either the DWD plugin of smarthome.py or to the DWD FTP server would be a good thing. Volunteers welcome.

## Display algorithm

yr.no delivers an XML file with various forecasts for different time ranges and different items. For instance, precipitation and temperature are in different items. 
Plus, yr.no will deliver a forecast for the next day from 0:00 to 1:00, one from 0:00 to 6:00, one for the whole day.

QVisu first aggregates the data into its own data structure, always overwriting data for longer time ranges with data for shorter time ranges (to get the best resolution out of the data).

Then, it calculates the average precipitation (i.e. rain/snow) and wind from this data set, as well as the max and min temperatures. It ignores the max/min values from
yr.no and determines them directly from the expected temperatures. This works quite well for short-term, but has its downsides for long-term prognosis.