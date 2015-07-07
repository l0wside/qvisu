#ifdef VIDEO
#include <QThread>
#include "qvvideo.h"

QVVideo::QVVideo(QDomElement xml_desc, QString container, QWidget *parent = 0) :
    QVElement(xml_desc,container,parent)
{
    if (xml_desc.firstChildElement("url").isNull()) {
        player = 0;
        return;
    }
    player = new QVVideoPlayer(this,xml_desc.firstChildElement("url").text());
    QThread *thread = new QThread();
    player->moveToThread(thread);

    QObject::connect(this,SIGNAL(requestPlay()),player,SLOT(play()));
    QObject::connect(this,SIGNAL(requestStop()),player,SLOT(stop()));

    thread->start();
}

void QVVideo::hideEvent(QHideEvent*) {
    qDebug() << "video hidden";
    emit requestStop();
}

void QVVideo::showEvent(QShowEvent*) {
    qDebug() << "video shown";
    emit requestPlay();
}

void QVVideo::resizeEvent(QResizeEvent*) {

}

void QVVideo::valueModified(QString,QString) {

}
#endif
