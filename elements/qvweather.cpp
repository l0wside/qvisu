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
 *
 * Alle Icons sind unter einer Creative Commons Lizenz vom Typ Namensnennung -
 * Weitergabe unter gleichen Bedingungen 3.0 Deutschland zugänglich.
 * Um eine Kopie dieser Lizenz einzusehen, konsultieren Sie
 * http://creativecommons.org/licenses/by-sa/3.0/de/ oder wenden Sie
 * sich brieflich an
 * Creative Commons, Postfach 1866, Mountain View, California, 94042, USA.
 */

#include <QFile>
#include <QThread>
#include <QNetworkReply>
#include "qvweather.h"

QVWeather::QVWeather(QDomElement xml_desc, QString container, QWidget *parent) :
    QVElement(xml_desc,container,parent)
{
    if (w < 3) {
        w = 6;
    }
    if (h < 2) {
        h = 3;
    }

    QDomElement xml_elem = xml_desc.firstChildElement("text");
    if (xml_elem.isNull()) {
        w_title = 0;
    } else {
        w_title = new QLabel(xml_elem.text(),this);
    }

    bool ok;
    longitude = latitude = std::numeric_limits<double>::infinity();

    xml_elem = xml_desc.firstChildElement("latitude");
    if (!xml_elem.isNull()) {
        double res = xml_elem.text().toDouble(&ok);
        if (ok) {
            latitude = res;
        }
    }
    xml_elem = xml_desc.firstChildElement("longitude");
    if (!xml_elem.isNull()) {
        double res = xml_elem.text().toDouble(&ok);
        if (ok) {
            longitude = res;
        }
    }

    xml_elem = xml_desc.firstChildElement("days");
    if (xml_elem.isNull()) {
        n_days = 3;
    } else {
        n_days = xml_elem.text().toInt(&ok);
        if (!ok) {
            n_days = 3;
        }
    }

    QFile f_svg_weather(":/icons/weather.svg");
    if (f_svg_weather.open(QIODevice::ReadOnly)) {
        s_svg_weather = f_svg_weather.readAll();
        f_svg_weather.close();
    }

    QFile f_svg_wind(":/icons/weather_wind.svg");
    if (f_svg_wind.open(QIODevice::ReadOnly)) {
        s_svg_wind = f_svg_wind.readAll();
        f_svg_wind.close();
    }

    /* Generate widgets for displaying today´s forecast */
    for (int n=1; n < 4; n++) {
        w_separator.append(new QLabel(this));
        w_separator.last()->setStyleSheet("QLabel { background-color:#ffffff }");
        w_separator.last()->setFixedWidth(2);
    }

    for (int n=0; n < 4; n++) {
        w_times.append(new QLabel(this));
        w_times.last()->setAlignment(Qt::AlignHCenter);
        w_times.last()->setStyleSheet("QLabel { background:none }");
    }

    for (int n=0; n < 4; n++) {
        w_clouds.append(new QVSvgWidget(this));
    }

    for (int n=0; n < 4; n++) {
        w_winddir.append(new QVSvgWidget(this));
    }

    for (int n=0; n < 4; n++) {
        w_temp.append(new QLabel(this));
        w_temp.last()->setAlignment(Qt::AlignRight);
        w_temp.last()->setObjectName("mini");
        w_temp.last()->setStyleSheet("QLabel { background-color:none }");
        w_temp.append(new QLabel(this));
        w_temp.last()->setAlignment(Qt::AlignRight);
        w_temp.last()->setObjectName("mini");
        w_temp.last()->setStyleSheet("QLabel { background-color:none }");
    }

    /* ...and widgets for the next n_days days */
    for (int n=1; n < n_days; n++) {
        w_separator.append(new QLabel(this));
        w_separator.last()->setStyleSheet("QLabel { background-color:#ffffff }");
        w_separator.last()->setFixedWidth(2);
    }

    for (int n=0; n < n_days; n++) {
        w_times.append(new QLabel(this));
        w_times.last()->setAlignment(Qt::AlignHCenter);
        w_times.last()->setStyleSheet("QLabel { background:none }");
    }

    for (int n=0; n < n_days; n++) {
        w_clouds.append(new QVSvgWidget(this));
    }

    for (int n=0; n < n_days; n++) {
        w_temp.append(new QLabel(this));
        w_temp.last()->setAlignment(Qt::AlignRight);
        w_temp.last()->setObjectName("mini");

        w_temp.append(new QLabel(this));
        w_temp.last()->setAlignment(Qt::AlignRight);
        w_temp.last()->setObjectName("mini");
    }

    w_separator.append(new QLabel(this));
    w_separator.last()->setStyleSheet("QLabel { background-color:#ffffff }");
    w_separator.last()->setFixedHeight(2);


#if 1
    if ((longitude != std::numeric_limits<double>::infinity()) && (latitude != std::numeric_limits<double>::infinity())) {
        network_manager = new QNetworkAccessManager();
        QObject::connect(network_manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(dataReceived(QNetworkReply*)));
        network_manager->get(QNetworkRequest(QUrl("http://api.yr.no/weatherapi/locationforecast/1.9/?lat=" + QString::number(latitude,'f',2) + ";lon=" + QString::number(longitude,'f',2))));
        fetch_timer = new QTimer();
        QObject::connect(fetch_timer,SIGNAL(timeout()),this,SLOT(fetchAgain()));
    }

#else
    /******** Dummy ***********/



    QDomDocument doc;
    QFile f(QStringLiteral("c:/temp/api.yr.no.xml"));
    f.open(QIODevice::ReadOnly);
    doc.setContent(f.readAll());
    f.close();
    qDebug() << "constr onXML";
    onXMLReceived(doc.documentElement());


    /******** End Dummy ***********/
#endif
}

