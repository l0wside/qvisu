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

#include "qvdriver.h"
#include <QJsonObject>

QVDriver::QVDriver(QDomElement xml_data, QStringList item_list, QObject *parent) :
    QObject(parent)
{
    QDomElement conn_elem = xml_data.firstChildElement("connection");
    if (conn_elem.isNull()) {
        qDebug() << "connection element not found in XML";
        return;
    }
    this->item_list = QStringList(item_list);

    QString conn_type = conn_elem.attribute("type");

    /* Websocket / SmartVisu */
    if (conn_type == "smartvisu") {
        webSocket = new QVWebsocket();

#ifdef USE_THREADS
        socket_thread = new QThread();
#endif

        closing = false;
        QString s_url = QStringLiteral("ws://").append(conn_elem.text());
        webSocketUrl = QUrl(s_url);
        qDebug() << webSocketUrl;
        QObject::connect(webSocket,SIGNAL(connected()),this,SLOT(onConnected()));
        QObject::connect(webSocket,SIGNAL(disconnected()),this,SLOT(onDisconnected()));
        QObject::connect(webSocket,SIGNAL(textMessageReceived(QString)),this,SLOT(onTextMessageReceived(QString)));
        QObject::connect(this,SIGNAL(sendTextMessage(QString)),webSocket,SLOT(sendTextMessage(QString)));

        if (conn_elem.hasAttribute("log")) {
            webSocket->startLogging(conn_elem.attribute("log"));
        }

        webSocket->open(webSocketUrl);
        emit valueChanged("#driver-statustext","Connecting");
        emit valueChanged("#driver-status","0");
#ifdef USE_THREADS
        webSocket->moveToThread(socket_thread);
        webSocket->setThread(socket_thread);
        socket_thread->start();
#endif
        reopen_timer = new QTimer(this);
        reopen_timer->setSingleShot(true);
        reopen_timer->setInterval(1000);
        QObject::connect(reopen_timer,SIGNAL(timeout()),this,SLOT(onReopenTimer()));

        return;
    }


    qDebug() << "Invalid or missing connection type " << conn_type;
}

QVDriver::~QVDriver() {
    this->shutdown();
}

void QVDriver::shutdown() {
    emit valueChanged("#driver-statustext","Offline");
    emit valueChanged("#driver-status","0");
    closing = true;
    webSocket->shutdown();
}

void QVDriver::onValueModified(QString id, QString value) {
//    qDebug() << "Driver: " << id << value;
    if (closing) {
        qDebug() << "closing";
        return;
    }
    emit valueChanged(id,value);
    emit sendTextMessage(QString("{\"cmd\":\"item\",\"id\":\"")
                              .append(id)
                              .append(QString("\",\"val\":\""))
                              .append(value)
                              .append(QString("\"}")));
}

void QVDriver::onSeriesRequest(QString item, QString series_type, QString start) {
    qDebug() << "Driver for series: " << item;
    if (closing) {
        qDebug() << "closing";
        return;
    }
    QString request = QString("{\"cmd\":\"series\",") +
            QString("\"item\":\"") + item + QString("\",") +
            QString("\"series\":\"") + series_type + QString("\",") +
            QString("\"start\":\"") + start + QString("\"}");
#if 0
    series_list.append(request);
    if (series_list.count() == 1) {
        emit sendTextMessage(series_list.first());
    }
#else
    emit sendTextMessage(request);
#endif
}

void QVDriver::onConnected() {
    qDebug() << "Websocket Connect";
    emit sendTextMessage(QString("{\"cmd\":\"proto\",\"ver\":3}"));
    QString monitor_msg = QString("{\"cmd\":\"monitor\",\"items\":[");
    for (QStringList::iterator iter = item_list.begin(); iter != item_list.end(); iter++) {
        monitor_msg = monitor_msg.append("\"").append(*iter).append("\"");
        if ((iter+1) != item_list.end()) {
            monitor_msg = monitor_msg.append(",");
        }
    }
    monitor_msg = monitor_msg.append("]}");

    emit sendTextMessage(monitor_msg);
    emit initialized();
    qDebug() << "QVDriver::onConnected done";
}


