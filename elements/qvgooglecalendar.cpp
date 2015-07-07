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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * EXCEPTION: The Google API keys contained in this file are NOT covered by the GPL.
 * For any derived work, you MUST generate your own keys.
 */

#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include "qvgooglecalendar.h"

#define client_id QStringLiteral("648870426942-gtm2uiulinqh8li675co0hvj77jn0rr7.apps.googleusercontent.com")
#define client_key QStringLiteral("7k4tvhjLtU5VBN7-KxCcOqr-")
#define scope QStringLiteral("https://www.googleapis.com/auth/calendar.readonly")


QVGoogleCalendar::QVGoogleCalendar(QDomElement xml_desc, QString container, QWidget *parent) :
    QVElement(xml_desc,container,parent)
{
    if (h < 2) {
        h = 4;
    }
    w = 4;

    registry_settings = new QSettings("kalassi","QVisu");
    timer = new QTimer();
    timer->setInterval(100);
    timer->setSingleShot(true);

    QDomElement e_calendar_id = xml_desc.firstChildElement("calendar-id");
    if (e_calendar_id.isNull()) {
        qDebug() << "Missing calendar id";
        return;
    }
    calendar_id = e_calendar_id.text();

    max_days = 365;
    QDomElement e_max_days = xml_desc.firstChildElement("max_days");
    if (!e_max_days.isNull()) {
        bool ok;
        max_days = e_max_days.text().toInt(&ok);
        if (!ok) {
            max_days = 365;
        }
    }

    network_manager = new QNetworkAccessManager();
    QObject::connect(network_manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(onDataReceived(QNetworkReply*)));

    QNetworkRequest request;
    QString body;
    QVariant v_refresh_token = registry_settings->value(calendar_id + "/google_refresh_token");

    if (v_refresh_token == QVariant()) {
        request.setUrl(QUrl("https://accounts.google.com/o/oauth2/device/code"));
        body = QString("client_id=" + client_id + "&scope=" + scope);
    } else {
        request.setUrl(QUrl("https://www.googleapis.com/oauth2/v3/token"));
        body = QString("client_id=" + client_id + "&client_secret=" + client_key + "&refresh_token=" + v_refresh_token.toString() + "&grant_type=refresh_token");
    }
    request.setHeader(QNetworkRequest::ContentTypeHeader,QStringLiteral("application/x-www-form-urlencoded"));
    network_manager->post(request,body.toUtf8());
    qDebug() << request.url().toString() << "\n" << body;
	
    w_auth.hide();
}