void QVWeather::dataReceived(QNetworkReply* reply) {
    if (reply->isRunning()) {
        return;
    }
    QDomDocument doc;
    doc.setContent(reply->readAll());
    qDebug() << "constr onXML";
    onXMLReceived(doc.documentElement());
    fetch_timer->setSingleShot(false);
    fetch_timer->setInterval(1000);
    fetch_counter = 3600;
    fetch_timer->start();
}

void QVWeather::fetchAgain() {
    fetch_counter--;
    if (fetch_counter <= 0) {
        fetch_timer->stop();
        network_manager->get(QNetworkRequest(QUrl("http://api.yr.no/weatherapi/locationforecast/1.9/?lat=" + QString::number(latitude,'f',2) + ";lon=" + QString::number(longitude,'f',2))));
    }
}

void QVWeather::resizeEvent(QResizeEvent *event) {
    if ((event->oldSize().height() == QWidget::height()) && (event->oldSize().width() == QWidget::width())) {
        return;
    }
    qDebug() << "weather resize " << width() << height();

    if (w_title) {
        w_title->resize(w_title->sizeHint());
        w_title->move(ofs_x()+(int)((width()-w_title->sizeHint().width())/2),ofs_y()+height()-w_title->sizeHint().height());
    }

    displayWeather();
}