void QVDriver::onDisconnected() {
    qDebug() << "Websocket Disconnect";
    if (!closing) {
        reopen_timer->start();
    }
}

void QVDriver::onReopenTimer() {
    if (closing) {
        return;
    }
    qDebug() << "Reopening";
    emit valueChanged("#driver-statustext","Reconnecting");
    emit valueChanged("#driver-status","0");
    webSocket->open(webSocketUrl);
}

void QVDriver::onTextMessageReceived(QString message) {
    qDebug() << "Received in driver " << message;

    QJsonDocument jsd = QJsonDocument::fromJson(QByteArray(message.toUtf8()));
    QJsonObject jso = jsd.object();
//    qDebug() << "Object is " << jsd;

    QJsonValue jcmd = jso.take("cmd");
//    qDebug() << "Command is " << jcmd << " -> " << jcmd.toString();
    if (jcmd.isUndefined() || !jcmd.isString())  {
        return;
    }


    if (jcmd.toString() == QString("proto")) {
        QJsonValue proto = jso.take("ver");
//        qDebug() << proto;
        if (proto.isUndefined() || !proto.isDouble() || ((int)(proto.toDouble()) != 3)) {
            qDebug() << "Invalid protocol version";
            shutdown();
            return;
        }
        emit valueChanged("#driver-statustext","Connected");
        emit valueChanged("#driver-status","1");
    }
    if (jcmd.toString() == QString("item")) {
        QJsonValue itemdata = jso.take("items");
        if (!itemdata.isArray()) {
            qDebug() << "Invalid format for items" ;
            return;
        }
        QJsonArray itemlist = itemdata.toArray();
        for (QJsonArray::iterator iter = itemlist.begin(); iter != itemlist.end(); iter++) {
            QJsonValue item_data = *iter;
//            qDebug() << item_data;
            if (!item_data.isArray()) {
                qDebug() << "Invalid format for items (array expected)";
                return;
            }
            QJsonArray item_data_arr = item_data.toArray();
            if (item_data_arr.count() < 2) {
                qDebug() << "Invalid format for items (array too short)";
                return;
            }
            QString item = item_data_arr.at(0).toString();
            QString value;
            if (item_data_arr.at(1).isBool()) {
                bool val = item_data_arr.at(1).toBool();
                value = val?"1":"0";
            } else if (item_data_arr.at(1).isDouble()) {
                double val = item_data_arr.at(1).toDouble();
                value = QString::number(val);
            } else if (item_data_arr.at(1).isString()) {
                value = item_data_arr.at(1).toString();
            } else {
                return;
            }
//            qDebug() << "emit " << item << value;
            emit valueChanged(item,value);
        }
        return;
    }

    if (jcmd.toString() == QString("series")) {
        qDebug() << "Series";
        qDebug() << message;

        /* If more requests for series are pending, send next request.
         * Ugly code, does not check which series was actually received.
         * Will hang if server does not respond to a series request.
         * FIXME
         */
        if (series_list.count() > 0) {
            series_list.pop_front();
        }
        if (series_list.count() > 0) {
            emit sendTextMessage(series_list.first());
        }

        QJsonValue jitem = jso.take("sid");
        QString item = jitem.toString();
        int item_len = item.indexOf('|');
        if (item_len <= 0) {
            return;
        }
        item = item.left(item_len);
        qDebug() << "series item " << item;
        QJsonValue jpairs = jso.take("series");
        if (!jpairs.isArray()) {
            return;
        }
        QJsonArray jarray = jpairs.toArray();
        QMap<double,double> map;
        for (QJsonArray::iterator iter = jarray.begin(); iter != jarray.end(); iter++) {
            if (!(*iter).isArray()) {
                return;
            }

            QJsonArray jpair = (*iter).toArray();
            if (jpair.count() != 2) {
                return;
            }

            QJsonValue t = jpair.at(0);
            QJsonValue v = jpair.at(1);
            double td = t.toDouble(-1e9);
            double vd = v.toDouble(-1e9);
            if (td+vd < -9e-8) {
                return;
            }

            map.insert(td,vd);
        }
        qDebug() << "emitting received series " << item << map.count();
        emit seriesReceived(item,map);
    }

}
