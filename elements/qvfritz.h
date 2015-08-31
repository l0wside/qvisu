#ifndef QVFRITZ_H
#define QVFRITZ_H

#include <QtXml>
#include <QGridLayout>
#include <QLabel>
#include <QList>
#include <QString>
#include <QDateTime>
#include <QTime>
#include <QTimer>
#include <QTcpSocket>
#include "../qvelement.h"
#include "../qvsvgwidget.h"


class QVFritz : public QVElement
{
    Q_OBJECT
public:
    explicit QVFritz(QDomElement xml_desc, QString container, QWidget *parent);
    ~QVFritz();

    void resizeEvent(QResizeEvent*);

signals:
    void soapReceived(QString,QDomElement);
    void updateCallList(QDomElement);

public slots:
    void onValueChanged(QString,QString);
    void onInitCompleted();
    void popupClosed();

protected slots:
    void onConnected();
    void onDisconnected();
    void onDataReceived();
    void onSoapReceived(QString, QDomElement);

    void requestCallList();
    void onDLConnected();
    void onDLDisconnected();
    void onDLDataReceived();

    void onCallListUpdated(QDomElement);

private:
    QGridLayout *layout;
    QList<QLabel*> w_callers, w_times;
    QList<QVSvgWidget*> w_icons;
    QString svg_in, svg_out, svg_missed;
    QDateTime last_known_call;
    QList<int> show_list;

    QTcpSocket *socket;

    unsigned int status;
    QString *socket_buffer;
    QString *socket_header, *socket_body;

    int body_remaining;

    static const unsigned int socket_idle = 0;
    static const unsigned int process_header = 1;
    static const unsigned int process_body = 2;

    int dummy;
    int socket_status;

    static const unsigned int closed = 0;
    static const unsigned int opening = 1;
    static const unsigned int open = 2;
    static const unsigned int authenticating = 3;
    static const unsigned int request_sent = 4;
    static const unsigned int authenticated = 10;
    static const unsigned int calllist_pending = 11;
    static const unsigned int tamcalllist_pending = 11;
    static const unsigned int closing = 50;
    static const unsigned int close_for_reopen = 60; /*
    static const unsigned int opening = 0; */

    QMap<QString,QDateTime> last_successful_calls;
    QMap<QString,QDateTime> last_missed_calls;
    QMap<QString,int> missed_calls_count;

    int dummy2[32];

    QList<QLabel*> labels;
    QTimer timer;
    int timer_counter;

    /* Authentication */
    QString server;
    QString nonce;
    QString username;
    QString password;
    QString realm;
    QString auth;
    QString sid;

    int max_entries;
    int max_days;

    /* Attachment Download */
    QTcpSocket *dl_socket;
    QString dl_request;
    QByteArray dl_buffer;
};

#endif // QVFRITZ_H
