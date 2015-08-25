#include "qvdoorcom.h"
#include <pjsua-lib/pjsua.h>

int QVDoorcom::instance_count = 0;
QVDoorcom *doorcom = 0;

extern "C" {
/* Callback called by PJSUA upon receiving incoming call */
static void on_incoming_call(pjsua_acc_id acc_id, pjsua_call_id call_id,
                 pjsip_rx_data *rdata);

/* Callback called by PJSUA when call's state has changed */
static void on_call_state(pjsua_call_id call_id, pjsip_event *e);

/* Callback called by PJSUA when call's media state has changed */
static void on_call_media_state(pjsua_call_id call_id);
}

QVDoorcom::QVDoorcom(QDomElement xml_desc, QString container, QWidget *parent) :
    QVElement(xml_desc,container,parent)
{
    if (w < 1) {
        w = 1;
    }
    if (h < 1) {
        h = 1;
    }
    popup = 0;
    if (instance_count > 0) {
        w = h = 0;
        return;
    }
    instance_count = 1;
    doorcom = this;

    popup = new QVPopupFrame(parent);

    QDomElement e_server = xml_desc.firstChildElement("server");
    QDomElement e_user = xml_desc.firstChildElement("user");
    QDomElement e_password = xml_desc.firstChildElement("password");
    if (e_server.isNull() || e_user.isNull() || e_password.isNull()) {
        qDebug() << "SIP client: insufficient auth info";
        return;
    }

    QString s_server = e_server.text();
    QString s_user = e_user.text();
    QString s_password = e_password.text();

    QDomElement e_caller = xml_desc.firstChildElement("source");
    if (!e_caller.isNull()) {
        accepted_caller = e_caller.text();
    }

    QDomElement e_accept = xml_desc.firstChildElement("accept");
    if (!e_accept.isNull()) {
        code_accept = e_accept.text();
    }

    QDomElement e_hangup = xml_desc.firstChildElement("hangup");
    if (!e_hangup.isNull()) {
        code_hangup = e_hangup.text();
    }

    QDomElement e_dooropen = xml_desc.firstChildElement("dooropen");
    if (!e_dooropen.isNull()) {
        code_dooropen = e_dooropen.text();
    }

    active_call = -1;

    QObject::connect(this,SIGNAL(incomingCall(int)),this,SLOT(onIncomingCall(int)),Qt::QueuedConnection);
    QObject::connect(this,SIGNAL(callState(int,QString)),this,SLOT(onCallState(int,QString)),Qt::QueuedConnection);
    QObject::connect(this,SIGNAL(callMediaState(int)),this,SLOT(onCallMediaState(int)),Qt::QueuedConnection);

    w_accept = new QVSvgWidget(":/icons/phone_call.svg",popup->content());
    w_hangup = new QVSvgWidget(":/icons/phone_call_end.svg",popup->content());
    w_dooropen = new QVSvgWidget(":/icons/door_open.svg",popup->content());
    QObject::connect(w_accept,SIGNAL(clicked(double,double)),this,SLOT(onAcceptPressed()));
    QObject::connect(w_hangup,SIGNAL(clicked(double,double)),this,SLOT(onHangupPressed()));
    QObject::connect(w_dooropen,SIGNAL(clicked(double,double)),this,SLOT(onDoorOpenPressed()));

    hangup_timer.setSingleShot(true);
    hangup_timer.setInterval(800);
    QObject::connect(&hangup_timer,SIGNAL(timeout()),this,SLOT(onHangupTimer()));

    dtmf_timer.setSingleShot(true);
    dtmf_timer.setInterval(800);
    QObject::connect(&dtmf_timer,SIGNAL(timeout()),this,SLOT(onDTMFTimer()));

    /******* Init PJSUA ********/
    pjsua_acc_id acc_id;
    pj_status_t status;

    /* Create pjsua first! */
    status = pjsua_create();
    if (status != PJ_SUCCESS) {
        qDebug() << "Cannot create PJSUA SIP client, cause:" << status;
        return;
    }

    /* Init pjsua */
    pjsua_config cfg;
    pjsua_logging_config log_cfg;

    pjsua_config_default(&cfg);
    cfg.cb.on_incoming_call = &on_incoming_call;
    cfg.cb.on_call_media_state = &on_call_media_state;
    cfg.cb.on_call_state = &on_call_state;

    pjsua_logging_config_default(&log_cfg);
    log_cfg.console_level = 1;

    status = pjsua_init(&cfg, &log_cfg, NULL);
    if (status != PJ_SUCCESS) {
        qDebug() << "Cannot init PJSUA SIP client, cause: " << status;
        return;
    }

    /* Add UDP transport. */
    pjsua_transport_config transport_cfg;

    pjsua_transport_config_default(&transport_cfg);
    transport_cfg.port = 5060;
    status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &transport_cfg, NULL);
    if (status != PJ_SUCCESS) {
        qDebug() << "Cannot init PJSUA UDP transport, cause: " << status;
        return;
    }

    /* Initialization is done, now start pjsua */
    status = pjsua_start();
    if (status != PJ_SUCCESS) {
        qDebug() << "Cannot start PJSUA SIP client, cause: " << status;
        return;
    }
    /* Register to SIP server by creating SIP account. */
    pjsua_acc_config acc_cfg;

    pjsua_acc_config_default(&acc_cfg);
    QString s_id = "sip:" + s_user + "@" + s_server;
    QString s_uri = "sip:" + s_server;
    acc_cfg.cred_count = 1;
    acc_cfg.cred_info[0].realm = pj_str(strdup(s_server.toLocal8Bit().data()));
    acc_cfg.cred_info[0].scheme = pj_str("digest");
    acc_cfg.cred_info[0].username = pj_str(strdup(s_user.toLocal8Bit().data()));
    acc_cfg.cred_info[0].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
    acc_cfg.cred_info[0].data = pj_str(strdup(s_password.toLocal8Bit().data()));
    acc_cfg.id = pj_str(strdup(s_id.toLocal8Bit().data()));
    acc_cfg.reg_uri = pj_str(strdup(s_uri.toLocal8Bit().data()));
    status = pjsua_acc_add(&acc_cfg, PJ_TRUE, &acc_id);
    if (status != PJ_SUCCESS) {
        qDebug() << "PJSUA auth data invalid, cause: " << status;
        return;
    }
}

