/* QVisu websockets (c) 2015 Maximilian Gauger mgauger@kalassi.de
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QVWEBSOCKET_H
#define QVWEBSOCKET_H

//#define USE_THREADS

#include <QObject>
#include <QTcpSocket>
#include <QUrl>
#include <QTimer>
#include <QFile>
/** Provides websocket functionality. The Qt5 websocket module seems to be incompatible with sh.py.
 * This class is used by QVDriver and should not be accessed directly.
 * Only text messages are supported.
 */

class QVWebsocket : public QObject
{
    Q_OBJECT
public:
    explicit QVWebsocket(QObject *parent = 0);
    ~QVWebsocket();

signals:
    /** Emitted when the websocket connection is established. */
    void connected();
    /** Emitted when the websocket connection is terminated. */
    void disconnected();
    /** Emitted when a text message was received. */
    void textMessageReceived(QString);

public slots:
    /** Open a websocket URL. The URL must start with ws:// */
    void open(QUrl);
    /** Hard close of the websocket connection. shutdown() should be used instead. */
    void close();
    /** Send a termination message to the other party, requesting it to shut down the connection */
    void shutdown();
    /** Send a text message to the other party */
    void sendTextMessage(QString);
    void startLogging(QString);
    void stopLogging();
#ifdef USE_THREADS
    void setThread(QThread*);
#endif

private slots:
    /** Called when the underlying TCP socket is connected */
    void socketConnected();
    /** Called when the underlying TCP socket is connected */
    void socketDisconnected();
    /** Called when the underlying TCP socket delivers data */
    void socketReadyRead();
    void onSocketError(QAbstractSocket::SocketError err) {
        qDebug() << err;
    }

private:
    QTcpSocket *socket;
    QString host, path, key;
    QByteArray inputBuffer;
    QTimer open_timer, close_timer;
    QFile log_file;
    QTextStream log_stream;

    enum {
        socketClosed,
        socketOpening,
        awaitingUpgrade,
        wsEstablished,
        wsClosing
    } status;


};

#endif // QVWEBSOCKET_H
