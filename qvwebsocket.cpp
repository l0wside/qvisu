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

#include <QHostAddress>
#include <QTime>
#include <QTimer>
#include <QCryptographicHash>
#include <stdint.h>
#include "qvwebsocket.h"

QVWebsocket::QVWebsocket(QObject *parent) :
    QObject(parent)
{
    socket = new QTcpSocket();
    status = socketClosed;
    open_timer.setSingleShot(true);
    open_timer.setInterval(5000);
    QObject::connect(&open_timer,SIGNAL(timeout()),this,SLOT(close()));

    close_timer.setSingleShot(true);
    close_timer.setInterval(1000);
    QObject::connect(&close_timer,SIGNAL(timeout()),this,SLOT(close()));
}

QVWebsocket::~QVWebsocket() {
    if (socket->isOpen()) {
        socket->close();
        delete socket;
        status = socketClosed;
    }
}

void QVWebsocket::open(QUrl url) {
    qDebug() << "opening WS " << url;
    if (!url.isValid()) {
        emit disconnected();
        return;
    }

    host = url.host();
    int port = url.port();
    if (port < 0) {
        port = 80;
    }
    QString scheme = url.scheme();

    if (scheme != QStringLiteral("ws")) {
        emit disconnected();
        return;
    }

    path = url.path();

    QObject::connect(socket,SIGNAL(connected()),this,SLOT(socketConnected()),Qt::QueuedConnection);
    QObject::connect(socket,SIGNAL(disconnected()),this,SLOT(socketDisconnected()),Qt::QueuedConnection);
    QObject::connect(socket,SIGNAL(readyRead()),this,SLOT(socketReadyRead()),Qt::QueuedConnection);
    QObject::connect(socket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(onSocketError(QAbstractSocket::SocketError)));
    inputBuffer.clear();
    qDebug() << "calling connectToHost()" << host << port;
    open_timer.start();
    socket->connectToHost(host,port);
}

void QVWebsocket::shutdown() {
    qDebug() << "Websocket shutdown";
    if (status != wsEstablished) {
        return;
    }
    status = wsClosing;

    QByteArray txMessage;

    txMessage.append((uint8_t)0x88);
    socket->write(txMessage);
    close_timer.start();
}

void QVWebsocket::close() {
    qDebug() << "Websocket close";
    emit disconnected();
    socket->close();
    status = socketClosed;
}


void QVWebsocket::sendTextMessage(QString message) {
//    qDebug() << "sendTextMessage " << message;
    if (status != wsEstablished) {
        return;
    }

    if (log_file.isOpen()) {
        log_stream << "<< " + message + "\n";
    }

    QByteArray txMessage;

    txMessage.append((uint8_t)0x81);
    if (message.length() < 126) {
        txMessage.append(((uint8_t)message.length()) | 0x80);
    } else if (message.length() < 65536) {
        uint16_t len = message.length();
        txMessage.append((uint8_t)0xFE);
        txMessage.append((uint8_t)(len >> 8));
        txMessage.append((uint8_t)(len & 0xFF));
    } else {
        return; /* > 64k not supported */
    }
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());
    uint8_t mask[4];
    mask[0] = (uint8_t)(qrand()); txMessage.append(mask[0]);
    mask[1] = (uint8_t)(qrand()); txMessage.append(mask[1]);
    mask[2] = (uint8_t)(qrand()); txMessage.append(mask[2]);
    mask[3] = (uint8_t)(qrand()); txMessage.append(mask[3]);
    QByteArray msgArray = QByteArray(message.toUtf8());
    for (int n = 0; n < msgArray.count(); n++) {
        txMessage.append(msgArray.at(n) ^ mask[n%4]);
    }
    socket->write(txMessage);
}

void QVWebsocket::socketConnected() {
    qDebug() << "Socket Connected";
    key.clear();
    open_timer.stop();
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());
    for (int n=0; n < 16; n++) {
        int r = abs(qrand()) % 62;
        if (r < 10) {
            key.append('0' + r);
        } else if (r < 36) {
            key.append('A' + (r - 10));
        } else {
            key.append('a' + (r-36));
        }
    }
    key = QByteArray().append(key).toBase64();

    QString get = QString("GET /").append(path).append(" HTTP/1.1\r\n");
    QString hoststr = QString("Host: ").append(host).append("\r\n");
    QString upgrade = QStringLiteral("Upgrade: websocket\r\n");
    QString connection = QStringLiteral("Connection: Upgrade\r\n");
    QString keystr = QStringLiteral("Sec-WebSocket-Key: ").append(key).append("\r\n");
    QString ver = QStringLiteral("Sec-WebSocket-Version: 13\r\n");
    QString terminator = QStringLiteral("\r\n");

    socket->write((get + hoststr + upgrade + connection + keystr + ver + terminator).toUtf8());
    status = awaitingUpgrade;
}

void QVWebsocket::socketDisconnected() {
    open_timer.stop();
    qDebug() << "Socket Disconnected";
    if (status != socketClosed) {
        emit disconnected();
        status = socketClosed;
        close_timer.stop();
    }
}