void QVDoorcom::resizeEvent(QResizeEvent*) {
    if ((width() <= 0) || (height() <= 0) || (popup == 0)) {
        return;
    }
    int popup_w = (int)(width()/w*6);
    int popup_h = (int)(height()/h*4);
    int popup_x = (int)((popup->width()-popup_w)/2);
    int popup_y = (int)((popup->height()-popup_h)/2);

    popup->place(popup_x,popup_y,popup_w,popup_h);
    popup->content()->setStyleSheet(color);

    int icon_size = (int)(popup_h/4);

    w_accept->setFixedSize(icon_size,icon_size);
    w_accept->move((int)(popup_w/4-icon_size/2),popup_h-(int)(icon_size*1.5));
    w_hangup->setFixedSize(icon_size,icon_size);
    w_hangup->move((int)(popup_w/2-icon_size/2),popup_h-(int)(icon_size*1.5));
    w_dooropen->setFixedSize(icon_size,icon_size);
    w_dooropen->move((int)(popup_w*3/4-icon_size/2),popup_h-(int)(icon_size*1.5));
}

bool QVDoorcom::checkCaller(QString caller) {
    qDebug() << "checkCaller()" << caller;
    if (accepted_caller.isEmpty()) {
        return true;
    }
    int start = caller.indexOf("<");
    int end = caller.indexOf(">");
    if ((start < 0) || (end < 0) || (end < start+2)) {
        return false;
    }
    caller = caller.mid(start+1,end-start-1);
    qDebug() << "checking caller" << caller;
    if (caller == accepted_caller) {
        return true;
    }
    return false;
}

QString QVDoorcom::pj2qstring(pj_str_t pstr) {
    char* str = strndup(pstr.ptr,pstr.slen);
    QString qstr(str);
    free(str);
    return qstr;
}

void QVDoorcom::onIncomingCall(int call_id) {
    if (active_call >= 0) {
        return;
    }
    qDebug() << "incoming call" << call_id;
    active_call = call_id;
    w_dooropen->hide();
    popup->show();
}

void QVDoorcom::onCallState(int call_id, QString state) {
    if (call_id != active_call) {
        return;
    }
    qDebug() << "new state" << state;
    if (state == "DISCONNCTD") {
        popup->hide();
        active_call = -1;
    }
    if (state == "CONFIRMED") {
        w_dooropen->show();

        /* Create DTMF generator */
        pjsua_call_info ci;

            pj_pool = pjsua_pool_create("qvisu", 8192, 8192);

        qDebug() << "2" << pjmedia_tonegen_create(pj_pool, 16000, 1, 160, 16, 0, &pj_tonegen);
        qDebug() << "3" << pjsua_conf_add_port(pj_pool, pj_tonegen, &pj_toneslot);

        pjsua_call_get_info(call_id, &ci);
        qDebug() << "4" << pjsua_conf_connect(pj_toneslot, ci.conf_slot);


        /* Send DTMF code */
        if (!code_accept.isEmpty()) {
            qDebug() << "accept code" << code_accept;

            pjmedia_tone_digit d[16];
            unsigned i, count = code_accept.length();

            if (count > PJ_ARRAY_SIZE(d)) {
                count = PJ_ARRAY_SIZE(d);
            }

            pj_bzero(d, sizeof(d));
            for (i=0; i<count; i++) {
                d[i].digit = code_accept.at(i).toLatin1();
                d[i].on_msec = 200;
                d[i].off_msec = 200;
                d[i].volume = 32767;
            }

            qDebug() << "before tonegen";
            qDebug() << "tonegen result " << pjmedia_tonegen_play_digits(pj_tonegen, count, d, 0);
            dtmf_timer.start();
        }
    }
}

