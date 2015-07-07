/* QVisu (c) 2015 Maximilian Gauger mgauger@kalassi.de
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QVWEATHER_H
#define QVWEATHER_H

#include <QDateTime>
#include <QtXml>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QTimer>
#include "../qvelement.h"
#include "../qvsvgwidget.h"

class weather_value {
public:
    weather_value() {
        valid = false;
    }

    bool valid;
    double value;
    QString unit;
    int timespan;

};

class weather_entry {
public:
    weather_value
    temperature,
    windDirection,
    windSpeed,
    humidity,
    pressure,
    cloudiness,
    fog,
    precipitation;
};

class QVWeather : public QVElement
{
    Q_OBJECT
public:
    explicit QVWeather(QDomElement xml_desc, QString container, QWidget *parent = 0);
    void resizeEvent(QResizeEvent*);

signals:

public slots:
    void onXMLReceived(QDomElement);
    void displayWeather();

protected slots:
    void dataReceived(QNetworkReply*);
    void fetchAgain();

private:
    QMap<qint64,weather_entry*> weather_entries;

    QLabel *w_title;
    QList<QVSvgWidget*> w_clouds, w_winddir;
    QList<QLabel*> w_times, w_temp, w_separator;
    QString s_svg_wind;
    QString s_svg_weather;

    int n_days;

    QNetworkAccessManager *network_manager;
    QTimer *fetch_timer;
    int fetch_counter;
    double latitude, longitude;

    const static int padding = 5;
};

#endif // QVWEATHER_H