void QVGoogleCalendar::onDataReceived(QNetworkReply *reply) {
    qDebug() << "CALENDAR 2";
    if (reply->isRunning()) {
        qDebug() << "Running reply";
        return;
    }

    QByteArray json = reply->readAll();
    qDebug() << json;

    QJsonParseError result;
    QJsonDocument jsd = QJsonDocument::fromJson(json,&result);
    if (result.error != QJsonParseError::NoError) {
        qDebug() << "Incorrect JSON" << result.errorString();
        return;
    }
    QJsonObject jso = jsd.object();

    if (jso.contains("error")
            && ((jso.value("error").toString() == "invalid_request") || (jso.value("error").toString() == "invalid_grant"))) {
        /* Invalid refresh token, start all over again */
        QNetworkRequest request;
        request.setUrl(QUrl("https://accounts.google.com/o/oauth2/device/code"));
        QString body = QString("client_id=" + client_id + "&scope=" + scope);
        network_manager->post(request,body.toUtf8());
        return;
    }

    if (jso.contains("verification_url")) {  /* First contact, receiving user code and verification URL */
        QString auth_url = jso.value("verification_url").toString();
        if (auth_url.isEmpty()) {
            auth_url = "https://www.google.com/device";
        }
        w_auth.setText("<center><b>Authentication required</b><p/>Go to " + auth_url + "<br/>and enter this code:<p/><font size=\"5\">" + jso.value("user_code").toString() + "</font></center>");
        w_auth.show();
        w_auth.raise();

        qDebug() << "First contact, verify " << jso.value("user_code").toString();
        device_code = jso.value("device_code").toString();
        QObject::disconnect(timer,0,0,0);
        int interval = jso.value("interval").toInt();
        if (interval < 5) {
            refresh_value = 100;
        } else {
            refresh_value = interval * 20;
        }
        QObject::connect(timer,SIGNAL(timeout()),this,SLOT(getAccessToken()));
        refresh_counter = refresh_value;
        timer->start();
        return;
    }
    if (jso.contains("error") && (jso.value("error") == "authorization_pending")) {
        refresh_counter = refresh_value;
        timer->start();
        return;
    }
    if (jso.contains("access_token")) {
        w_auth.hide();
        access_token = jso.value("access_token").toString();
        if (jso.contains("refresh_token")) {
            registry_settings->setValue(calendar_id + "/google_refresh_token",jso.value("refresh_token").toString());
            registry_settings->sync();
        }
        QObject::disconnect(timer,0,0,0);
        QObject::connect(timer,SIGNAL(timeout()),this,SLOT(getEventList()));
        do_getEventList(true);
    }

    if (jso.contains("kind") && (jso.value("kind") == "calendar#events")) {
        if (!jso.value("nextPageToken").isNull()) {
            next_page_token = jso.value("nextPageToken").toString();
        } else {
            next_page_token.clear();
            if (!jso.value("nextSyncToken").isNull()) {
                next_sync_token = jso.value("nextSyncToken").toString();
            }
        }

        QJsonArray j_items = jso.value("items").toArray();

        for (int n=0; n< j_items.count(); n++) {
            QJsonObject o_item = j_items.at(n).toObject();
            QString id = o_item.value("id").toString();
//            qDebug() << o_item.value("status").toString() << o_item.value("summary").toString() << id << entries.contains(id);
            if (o_item.value("status") == "cancelled") {
                if (entries.contains(id)) {
                    entries.remove(id);
                }
                continue;
            }
            QDateTime start_time, end_time;
            bool all_day;

            QJsonObject o_begin = o_item.value("start").toObject();
            if (o_begin.contains("date")) {
                start_time = QDateTime::fromString(o_begin.value("date").toString(),"yyyy-MM-dd");
                all_day  = true;
            } else if (o_begin.contains("dateTime")) {
                start_time = QDateTime::fromString(o_begin.value("dateTime").toString().left(19),"yyyy-MM-ddThh:mm:ss");
                all_day = false;
            }
            start_time.setTimeZone(QTimeZone(0));

            QJsonObject o_end = o_item.value("end").toObject();
            if (o_end.contains("date")) {
                end_time = QDateTime::fromString(o_end.value("date").toString(),"yyyy-MM-dd");
            } else if (o_end.contains("dateTime")) {
                end_time = QDateTime::fromString(o_end.value("dateTime").toString().left(19),"yyyy-MM-ddThh:mm:ss");
            }
            end_time.setTimeZone(QTimeZone(0));

            if (!start_time.isValid() || !end_time.isValid()
                    || (start_time.toMSecsSinceEpoch() > QDateTime::currentDateTime().addDays(max_days).toMSecsSinceEpoch())
                    || (end_time.toMSecsSinceEpoch() < QDateTime::currentDateTime().toMSecsSinceEpoch())) {
                continue;
            }

            if (!entries.contains(id)) {
                entries.insert(id,calendar_entry());
            }

            entries[id].begin = start_time;
            entries[id].end = end_time;
            entries[id].all_day = all_day;
            entries[id].location = o_item.value("location").toString();
            entries[id].summary = o_item.value("summary").toString();
        }


        showCalendarEvents();
        if (next_page_token.isEmpty()) {
            refresh_value = 300;
        } else {
            refresh_value = 0;
        }
        refresh_counter = refresh_value;
        timer->start();
    }

//    qDebug() << reply->readAll();
}

