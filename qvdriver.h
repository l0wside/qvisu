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

#ifndef QVDRIVER_H
#define QVDRIVER_H

#include <QObject>
#include <QTimer>
#include <QThread>
#include <QtXml>
#include <QStringList>
#include "qvwebsocket.h"

class QVDriver : public QObject
{
    Q_OBJECT
public:
    explicit QVDriver(QDomElement xml_data, QStringList item_list, QObject *parent = 0);
    ~QVDriver();

signals:
    void initialized();
    void valueChanged(QString,QString);
    void seriesReceived(QString,QMap<double,double>);
    void sendTextMessage(QString);

public slots:
    void onValueModified(QString,QString);
    void onSeriesRequest(QString item, QString series_type, QString start);

private slots:
    void onConnected();
    void onDisconnected();
    void onReopenTimer();
    void onTextMessageReceived(QString message);
    void shutdown();

private:
    QVWebsocket *webSocket;
    QUrl webSocketUrl;
#ifdef USE_THREADS
    QThread *socket_thread;
#endif

    QStringList item_list;
    QStringList series_list;
    bool closing;
    QTimer *reopen_timer;
};

#endif // QVDRIVER_H
