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
 */

#ifndef QVGOOGLECALENDAR_H
#define QVGOOGLECALENDAR_H

#include <QTimer>
#include <QString>
#include <QLabel>
#include <QSettings>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include "../qvelement.h"

class calendar_entry {
public:
    QString summary, location;
    QDateTime begin, end;
    bool all_day;
};

class QVGoogleCalendar : public QVElement
{
    Q_OBJECT
public:
    explicit QVGoogleCalendar(QDomElement xml_desc, QString container, QWidget *parent = 0);

protected:
    void resizeEvent(QResizeEvent*);

signals:

protected slots:
    void onDataReceived(QNetworkReply*);
    void getAccessToken();
    void getEventList();

private:
    QNetworkAccessManager *network_manager;

    QSettings *registry_settings;
    QString device_code;
    QString access_token;
    QString next_sync_token, next_page_token;
    int refresh_value, refresh_counter;
    QString calendar_id;
    int max_days;

    QMap<QString,calendar_entry> entries;
    QTimer *timer;

    QList<QLabel*> w_summary, w_date, w_location;
    QLabel *w_auth;

    void do_getEventList(bool);
    void showCalendarEvents();

    static const int padding = 8;
};




#endif // QVGOOGLECALENDAR_H