void QVGoogleCalendar::getAccessToken() {
    if (refresh_counter > 0) {
        refresh_counter--;
        timer->start();
        return;
    }

    QNetworkRequest request;
    QString body;

    request.setUrl(QUrl("https://www.googleapis.com/oauth2/v3/token"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QStringLiteral("application/x-www-form-urlencoded"));

    body = QStringLiteral("client_id=") + client_id +
            "&client_secret=" + client_key +
            "&code=" + device_code +
            "&grant_type=http://oauth.net/grant_type/device/1.0";
    network_manager->post(request,body.toUtf8());
}

void QVGoogleCalendar::do_getEventList(bool initial_request = false) {
    QNetworkRequest request;
    QString url;
    url = "https://www.googleapis.com/calendar/v3/calendars/" + calendar_id + "/events";
    url += "?access_token=" + access_token;
    if (initial_request || (next_sync_token.isEmpty() && next_page_token.isEmpty())) {
        url += "&singleEvents=true";
        url += "&timeZone=UTC";
        url += "&showDeleted=True";
        url += "&timeMin=" + QDateTime::currentDateTime().toUTC().toString("yyyy-MM-ddThh:mm:ss%2B00:00");
        if (max_days > 0) {
            QDateTime endtime = QDateTime::currentDateTime().addDays(max_days);
            url += "&timeMax=" + endtime.toUTC().toString("yyyy-MM-ddThh:mm:ss%2B00:00");
        }
    } else if (!next_page_token.isEmpty()) {
        url += "&nextPageToken=" + next_page_token;
        next_page_token.clear();
    } else if (!next_sync_token.isEmpty()) {
        url += "&nextSyncToken=" + next_sync_token;
    }
    request.setUrl(QUrl(url));
    network_manager->get(request);
    qDebug() << url;
}


void QVGoogleCalendar::getEventList() {
    if (refresh_counter > 0) {
        refresh_counter--;
        timer->start();
        return;
    }

    do_getEventList(false);
}

void QVGoogleCalendar::showCalendarEvents() {
    QMultiMap<QDateTime,QString> events_by_date;

    for (QMap<QString,calendar_entry>::iterator iter = entries.begin(); iter != entries.end(); iter++) {
        QString key = iter.key();
        QString key2;

        QRegularExpression re("(.+)_[0-9]{8}");
        QRegularExpressionMatch match = re.match(key);
        if (match.hasMatch()) {
            key2 = match.captured(1);
        }

        if (!events_by_date.contains((*iter).begin,key) && !events_by_date.contains((*iter).begin,key2)) {
            events_by_date.insert((*iter).begin,key);
        }
    }

    QList<QDateTime> keylist = events_by_date.keys().toSet().toList();
    std::sort(keylist.begin(),keylist.end());


    if (keylist.count() == 0) {
        for (int n=0; n < w_summary.count(); n++) {
            w_summary[n]->clear();
            w_date[n]->clear();
            w_location[n]->clear();
        }
        return;
    }

    int line = 0;
    for (QList<QDateTime>::iterator iter = keylist.begin(); iter != keylist.end(); iter++) {
        QList<QString> iter_keys = events_by_date.values(*iter);

        if (line >= w_summary.count()) {
            break;
        }
        for (int n=0; n < iter_keys.count(); n++) {
//            qDebug() << n << line << iter_keys[n] << entries[iter_keys[n]].summary << entries[iter_keys[n]].begin;
            if (line >= w_summary.count()) {
                break;
            }
            w_summary[line]->setText(entries[iter_keys[n]].summary);
            if (entries[iter_keys[n]].all_day) {
                w_date[line]->setText(entries[iter_keys[n]].begin.toLocalTime().toString("ddd dd.MM."));
            } else {
                w_date[line]->setText(entries[iter_keys[n]].begin.toLocalTime().toString("ddd dd.MM. hh:mm"));
            }
            w_location[line]->setText(entries[iter_keys[n]].location);
            line++;
        }
    }
}

void QVGoogleCalendar::resizeEvent(QResizeEvent *e) {
    if (e->oldSize().height() == height()) {
        return;
    }
    qDebug() << "calendar resize";

    QLabel l1("W");
    QLabel l2("W");
    l2.setObjectName("mini");
    int line_height = l1.sizeHint().height();
    int entry_height = l1.sizeHint().height() + l2.sizeHint().height() + padding;

    int lines = (int)floor(height()/entry_height);
    while (w_summary.count() < lines) {
        w_summary.append(new QLabel(this));
        w_date.append(new QLabel(this));
        w_date.last()->setObjectName("mini");
        w_date.last()->setStyleSheet("QLabel { font:bold }");
        w_location.append(new QLabel(this));
        w_location.last()->setObjectName("mini");
    }

    for (int n=0; n < w_summary.count(); n++) {
        if (n < lines) {
            w_summary[n]->setFixedWidth(width());
            w_summary[n]->move(ofs_x(),n*entry_height+ofs_y());
            w_summary[n]->show();
            w_date[n]->setFixedWidth((int)(width()/3));
            w_date[n]->move(ofs_x(),n*entry_height+line_height+ofs_y());
            w_date[n]->show();
            w_location[n]->setFixedWidth((int)(width()*2/3)-padding);
            w_location[n]->move(ofs_x()+(int)(width()/3)+padding,n*entry_height+line_height+ofs_y());
            w_location[n]->show();
        } else {
            w_summary[n]->hide();
            w_date[n]->hide();
            w_location[n]->hide();
        }
    }

    w_auth.setFixedSize(size());
    w_auth.move(0,0);
    w_auth.setStyleSheet("QLabel { background-color:#ffffff; color:#000000 }");
}
