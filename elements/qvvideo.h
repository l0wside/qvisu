#ifdef VIDEO
#ifndef QVVIDEO_H
#define QVVIDEO_H

#include "../qvelement.h"
#include "qvvideoplayer.h"
#include "vlc/vlc.h"

class QVVideo : public QVElement
{
    Q_OBJECT
public:
    explicit QVVideo(QDomElement xml_desc, QString container, QWidget *parent);

protected:
    void hideEvent(QHideEvent*);
    void showEvent(QShowEvent*);
    void resizeEvent(QResizeEvent*);

signals:
    void requestPlay();
    void requestStop();

public slots:
    void valueModified(QString,QString);

private:
    QVVideoPlayer *player;
};

#endif // QVVIDEO_H
#endif
