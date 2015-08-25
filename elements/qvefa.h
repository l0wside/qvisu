#ifndef QVEFA_H
#define QVEFA_H

#include <QtXml>
#include <QGridLayout>
#include <QLabel>
#include <QMultiMap>
#include <QString>
#include <QTimer>
#include <QNetworkAccessManager>
#include "../qvelement.h"
#include "../qvsvgwidget.h"


class QVEfa : public QVElement
{
    Q_OBJECT
public:
    explicit QVEfa(QDomElement xml_desc, QString container, QWidget *parent);
    ~QVEfa();

    void resizeEvent(QResizeEvent*);

signals:

public slots:
    void onValueChanged(QString,QString);
    void onInitCompleted();

protected slots:
    void replyReceived(QNetworkReply*);
    void onTimer();
    void redraw();

private:
    QNetworkAccessManager *network_manager;
    QString base_url, haltestelle;
    QMultiMap<QString,QString> lines;
    int time_offset;

    QList<QDateTime> req_departures;
    QList<int> req_mot_types;
    QList<bool> req_delayed;
    QList<QString> req_lines;
    QList<QString> req_destinations;

    QTimer timer;
    int redraw_counter, reload_counter;
    QGridLayout *layout;
    QList<QLabel*> labels;

};

#endif // QVEFA_H
