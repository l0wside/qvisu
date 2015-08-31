#ifndef QVDEPLOYSERVER_H
#define QVDEPLOYSERVER_H

#include <QObject>
#include <QByteArray>
#include <QUdpSocket>
#include <QTcpServer>


class QVDeployServer : public QObject
{
    Q_OBJECT
public:
    explicit QVDeployServer(QString);
    ~QVDeployServer();

signals:
    void restart();

public slots:

protected slots:
    void onUdpReceived();
    void onSocketError(QAbstractSocket::SocketError);

    void onTcpIncoming();
    void onTcpRead();
    void onTcpDisconnected();

private:
    QByteArray deploy_data;
    QString appdir;

    QUdpSocket *udp_socket;
    QByteArray udp_data;
    QTcpServer *tcp_server;
    QTcpSocket *tcp_socket;


};

#endif // QVDEPLOYSERVER_H
