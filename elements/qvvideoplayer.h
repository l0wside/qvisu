#ifdef VIDEO
#ifndef QVVIDEOPLAYER_H
#define QVVIDEOPLAYER_H

#include <QObject>
#include "vlc/vlc.h"

class QVVideoPlayer : public QObject
{
    Q_OBJECT
public:
    explicit QVVideoPlayer(QWidget *window, QString url);
    static libvlc_instance_t * inst;

signals:

public slots:
    void play();
    void stop();

private:
    libvlc_media_player_t *mp;
    libvlc_media_t *m;
    void *hwnd;
};

#endif // QVVIDEOPLAYER_H

#endif