/** Parse an XML document from yr.no, API version 1.9 */
void QVWeather::onXMLReceived(QDomElement e_weather) {
    QDomElement e_point_data = e_weather.firstChildElement("product");
    if (e_point_data.isNull() || !e_point_data.hasAttribute("class") || (e_point_data.attribute("class") != "pointData")) {
        return;
    }

    for (QMap<qint64, weather_entry*>::iterator iter = weather_entries.begin(); iter != weather_entries.end(); iter++) {
        delete (*iter);
    }
    weather_entries.clear();

    /* Parse XML file:
     * - Create an entry in weather_entries for each existing point in time
     * - Write the data with the shortest available timespan into the entry
     * Example: if there is a precipitation entry for 12:00 to 15:00 and one for 12:00 to 13:00, the latter will prevail
     */
    QDomNodeList e_time_list = e_point_data.elementsByTagName("time");

    /* Create entries */
    for (int n = 0; n < e_time_list.count(); n++) {
        if (!e_time_list.at(n).isElement()) {
            continue;
        }
        QDomElement e_time = e_time_list.at(n).toElement();
        if (!e_time.hasAttribute("datatype") || (e_time.attribute("datatype") != "forecast")) {
            continue;
        }

        QString s_from = e_time.attribute("from");
        if (s_from.isNull()) {
            continue;
        }

        QDateTime t_from = QDateTime::fromString(s_from,"yyyy-MM-ddThh:mm:ssZ");
        if (!t_from.isValid() || (QDateTime::currentDateTime().daysTo(t_from) > 5)) {
            continue;
        }
//        t_from.setTimeZone(QTimeZone("UTC",0,"UTC","UTC"));
        if (!weather_entries.contains(t_from.toMSecsSinceEpoch())) {
            weather_entries.insert(t_from.toMSecsSinceEpoch(),new weather_entry());
        }

    }

    /* Assign data to entries */
    for (int n = 0; n < e_time_list.count(); n++) {
        if (!e_time_list.at(n).isElement()) {
            continue;
        }
        QDomElement e_time = e_time_list.at(n).toElement();
        if (!e_time.hasAttribute("datatype") || (e_time.attribute("datatype") != "forecast")) {
            continue;
        }

        QString s_from = e_time.attribute("from");
        QString s_to = e_time.attribute("to");
        if (s_from.isNull() || s_to.isNull()) {
            continue;
        }

        QDateTime t_from = QDateTime::fromString(s_from,"yyyy-MM-ddThh:mm:ssZ");
        QDateTime t_to = QDateTime::fromString(s_to,"yyyy-MM-ddThh:mm:ssZ");
        if (!t_from.isValid() || !t_to.isValid()) {
            continue;
        }
        int timespan = (int)((t_to.toMSecsSinceEpoch() - t_from.toMSecsSinceEpoch())/1000);

        QList<weather_entry*> affected_entries;
        for (QMap<qint64,weather_entry*>::iterator iter = weather_entries.begin(); iter != weather_entries.end(); iter++) {
            if ((iter.key() >= t_from.toMSecsSinceEpoch()) && (iter.key() <= t_to.toMSecsSinceEpoch())) {
                affected_entries.append(*iter);
            }
        }

        QDomElement e_location = e_time.firstChildElement("location");
        if (e_location.isNull()) {
            continue;
        }
        QDomNodeList e_data_list = e_location.childNodes();
        for (int m=0; m < e_data_list.count(); m++) {
            if (!e_data_list.at(m).isElement()) {
                continue;
            }
            QDomElement e_data = e_data_list.at(m).toElement();

            for (QList<weather_entry*>::iterator we_iter = affected_entries.begin(); we_iter != affected_entries.end(); we_iter++) {
                weather_value *wv = 0;
                double value;
                QString unit;
                bool ok = false;

                if (e_data.nodeName() == "temperature") {
                    wv = &((*we_iter)->temperature);
                    value = e_data.attribute("value").toDouble(&ok);
                    unit = e_data.attribute("unit");
                } else if (e_data.nodeName() == "windDirection") {
                    wv = &((*we_iter)->windDirection);
                    value = e_data.attribute("deg").toDouble(&ok);
                    unit = "degrees";
                } else if (e_data.nodeName() == "windSpeed") {
                    wv = &((*we_iter)->windSpeed);
                    value = e_data.attribute("beaufort").toDouble(&ok);
                    unit = "bft";
                } else if (e_data.nodeName() == "humidity") {
                    wv = &((*we_iter)->humidity);
                    value = e_data.attribute("value").toDouble(&ok);
                    unit = e_data.attribute("unit");
                } else if (e_data.nodeName() == "pressure") {
                    wv = &((*we_iter)->pressure);
                    value = e_data.attribute("value").toDouble(&ok);
                    unit = e_data.attribute("unit");
                } else if (e_data.nodeName() == "cloudiness") {
                    wv = &((*we_iter)->cloudiness);
                    value = e_data.attribute("percent").toDouble(&ok);
                    unit = "percent";
                } else if (e_data.nodeName() == "fog") {
                    wv = &((*we_iter)->fog);
                    value = e_data.attribute("percent").toDouble(&ok);
                    unit = "percent";
                } else if (e_data.nodeName() == "precipitation") {
                    wv = &((*we_iter)->precipitation);
                    if (timespan > 0) {
                        value = e_data.attribute("value").toDouble(&ok) * 3600.0 / timespan;
                    } else {
                        value = e_data.attribute("value").toDouble(&ok);
                    }
                    unit = e_data.attribute("unit");
                }  else {
                    continue;
                }
                if (!ok) {
                    continue;
                }
                if (!wv->valid || (timespan < wv->timespan)) {
                    wv->valid = true;
                    wv->timespan = timespan;
                    wv->value = value;
                    wv->unit = unit;
                }
            }
        }


    }

    setContentsMargins(0,0,0,0);
    setLineWidth(0);
    setMidLineWidth(0);

    /* All entries now broken down into (at best) hourly data */
    qDebug() << "display from XML";
    displayWeather();
}


