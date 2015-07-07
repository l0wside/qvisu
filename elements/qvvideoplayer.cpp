#ifdef VIDEO
#include <QWidget>
#include "qvvideoplayer.h"

libvlc_instance_t* QVVideoPlayer::inst = 0;

QVVideoPlayer::QVVideoPlayer(QWidget *window,QString url) :
    QObject(0)
{
    hwnd = (void*)(window->winId());
    if (inst == 0) {
        inst = libvlc_new(0, NULL);
    }

    // Create a new item
    m = libvlc_media_new_location (inst,url.toUtf8());

    // Create a media player playing environement
    mp = libvlc_media_player_new_from_media (m);
    libvlc_media_player_set_hwnd(mp,hwnd);


}

void QVVideoPlayer::play() {
    libvlc_media_player_play (mp);
}

void QVVideoPlayer::stop() {
    libvlc_media_player_stop(mp);
}
#endif
