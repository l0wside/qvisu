#ifndef QVDOORCOM_H
#define QVDOORCOM_H

#include <QTimer>
#include "../qvelement.h"
#include "../qvpopupframe.h"
#include "../qvsvgwidget.h"
#include <pj/types.h>
#include <pjsua.h>


class QVDoorcom : public QVElement
{
    Q_OBJECT
public:
    explicit QVDoorcom(QDomElement xml_desc, QString container, QWidget *parent);

    bool checkCaller(QString caller);
    static QString pj2qstring(pj_str_t);
    int active_call;

    void onIncomingCall_wrapper(int call_id);
    void onCallState_wrapper(int call_id,QString state);
    void onCallMediaState_wrapper(int call_id);
    void onFilePlayed_wrapper();

protected:
    void resizeEvent(QResizeEvent*);

signals:
    void incomingCall(int call_id);
    void callState(int call_id,QString state);
    void callMediaState(int call_id);

public slots:
    void onIncomingCall(int call_id);
    void onCallState(int call_id,QString state);
    void onCallMediaState(int call_id);

protected slots:
    void onAcceptPressed();
    void onHangupPressed();
    void onDoorOpenPressed();
    void onDTMFTimer();
    void onHangupTimer();

private:
    static int instance_count;
    QString accepted_caller;

    QVPopupFrame *popup;
    QVSvgWidget *w_accept, *w_hangup, *w_dooropen;

    QTimer dtmf_timer;
    QTimer hangup_timer;
    QString code_accept, code_hangup, code_dooropen;

    pj_pool_t          *pj_pool;
    pj_caching_pool    pj_cpool;
    pjmedia_port       *pj_tonegen;
    pjsua_conf_port_id  pj_toneslot;

    /* Doorbell tone */
    pjmedia_port *bell_file_port;
    pjsua_conf_port_id bell_port_id;
    char *bell_wav;
};


#endif // QVDOORCOM_H