void QVWeather::displayWeather() {
    QDateTime now = QDateTime::currentDateTime();
    now.setTime(QTime(now.time().hour(),0,0));

    QList<QVSvgWidget*>::iterator iter_clouds = w_clouds.begin();
    QList<QVSvgWidget*>::iterator iter_winddir = w_winddir.begin();
    QList<QLabel*>::iterator iter_times = w_times.begin();
    QList<QLabel*>::iterator iter_temp = w_temp.begin();
    QDateTime lookup_time = now;
    while (lookup_time.time().hour() % 6) {
        lookup_time = lookup_time.addSecs(-3600);
    }

    int h = height();
    if (w_title != 0) {
        h -= w_title->height();
    }

    int part_width = (int)(width()/4);

    int upper_height = (int)(h*2/3)+5;
    int lower_height = h-upper_height+8;

    int time_height = w_times.at(0)->sizeHint().height();
    int cloud_height = (int)((upper_height-time_height)*2/3);
    int wind_temp_height = upper_height-cloud_height-time_height;

    int cloud_width = part_width - 2*padding;
    if (w_clouds.first()->aspectRatio() * cloud_width > cloud_height) {
        cloud_width = (int)floor(cloud_height/w_clouds.first()->aspectRatio());
    } else {
        cloud_height = (int)(floor(w_clouds.first()->aspectRatio() * cloud_width));
    }

    wind_temp_height = upper_height - time_height - cloud_height;

    int wind_width = (int)((part_width - 3*padding)*2/3);
    if (wind_width * 0.3 > wind_temp_height) {
        wind_width = (int)floor(wind_temp_height / 0.3);
    } else {
        wind_temp_height = (int)floor(wind_width * 0.3);
    }

    int layout_x = ofs_x();
    for (int n=0; n < 3; n++) {
        w_separator.at(n)->setFixedHeight(upper_height);
        w_separator.at(n)->move(ofs_x()+(n+1)*part_width,3);

    }

    for (int hour=0; hour < 24; hour+=6) {
        int layout_y = ofs_y();

        /* Find earliest corresponding weather entry */
        weather_entry *entry = 0;
        QMap<qint64,weather_entry*>::iterator iter_entry = weather_entries.end();
        if (weather_entries.size() > 0) {
            do {
                iter_entry--;
                if (iter_entry.key() <= lookup_time.toMSecsSinceEpoch()) {
                    entry = *iter_entry;
                    break;
                }
            } while (iter_entry != weather_entries.begin());
        }
        qDebug() << QDateTime::fromMSecsSinceEpoch(iter_entry.key()) << entry;

        double temp_min = std::numeric_limits<double>::infinity();
        double temp_max = -std::numeric_limits<double>::infinity();
        double rain = 0.0, clouds = 0.0;
        int rain_count = 0, cloud_count = 0;
        if (entry) {
            QMap<qint64,weather_entry*>::iterator iter_nextentries = iter_entry;
            while ((iter_nextentries.key() - iter_entry.key() < 6*3600*1000) && (iter_nextentries != weather_entries.end())) { // 6h
                if ((*iter_nextentries)->temperature.valid) {
                    if ((*iter_nextentries)->temperature.value < temp_min) {
                        temp_min = (*iter_nextentries)->temperature.value;
                    }
                    if ((*iter_nextentries)->temperature.value > temp_max) {
                        temp_max = (*iter_nextentries)->temperature.value;
                    }
                }
                if ((*iter_nextentries)->precipitation.valid) {
                    rain += (*iter_nextentries)->precipitation.value;
                    rain_count++;
                }
                if ((*iter_nextentries)->cloudiness.valid) {
                    clouds += (*iter_nextentries)->cloudiness.value;
                    cloud_count++;
                }
                iter_nextentries++;
            }
        }
        qDebug() << temp_min << temp_max;
        if (cloud_count > 0) {
            clouds /= (double)cloud_count;
        }
        if (rain_count > 0) {
            rain /= (double)rain_count;
        }

        (*iter_times)->setText(QString::number(lookup_time.time().hour()%24) + "-" + QString::number((lookup_time.time().hour()+6)%24));
        (*iter_times)->setFixedSize((*iter_times)->sizeHint());
        (*iter_times)->move(layout_x-2,layout_y);
        (*iter_times)->setFixedSize(part_width-4,time_height);
        layout_y += time_height + padding;
        iter_times++;


        QString s_cloud = s_svg_weather;

        if ((cloud_count == 0) || (rain_count == 0)) {
            qDebug() << "invalidating";
            /* No or incomplete data, hide all */
            s_cloud.replace("##SUN100##","0");
            s_cloud.replace("##SUN50##","0");
            s_cloud.replace("##CLOUDS50##","0");
            s_cloud.replace("##CLOUDS100##","0");
            s_cloud.replace("##RAIN25##","0");
            s_cloud.replace("##RAIN50##","0");
            s_cloud.replace("##RAIN100##","0");
        } else {
            /* No sun by night... */
            if ((lookup_time.time().hour() >= 18) || (lookup_time.time().hour() < 6)) {
                s_cloud.replace("##SUN100##","0");
                s_cloud.replace("##SUN50##","0");
            }

            if (clouds < 1.0) {
                s_cloud.replace("##SUN100##","1");
                s_cloud.replace("##SUN50##","0");
                s_cloud.replace("##CLOUDS50##","0");
                s_cloud.replace("##CLOUDS100##","0");
                s_cloud.replace("##DARKNESS##","ffffff");
            } else if (clouds < 30) {
                s_cloud.replace("##SUN100##","0");
                s_cloud.replace("##SUN50##","1");
                s_cloud.replace("##CLOUDS50##","1");
                s_cloud.replace("##CLOUDS100##","0");
                s_cloud.replace("##DARKNESS##","ffffff");
            } else if (clouds < 99) {
                s_cloud.replace("##SUN100##","0");
                s_cloud.replace("##SUN50##","1");
                s_cloud.replace("##CLOUDS50##","0");
                s_cloud.replace("##CLOUDS100##","1");
                QString cloud_brightness = QString::number(255-(int)(clouds * 1.27),16);
                while (cloud_brightness.length() < 2) {
                    cloud_brightness = "0" + cloud_brightness;
                }
                while (cloud_brightness.length() > 2) {
                    cloud_brightness.remove(0,1);
                }
                s_cloud.replace("##DARKNESS##",cloud_brightness + cloud_brightness + cloud_brightness);
            } else {
                s_cloud.replace("##SUN100##","0");
                s_cloud.replace("##SUN50##","0");
                s_cloud.replace("##CLOUDS50##","0");
                s_cloud.replace("##CLOUDS100##","1");
                s_cloud.replace("##DARKNESS##","808080");
            }

            /* Rain */
            if (rain < 1e-3) {
                s_cloud.replace("##RAIN25##","0");
                s_cloud.replace("##RAIN50##","0");
                s_cloud.replace("##RAIN100##","0");
            } else if (rain < 0.5) {
                s_cloud.replace("##RAIN25##","1");
                s_cloud.replace("##RAIN50##","0");
                s_cloud.replace("##RAIN100##","0");
            } else if (rain < 30) {
                s_cloud.replace("##RAIN25##","0");
                s_cloud.replace("##RAIN50##","1");
                s_cloud.replace("##RAIN100##","0");
            } else {
                s_cloud.replace("##RAIN25##","0");
                s_cloud.replace("##RAIN50##","0");
                s_cloud.replace("##RAIN100##","1");
            }
        }
        (*iter_clouds)->load(s_cloud.toUtf8());
        (*iter_clouds)->setFixedSize(cloud_height,cloud_width);
        (*iter_clouds)->move(layout_x + (int)((part_width - cloud_width)/2),layout_y);
        layout_y += cloud_height;

        //            if (entry) qDebug() << "dir/speed" << entry->windDirection.valid << entry->windSpeed.valid;
        if (entry && entry->windDirection.valid && entry->windSpeed.valid && !s_svg_wind.isEmpty()) {
            QString s_wind = s_svg_wind;
            QString wd;
            if (entry->windDirection.value < 22.5) {
                wd = "N";
            } else if (entry->windDirection.value < 67.5) {
                wd = "NO";
            } else if (entry->windDirection.value < 112.5) {
                wd = "O";
            } else if (entry->windDirection.value < 157.5) {
                wd = "SO";
            } else if (entry->windDirection.value < 202.5) {
                wd = "S";
            } else if (entry->windDirection.value < 247.5) {
                wd = "SW";
            } else if (entry->windDirection.value < 292.5) {
                wd = "W";
            } else if (entry->windDirection.value < 337.5) {
                wd = "NW";
            } else {
                wd = "N";
            }
/*            s_wind.replace("##DIR##",QString::number(entry->windDirection.value,'f',0)); */
            s_wind.replace("##WINDDIR##",wd + " " + QString::number(entry->windSpeed.value,'f',0));
            s_wind.replace("##ROT##",QString::number(90.0/15.0*entry->windSpeed.value-70.0,'f',3));

            (*iter_winddir)->load(s_wind.toUtf8());
        } else {
            (*iter_winddir)->load(QString().toUtf8());
        }
        (*iter_winddir)->setFixedSize(wind_width,wind_temp_height);
        (*iter_winddir)->move(layout_x,layout_y);

        QLabel *w_temp_max, *w_temp_min;
        w_temp_max = *iter_temp;
        iter_temp++;
        w_temp_min = *iter_temp;

        if (temp_min > temp_max) {
            w_temp_max->clear();
            w_temp_min->clear();
        } else if (temp_min == temp_max) {
            w_temp_max->clear();
            w_temp_min->setText(QString::number(temp_min,'f',0) + "°");
        } else {
            w_temp_max->setText(QString::number(temp_max,'f',0) + "°");
            w_temp_min->setText(QString::number(temp_min,'f',0) + "°");
        }

        w_temp_max->setFixedSize(w_temp_max->sizeHint());
        w_temp_max->move(layout_x + part_width - w_temp_max->sizeHint().width() - padding, upper_height - 2*padding - w_temp_min->sizeHint().height() - w_temp_max->sizeHint().height());
        w_temp_min->setFixedSize(w_temp_min->sizeHint());
        w_temp_min->move(layout_x + part_width - w_temp_min->sizeHint().width() - padding, upper_height - padding - w_temp_min->sizeHint().height());

        iter_temp++;
        iter_clouds++;
        iter_winddir++;

        layout_x += part_width;

        lookup_time = lookup_time.addSecs(6*3600);
    }

    qDebug() << "Next days";
    /* Next four days. Rain, clouds and wind daily; temperatures in 6h intervals. */
    part_width = (int)(width()/n_days)-2*(n_days-1);

    if (part_width > lower_height-time_height-padding) {
        cloud_width = lower_height - time_height - 3*padding;
    } else {
        cloud_width = part_width - 2*padding;
    }

    layout_x = ofs_x() + padding;
    for (int n=0; n < n_days-1; n++) {
        w_separator.at(n+3)->setFixedHeight(lower_height);
        w_separator.at(n+3)->move(ofs_x()+(n+1)*(part_width+2),upper_height+3);
    }
    w_separator.last()->setFixedWidth(width());
    w_separator.last()->move(ofs_x(),upper_height+3);

    lookup_time = now.addDays(1);
    lookup_time.setTime(QTime(0,0,0));
    for (int hour=0; hour < n_days*24; hour+=24) {
        int layout_y = upper_height+3+padding;

        (*iter_times)->setText(lookup_time.date().toString("ddd"));
        (*iter_times)->setFixedSize((*iter_times)->sizeHint());
        (*iter_times)->move(layout_x+(int)floor((part_width-(*iter_times)->width())/2),upper_height+3+padding);
        qDebug() << "IT" << (*iter_times)->x() << (*iter_times)->y();
        layout_y += time_height + padding;

        iter_times++;

        /* Find earliest corresponding weather entry */
        weather_entry *entry = 0;
        QMap<qint64,weather_entry*>::iterator iter_entry = weather_entries.end();
        if (weather_entries.count() > 0) {
            do {
                iter_entry--;
                if (iter_entry.key() <= lookup_time.toMSecsSinceEpoch()) {
                    entry = *iter_entry;
                    break;
                }
            } while (iter_entry != weather_entries.begin());
        }

        double temp_min = std::numeric_limits<double>::infinity();
        double temp_max = -std::numeric_limits<double>::infinity();
        double rain = 0.0, clouds = 0.0;
        int rain_count = 0, cloud_count = 0;
        if (entry) {
            QMap<qint64,weather_entry*>::iterator iter_nextentries = iter_entry;
            while ((iter_nextentries.key() - iter_entry.key() < 24*3600*1000) && (iter_nextentries != weather_entries.end())) { // 6h
                if ((*iter_nextentries)->temperature.valid) {
                    if ((*iter_nextentries)->temperature.value < temp_min) {
                        temp_min = (*iter_nextentries)->temperature.value;
                    }
                    if ((*iter_nextentries)->temperature.value > temp_max) {
                        temp_max = (*iter_nextentries)->temperature.value;
                    }
                }
                if ((*iter_nextentries)->precipitation.valid) {
                    rain += (*iter_nextentries)->precipitation.value;
                    rain_count++;
                }
                if ((*iter_nextentries)->cloudiness.valid) {
                    clouds += (*iter_nextentries)->cloudiness.value;
                    cloud_count++;
                }
                iter_nextentries++;
            }
        }
        if (cloud_count > 0) {
            clouds /= (double)cloud_count;
        }
        if (rain_count > 0) {
            rain /= (double)rain_count;
        }

        QString s_cloud = s_svg_weather;
        if ((rain_count == 0) || (cloud_count == 0)) {
            qDebug() << "invalidating";
            /* No or incomplete data, hide all */
            s_cloud.replace("##SUN100##","0");
            s_cloud.replace("##SUN50##","0");
            s_cloud.replace("##CLOUDS50##","0");
            s_cloud.replace("##CLOUDS100##","0");
            s_cloud.replace("##RAIN25##","0");
            s_cloud.replace("##RAIN50##","0");
            s_cloud.replace("##RAIN100##","0");
        } else {
            if (clouds < 1.0) {
                s_cloud.replace("##SUN100##","1");
                s_cloud.replace("##SUN50##","0");
                s_cloud.replace("##CLOUDS50##","0");
                s_cloud.replace("##CLOUDS100##","0");
                s_cloud.replace("##DARKNESS##","ffffff");
            } else if (clouds < 30) {
                s_cloud.replace("##SUN100##","0");
                s_cloud.replace("##SUN50##","1");
                s_cloud.replace("##CLOUDS50##","1");
                s_cloud.replace("##CLOUDS100##","0");
                s_cloud.replace("##DARKNESS##","ffffff");
            } else if (clouds < 99) {
                s_cloud.replace("##SUN100##","0");
                s_cloud.replace("##SUN50##","1");
                s_cloud.replace("##CLOUDS50##","0");
                s_cloud.replace("##CLOUDS100##","1");
                QString cloud_brightness = QString::number(255-(int)(clouds * 1.27),16);
                while (cloud_brightness.length() < 2) {
                    cloud_brightness = "0" + cloud_brightness;
                }
                while (cloud_brightness.length() > 2) {
                    cloud_brightness.remove(0,1);
                }
                s_cloud.replace("##DARKNESS##",cloud_brightness + cloud_brightness + cloud_brightness);
            } else {
                s_cloud.replace("##SUN100##","0");
                s_cloud.replace("##SUN50##","0");
                s_cloud.replace("##CLOUDS50##","0");
                s_cloud.replace("##CLOUDS100##","1");
                s_cloud.replace("##DARKNESS##","808080");
            }

            /* Rain */
            if (rain < 1e-3) {
                s_cloud.replace("##RAIN25##","0");
                s_cloud.replace("##RAIN50##","0");
                s_cloud.replace("##RAIN100##","0");
            } else if (rain < 0.5) {
                s_cloud.replace("##RAIN25##","1");
                s_cloud.replace("##RAIN50##","0");
                s_cloud.replace("##RAIN100##","0");
            } else if (rain < 30) {
                s_cloud.replace("##RAIN25##","0");
                s_cloud.replace("##RAIN50##","1");
                s_cloud.replace("##RAIN100##","0");
            } else {
                s_cloud.replace("##RAIN25##","0");
                s_cloud.replace("##RAIN50##","0");
                s_cloud.replace("##RAIN100##","1");
            }
        }

        (*iter_clouds)->load(s_cloud.toUtf8());
        (*iter_clouds)->setFixedSize(cloud_width,cloud_width);
        (*iter_clouds)->move(layout_x + (int)floor((part_width-cloud_width)/2),layout_y);

        iter_clouds++;

        QLabel *w_temp_max, *w_temp_min;
        w_temp_max = *iter_temp;
        iter_temp++;
        w_temp_min = *iter_temp;
        iter_temp++;

        if (temp_min > temp_max) {
            w_temp_max->clear();
            w_temp_min->clear();
        } else if (temp_min == temp_max) {
            w_temp_max->clear();
            w_temp_min->setText(QString::number(temp_min,'f',0) + "°");
        } else {
            w_temp_max->setText(QString::number(temp_max,'f',0) + "°");
            w_temp_min->setText(QString::number(temp_min,'f',0) + "°");
        }

        if (layout_y + cloud_width > height() - 2*w_temp_min->sizeHint().height()) {
            layout_y = height() - cloud_width - 2*w_temp_min->sizeHint().height();
        }
        w_temp_max->setFixedSize(w_temp_max->sizeHint());
        w_temp_max->move(layout_x+part_width-2*padding-w_temp_max->width(),h-padding-w_temp_max->height()-w_temp_min->height());
        w_temp_max->setStyleSheet("QLabel { background:none }");
        w_temp_min->setFixedSize(w_temp_min->sizeHint());
        w_temp_min->move(layout_x+part_width-2*padding-w_temp_min->width(),h-padding-w_temp_min->height());
        w_temp_min->setStyleSheet("QLabel { background:none }");


        layout_x += part_width+2;
        lookup_time = lookup_time.addSecs(24*3600);
    }
}
