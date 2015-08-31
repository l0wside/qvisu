#include <QDir>
#include <QTcpSocket>
#include <QHostInfo>
#include "qvdeployserver.h"

QVDeployServer::QVDeployServer(QString appdir) :
    QObject()
{
    this->appdir = appdir;

    udp_socket = new QUdpSocket();
    udp_socket->bind(QHostAddress::AnyIPv4, 21024, QUdpSocket::ShareAddress);
    udp_socket->joinMulticastGroup(QHostAddress("239.192.67.184"));
    QObject::connect(udp_socket,SIGNAL(readyRead()),this,SLOT(onUdpReceived()));
    QObject::connect(udp_socket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(onSocketError(QAbstractSocket::SocketError)));

    tcp_socket = 0;
    tcp_server = new QTcpServer();
    tcp_server->listen(QHostAddress::AnyIPv4, 21024);
    QObject::connect(tcp_server,SIGNAL(newConnection()),this,SLOT(onTcpIncoming()));
}

QVDeployServer::~QVDeployServer() {
    delete udp_socket;
}

void QVDeployServer::onUdpReceived() {
    QHostAddress sender;
    char sock_data[1024];
    int sock_data_len = udp_socket->pendingDatagramSize();
    udp_socket->readDatagram(sock_data, 1024, &sender);
    sock_data[sock_data_len] = 0;

    qDebug() << "oUR" << QString(sock_data);

    QString packet(sock_data);
    if (!packet.startsWith("qvd:query")) {
        return;
    }

    QString reply = QString("qvd:reply#") + QHostInfo::localHostName();
    qDebug() << "sending" << sender << udp_socket->writeDatagram(reply.toUtf8(),sender,21023);

}

void QVDeployServer::onSocketError(QAbstractSocket::SocketError err) {
    qDebug() << "UDP socket error" << err;
}

void QVDeployServer::onTcpIncoming() {
    qDebug() << "TCP connection incoming";

    if (tcp_socket != 0) {
        if (tcp_server->nextPendingConnection() != 0) {
            tcp_server->nextPendingConnection()->close();
            return;
        }
    }
    qDebug() << "pending 1" << tcp_server->hasPendingConnections();
    tcp_socket = tcp_server->nextPendingConnection();
    qDebug() << "pending 2" << tcp_server->hasPendingConnections();
    qDebug() << tcp_socket;
    tcp_socket->open(QAbstractSocket::ReadOnly);
    QObject::connect(tcp_socket,SIGNAL(readyRead()),this,SLOT(onTcpRead()));
    QObject::connect(tcp_socket,SIGNAL(disconnected()),this,SLOT(onTcpDisconnected()));

    qDebug() << "data clear";
    deploy_data.clear();
    qDebug() << "incoming done";
}

void QVDeployServer::onTcpRead() {
    qDebug() << "oTR";

    deploy_data.append(tcp_socket->readAll());
    if (deploy_data.length() < 10) {
        return;
    }
    if (!deploy_data.startsWith("qvd-deploy\n")) {
        qWarning("Deploy server: Incorrect data");
        deploy_data.clear();
        tcp_socket->close();
        return;
    }
}

void QVDeployServer::onTcpDisconnected() {
    qDebug() << "Deploy: TCP disconnected, starting evaluation";
    tcp_socket->deleteLater();
    tcp_socket = 0;
    qDebug() << "=====================\n" << deploy_data;

    if (!deploy_data.startsWith("qvd-deploy\n")) {
        qWarning("Deploy server: Incorrect data");
        deploy_data.clear();
        return;
    }

    deploy_data.remove(0,QString("qvd-deploy\n").length());
    while (deploy_data.startsWith("##file\n")) {
        deploy_data.remove(0,QString("##file\n").length());
        int idx = deploy_data.indexOf("\n");
        if (idx <= 0) {
            qDebug() << "filename not found";
            break;
        }
        QString filename = deploy_data.left(idx);
        qDebug() << filename;
        idx = deploy_data.indexOf("\n");
        if (idx <= 0) {
            qDebug() << "length not found";
            break;
        }
        deploy_data.remove(0,idx+1);

        idx = deploy_data.indexOf("\n");
        QString s_len = deploy_data.left(idx);
        bool ok;
        int len = s_len.toInt(&ok);
        qDebug() << "len" << s_len << len << ok;
        if (!ok || (len <= 0)) {
            break;
        }
        deploy_data.remove(0,idx+1);
        if (deploy_data.length() < len) {
            qDebug() << "not enough data";
            break;
        }

        filename = appdir + "/" + filename;
        qDebug() << "writing to" << filename;
        QDir(appdir).mkpath(appdir);
        if (!QFileInfo(filename).absoluteDir().exists()) {
            QDir(appdir).mkpath(QFileInfo(filename).dir().dirName());
        }
        QFile f(filename);
        if (f.open(QIODevice::WriteOnly)) {
            f.write(deploy_data.left(len));
        }
        deploy_data.remove(0,len);
    }

    emit restart();
}