void QVDoorcom::onAcceptPressed() {
    if (active_call < 0) {
        return;
    }

    pjsua_call_info ci;
    if (!pjsua_call_get_info(active_call, &ci) == PJ_SUCCESS) {
        return;
    }
    qDebug() << "Accepted, state is " << pj2qstring(ci.state_text);
    if ((ci.state != PJSIP_INV_STATE_INCOMING) && (ci.state != PJSIP_INV_STATE_EARLY)) {
        return;
    }

    pjsua_call_answer(active_call, 200, NULL, NULL);

}

void QVDoorcom::onHangupPressed() {
    popup->hide();
    if (active_call < 0) {
        return;
    }

    pjsua_call_info ci;
    if (!pjsua_call_get_info(active_call, &ci) == PJ_SUCCESS) {
        return;
    }
    if (ci.state == PJSIP_INV_STATE_INCOMING) {
        pjsua_call_answer(active_call,486,NULL,NULL); /* Reject with busy */
        active_call = -1;
    } else if (ci.state == PJSIP_INV_STATE_CONNECTING) {
        pjsua_call_hangup(active_call,200,NULL,NULL); /* Hangup */
        active_call = -1;
    } else {
        if (!code_hangup.isEmpty()) {
            pj_str_t code = pj_str(strdup(code_hangup.toUtf8().data()));
            pjsua_call_dial_dtmf(active_call,&code);
            hangup_timer.setInterval(300*code_hangup.length());
        } else {
            hangup_timer.setInterval(100);
        }
        qDebug() << "hangup timer started";
        hangup_timer.start();
    }
}

void QVDoorcom::onDoorOpenPressed() {
    if (active_call < 0) {
        return;
    }

    pjsua_call_info ci;
    if (!pjsua_call_get_info(active_call, &ci) == PJ_SUCCESS) {
        return;
    }
    if (ci.state != PJSIP_INV_STATE_CONFIRMED) {
        return;
    }
    popup->hide();
    if (!code_dooropen.isEmpty()) {
        pj_str_t code = pj_str(strdup(code_dooropen.toUtf8().data()));
        pjsua_call_dial_dtmf(active_call,&code);
        hangup_timer.setInterval(300*code_dooropen.length());
    } else {
        hangup_timer.setInterval(100);
    }
    qDebug() << "hangup timer started";
    hangup_timer.start();
}

void QVDoorcom::onHangupTimer() {
    qDebug() << "hangup timer";
    if (active_call < 0) {
        return;
    }
    pjsua_call_info ci;
    if (!pjsua_call_get_info(active_call, &ci) == PJ_SUCCESS) {
        return;
    }
    if (ci.state != PJSIP_INV_STATE_CONFIRMED) {
        return;
    }
    pjsua_call_hangup(active_call,200,NULL,NULL);
    active_call = -1;
}

void QVDoorcom::onDTMFTimer() {
    qDebug() << "DTMF timer";
    pj_str_t code = pj_str(strdup(code_accept.toUtf8().data()));
    pjsua_call_dial_dtmf(active_call,&code);
}


void QVDoorcom::onCallMediaState(int call_id) {
    qDebug() << "new media state";
}

/* Wrappers for proper thread handling */
void QVDoorcom::onIncomingCall_wrapper(int call_id) {
    emit incomingCall(call_id);
}

void QVDoorcom::onCallState_wrapper(int call_id, QString state) {
    emit callState(call_id,state);
}

void QVDoorcom::onCallMediaState_wrapper(int call_id) {
    emit callMediaState(call_id);
}


/* Callback called by PJSUA upon receiving incoming call */
static void on_incoming_call(pjsua_acc_id acc_id, pjsua_call_id call_id,
                 pjsip_rx_data *rdata) {
    qDebug() << "Incoming call";
    if (!doorcom) {
        pjsua_call_answer(call_id,486,NULL,NULL); /* Busy */
        return;
    }

    pjsua_call_info ci;

    PJ_UNUSED_ARG(acc_id);
    PJ_UNUSED_ARG(rdata);

    pjsua_call_get_info(call_id, &ci);
    if (doorcom->checkCaller(QVDoorcom::pj2qstring(ci.remote_info)) && (doorcom->active_call < 0)) {
        pjsua_call_answer(call_id, 180, NULL, NULL); /* Ringing */
        doorcom->onIncomingCall_wrapper(call_id);
    }
}

/* Callback called by PJSUA when call's state has changed */
static void on_call_state(pjsua_call_id call_id, pjsip_event *e) {
    if (!doorcom) {
        return;
    }
    pjsua_call_info ci;
    pjsua_call_get_info(call_id, &ci);
    doorcom->onCallState_wrapper(call_id,QVDoorcom::pj2qstring(ci.state_text));
}

/* Callback called by PJSUA when call's media state has changed */
static void on_call_media_state(pjsua_call_id call_id) {
    qDebug() << "Call media state";
    pjsua_call_info ci;
    if (pjsua_call_get_info(call_id, &ci) != PJ_SUCCESS) {
        return;
    }
    pjsua_conf_connect(ci.conf_slot, 0);
    pjsua_conf_connect(0, ci.conf_slot);
}