void QVWebsocket::socketReadyRead() {
    qDebug() << "pre: " << inputBuffer.length();
    inputBuffer.append(socket->readAll());
    qDebug() << "post: " << inputBuffer.length();

    if (inputBuffer.length() > 65536) {
        qDebug() << "Input buffer overflow";
        socket->close();
        status = socketClosed;
        return;
    }

    if (status == awaitingUpgrade) {
        int headerlen = QString(inputBuffer).indexOf("\r\n\r\n");
        if (headerlen <= 0) {
            return;
        }
        QString header = QString(inputBuffer).left(headerlen+2);
        inputBuffer = inputBuffer.mid(headerlen+4);
        if (!header.toUpper().startsWith("HTTP/1.1 101")) {
            socket->close();
            status = socketClosed;
            return;
        }
        if ((!header.toLower().contains("upgrade: websocket\r\n")) || (!header.toLower().contains("connection: upgrade\r\n"))) {
            socket->close();
            status = socketClosed;
            return;
        }
        int key_pos = header.indexOf("Sec-WebSocket-Accept: ");
        if (key_pos <= 0) {
            socket->close();
            status = socketClosed;
            return;
        }
        key_pos += QStringLiteral("Sec-WebSocket-Accept: ").length();
        QString retkey = header.mid(key_pos);
        int key_len = retkey.indexOf("\r\n");
        if (key_len > 0) {
            retkey = retkey.left(key_len);
        }

        QString magic_guid("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
        QString check_key = QCryptographicHash::hash(QByteArray().append(key+magic_guid),QCryptographicHash::Sha1).toBase64();
        qDebug() << "Keys" << retkey << check_key;
        if (check_key != retkey) {
            socket->close();
            status = socketClosed;
            return;
        }

        status = wsEstablished;
        emit connected();

        return;
    }

    if (status == wsEstablished) {
        while (status == wsEstablished) {
            inputBuffer.append(socket->readAll());
            qDebug() << "Sock open " << socket->isOpen() << "Loop, avail in sock: " <<         socket->bytesAvailable();
            if (inputBuffer.length() < 2) {
                return;
            }
            bool fin = ((inputBuffer.at(0) & 0x80) > 0);
            uint8_t opcode = inputBuffer.at(0) & 0x03;
            uint8_t headerlen = 2;
//            qDebug() << "opcode " << opcode;

            if (!fin) {
                /* Concatenated and binary messages not supported */
                socket->close();
                status = socketClosed;
            }

            long payload = inputBuffer.at(1) & 0x7F;
            if (payload == 126) {
                headerlen = 4;
                if (inputBuffer.length() < 4) {
                    return;
                }
                payload = (unsigned int)((unsigned char)inputBuffer.at(2)) * 256 + (unsigned int)((unsigned char)inputBuffer.at(3));
                if (inputBuffer.length() < (payload + 4)) {
                    return;
                }
            } else if (payload == 127) {
                headerlen = 6;
                if (inputBuffer.length() < 6) {
                    return;
                }
                payload = (unsigned int)((unsigned char)inputBuffer.at(2)) * 0x1000000 + (unsigned int)((unsigned char)inputBuffer.at(3)) * 0x10000 + (unsigned int)((unsigned char)inputBuffer.at(4)) * 0x100 + (unsigned int)((unsigned char)inputBuffer.at(5));
                if (inputBuffer.length() < (payload + 6)) {
                    return;
                }
            } else {
                if (inputBuffer.length() < (payload + 2)) {
                    return;
                }
            }

            qDebug() << "rest: " << headerlen << payload << inputBuffer.length();

            /* We have at least one complete frame */
            inputBuffer.remove(0,headerlen);
            QByteArray pong;
            switch (opcode) {
            case 0x01: /* Text frame */
                if (log_file.isOpen()) {
                    log_stream << ">> " + inputBuffer.left(payload) + "\n";
                }
                emit textMessageReceived(QString(inputBuffer.left(payload)));
                break;
            case 0x02: /* Binary frame, ignored */
                qDebug() << "Binary frame";
                break;
            case 0x08: /* Connection close */
                if (log_file.isOpen()) {
                    log_stream << "CLOSED\n";
                }
                socket->close();
                status = socketClosed;
                inputBuffer.clear();
                return;
            case 0x09: /* Ping */
                pong.append((char)0x8A);
                pong.append((char)0x00);
                socket->write(pong);
                break;
            default:
                break;
            }

            inputBuffer.remove(0,payload);
            qDebug() << "payload was " << payload << " remaining len now " << inputBuffer.length();
        }
    }
    qDebug() << "done with input buffer";
}

void QVWebsocket::startLogging(QString filename) {
    if (log_file.isOpen()) {
        log_file.close();
    }
    log_file.setFileName(filename);
    if (log_file.open(QIODevice::WriteOnly)) {
        log_stream.setDevice(&log_file);
    }
}

void QVWebsocket::stopLogging() {
    if (log_file.isOpen()) {
        log_file.close();
    }
}

#ifdef USE_THREADS
void QVWebsocket::setThread(QThread *thread) {
    socket->moveToThread(thread);
}
#endif
