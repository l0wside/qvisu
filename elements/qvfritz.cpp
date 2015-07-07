#include <QFile>
#include <QCryptographicHash>
#include "qvfritz.h"

QVFritz::QVFritz(QDomElement xml_desc, QString container, QWidget *parent) :
    QVElement(xml_desc,container,parent)
{
    w=4;
    if (h < 2) {
        h = 6;
    }

    status = QVFritz::closed;

    QDomElement e_server = xml_desc.firstChildElement("server");
    QDomElement e_username = xml_desc.firstChildElement("username");
    QDomElement e_password = xml_desc.firstChildElement("password");
    if (e_server.isNull() || e_username.isNull() || e_password.isNotation()) {
        qDebug() << "FB Authentication data incomplete";
        return;
    }

    QDomElement e_show = xml_desc.firstChildElement("show");
    if (e_show.isNull()) {
        /* 1 = Incoming, 2 = missed, 3 = outgoing, 10 = rejected */
        show_list.append(1);
        show_list.append(2);
        show_list.append(3);
        show_list.append(10);
    } else {
        QStringList l = e_show.text().split(",");
        for (QStringList::iterator iter = l.begin(); iter != l.end(); iter++) {
            if ((*iter).trimmed().toLower() == "incoming") {
                show_list.append(2);
            }
            if ((*iter).trimmed().toLower() == "missed") {
                show_list.append(2);
            }
            if ((*iter).trimmed().toLower() == "outgoing") {
                show_list.append(3);
            }
            if ((*iter).trimmed().toLower() == "rejected") {
                show_list.append(10);
            }
        }
    }

    QFile f;
    f.setFileName(":/icons/phone_in.svg");
    if (f.open(QIODevice::ReadOnly)) {
        svg_in = f.readAll();
        f.close();
    }
    f.setFileName(":/icons/phone_out.svg");
    if (f.open(QIODevice::ReadOnly)) {
        svg_out = f.readAll();
        f.close();
    }
    f.setFileName(":/icons/phone_missed.svg");
    if (f.open(QIODevice::ReadOnly)) {
        svg_missed = f.readAll();
        f.close();
    }

    last_known_call = QDateTime::fromMSecsSinceEpoch(0);
    layout = new QGridLayout(this);
    setLayout(layout);

    server = e_server.text();
    username = e_username.text();
    password = e_password.text();

    this->socket = new QTcpSocket(this);
    this->socket_buffer = new QString();
    this->socket_header = new QString();
    this->socket_body = new QString();
    dl_socket = new QTcpSocket(this);

    QObject::connect(socket,SIGNAL(connected()),this,SLOT(onConnected()));
    QObject::connect(socket,SIGNAL(disconnected()),this,SLOT(onDisconnected()));
    QObject::connect(socket,SIGNAL(readyRead()),this,SLOT(onDataReceived()));
    QObject::connect(this,SIGNAL(soapReceived(QString,QDomElement)),this,SLOT(onSoapReceived(QString,QDomElement)));
    QObject::connect(dl_socket,SIGNAL(connected()),this,SLOT(onDLConnected()));
    QObject::connect(dl_socket,SIGNAL(disconnected()),this,SLOT(onDLDisconnected()));
    QObject::connect(dl_socket,SIGNAL(readyRead()),this,SLOT(onDLDataReceived()));
    QObject::connect(this,SIGNAL(updateCallList(QDomElement)),this,SLOT(onCallListUpdated(QDomElement)));
}

QVFritz::~QVFritz() {
    return;
    if (socket && socket->isOpen()) {
        socket->close();
    }
}

void QVFritz::onCallListUpdated(QDomElement e_data) {
    QDomNodeList e_call_list = e_data.elementsByTagName("Call");
    if (e_call_list.count() <= 0) {
        return;
    }
    QDomElement e_date = e_call_list.at(0).firstChildElement("Date");
    QDateTime reftime;
    if (e_date.isNull() || !(reftime = QDateTime::fromString(e_date.text(),"dd.MM.yy hh:mm")).isValid()) {
        return;
    }
    while (reftime.date().year() < 2000) {
        reftime = reftime.addYears(100);
    }
    qDebug() << "OCLU" << reftime << last_known_call;
    if (reftime.toMSecsSinceEpoch() <= last_known_call.toMSecsSinceEpoch()) {
        /* Update did not reveal any new calls */
qDebug() << "out!";
        return;
    }

    int line_height = QLabel("Text",this).sizeHint().height();
    int n_entries = (int)(floor(((double)height())/((double)line_height)));
qDebug() << line_height << n_entries;

    for (int n=0; n < e_call_list.count(); n++) {
        QDomElement e_call = e_call_list.at(n).toElement();
        bool ok;
        int type = e_call.firstChildElement("Type").text().toInt(&ok);
        if (!ok) {
            continue;
        }
        if (!show_list.contains(type)) {
            continue;
        }

        QDateTime when = QDateTime::fromString(e_call.firstChildElement("Date").text(),"dd.MM.yy hh:mm");
        if (!when.isValid()) {
            continue;
        }
        while (when.date().year() < 2000) {
            when = when.addYears(100);
        }
        if (when.toMSecsSinceEpoch() <= last_known_call.toMSecsSinceEpoch()) {
            break;
        }
        QLabel *w_time = new QLabel(this);
        if (when.date() == QDateTime::currentDateTime().date()) {
            w_time->setText(when.toString("h.mm"));
        } else if (when.date().daysTo(QDateTime::currentDateTime().date()) < 7 ) {
            w_time->setText(when.toString("ddd h:mm"));
        } else {
            w_time->setText(when.date().toString("d. MMM"));
        }

        QLabel *w_partner = new QLabel(this);
        if (!e_call.firstChildElement("Name").isNull()) {
            w_partner->setText(e_call.firstChildElement("Name").text());
        } else if ((type == 1) || (type == 2) || (type == 10)) {
            w_partner->setText(e_call.firstChildElement("Caller").text());
        } else {
            w_partner->setText(e_call.firstChildElement("Called").text());
        }
        if (w_partner->text().isEmpty()) {
            delete w_partner;
            delete w_time;
            continue;
        }

        QVSvgWidget *w_icon = new QVSvgWidget(this);
        switch (type) {
        case 1:
            w_icon->load(svg_in.toUtf8());
            break;
        case 2:
        case 10:
            w_icon->load(svg_missed.toUtf8());
            break;
        case 3:
            w_icon->load(svg_out.toUtf8());
            qDebug() << "OUT!";
            break;
        default: ;
        }

        w_icons.prepend(w_icon);
        w_times.prepend(w_time);
        w_callers.prepend(w_partner);
    }

//    qDebug() << "count" << w_icons.count() << w_times.count() << w_callers.count();

    while (w_icons.count() > n_entries) {
        delete w_icons.last();
        w_icons.pop_back();
    }
    while (w_times.count() > n_entries) {
        delete w_times.last();
        w_times.pop_back();
    }
    while (w_callers.count() > n_entries) {
        delete w_callers.last();
        w_callers.pop_back();
    }
//    qDebug() << "count" << w_icons.count() << w_times.count() << w_callers.count();

    int width_time = 0;
    for (QList<QLabel*>::iterator iter = w_times.begin(); iter != w_times.end(); iter++) {
        if (width_time < (*iter)->sizeHint().width()) {
            width_time = (*iter)->sizeHint().width();
        }
    }

    for (int n=0; n < w_icons.count(); n++) {
        int m = n_entries-n-1;
        w_icons.at(n)->setFixedSize(line_height,line_height);
        w_icons.at(n)->move(ofs_x(),ofs_y()+m*line_height);
        w_icons.at(n)->show();
        w_times.at(n)->move(ofs_x()+line_height,ofs_y()+m*line_height);
        w_times.at(n)->show();
        w_callers.at(n)->setFixedWidth(width()-line_height-width_time-10);
        w_callers.at(n)->move(ofs_x()+line_height+width_time+10,ofs_y()+m*line_height);
        w_callers.at(n)->show();
    }
}

void QVFritz::onConnected() {
    qDebug() << "FB connected";
    status = QVFritz::open;
    socket_buffer->clear();
    socket_header->clear();
    socket_body->clear();

    QString request;
    request = QStringLiteral("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n") +
            "<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"" +
            "xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" >\n" +
            " <s:Header>\n" +
            " <h:InitChallenge " +
            "xmlns:h=\"http://soap-authentication.org/digest/2001/10/\" " +
            "s:mustUnderstand=\"1\">\n" +
            "<UserID>admin</UserID>\n" +
            "</h:InitChallenge >\n" +
            "</s:Header>\n" +
            "<s:Body>\n" +
            "<u:GetHostNumberOfEntries xmlns:u=\"urn:dslforum-org:service:Hosts:1\">\n" +
            "</u:GetHostNumberOfEntries>\n" +
            "</s:Body>\n" +
            "</s:Envelope>\n";
    QString header =
             QStringLiteral("POST /upnp/control/wancommonifconfig1 HTTP/1.1\r\n") +
            "Host: " + server + ":49000\r\n" +
            "Content-Type: text/xml; charset=\"utf-8\"\r\n" +
            "Connection: keep-alive\r\n" +
            "SoapAction: urn:dslforum-org:service:WANCommonInterfaceConfig:1#GetCommonLinkProperties\r\n" +
            "Content-Length: " + QString::number(request.length()) + "\r\n\r\n";
    status = QVFritz::authenticating;
//    qDebug() << "AUTHENTICATING\n" << header << "\n" << request;
    socket->write(header.toUtf8());
    socket->write(request.toUtf8());
//    qDebug() << "FB request";

}

void QVFritz::onDisconnected() {
    qDebug() << "FB disconnected";
}

void QVFritz::onDataReceived() {
    qDebug() << "FB data received" << socket->bytesAvailable();
    while (socket->bytesAvailable() > 0) {
//        qDebug() << "FBR" << socket->bytesAvailable() << socket_buffer->length() << socket_status;
        socket_buffer->append(socket->readAll());
//        qDebug() << "FBR2" << socket->bytesAvailable() << socket_buffer->length();

        if (socket_status == socket_idle) {
            socket_status = process_header;
        }

        if (socket_status == process_header) {
            int pos = socket_buffer->indexOf("\r\n\r\n");
            if (pos <= 0) {
                return;
            }
            *socket_header = socket_buffer->left(pos);
//            qDebug() << "SH" << *socket_header;
            socket_buffer->remove(0,pos+4);
            pos = socket_header->toLower().indexOf("content-length: ");
            bool ok = false;
            if (pos > 0) {
                int endpos = socket_header->toLower().indexOf("\r\n",pos);
                body_remaining = socket_header->mid(pos+16,endpos-pos-16).toInt(&ok);
            }
            if (!ok) {
/*                qDebug() << "Invalid or non-existent content-length data";
                socket_status = close_for_reopen;
                socket->close();
                return; */ body_remaining = 1000;
            } else {
                socket_status = process_body;
            }
        }
        if (socket_status == process_body) {
//qDebug() << socket_buffer;
            if (socket_buffer->length() < body_remaining) {
                return;
            }
            socket_body->append(socket_buffer->left(body_remaining));
            socket_buffer->remove(0,body_remaining);
            body_remaining = 0;

            QDomDocument xml_doc;
            QDomElement xml_elem;

            if (!xml_doc.setContent(*socket_body) || (xml_doc.documentElement().nodeName() != "s:Envelope")) {
                qDebug() << "Invalid SOAP body\n" << *socket_body;
                return;
            }
//            qDebug() << *socket_body;
            emit soapReceived(*socket_header,xml_doc.documentElement());
            socket_body->clear();
            socket_header->clear();
            socket_buffer->clear();
            socket_status = socket_idle;
        }
    }
}

void QVFritz::onSoapReceived(QString, QDomElement soap) {
//    qDebug() << soap.firstChildElement().nodeName();
    QDomElement e_header;
    QDomElement e_body;

    e_header = soap.firstChildElement("s:Header");
    e_body = soap.firstChildElement("s:Body");

    if (e_header.isNull() || e_body.isNull()) {
        qDebug() << "SOAP: header and/or body missing";
        return;
    }

    if (status == QVFritz::authenticating) {
        QDomElement e_challenge = e_header.firstChildElement("h:Challenge");
        if (e_challenge.isNull()) {
            status = open;
            return;
        }
        QDomElement e_nonce = e_challenge.firstChildElement("Nonce");
        if (e_nonce.isNull()) {
            return;
        }
        nonce = e_nonce.text();
        QDomElement e_realm = e_challenge.firstChildElement("Realm");
        if (e_realm.isNull()) {
            return;
        }
        realm = e_realm.text();
        QByteArray secret = QCryptographicHash::hash(QString(username + ":" + realm + ":" + password).toUtf8(),QCryptographicHash::Md5).toHex();
/*        qDebug() << QString(username + ":" + realm + ":" + password);
        qDebug() << "nonce" << nonce << "realm" << realm << "secret" << secret.toHex();
        qDebug() << QString(secret + ":" + nonce); */
        auth = QCryptographicHash::hash(QString(secret + ":" + nonce).toUtf8(),QCryptographicHash::Md5).toHex();

        QString request = QStringLiteral("") +
                "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" +
                "<s:Envelope s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" " +
                "xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" >\n" +
                 "<s:Header>\n" +
                 "<h:ClientAuth " +
                 "xmlns:h=\"http://soap-authentication.org/digest/2001/10/\" " +
                 "s:mustUnderstand=\"1\">\n" +
                 "<Nonce>" + nonce + "</Nonce>\n" +
                 "<Auth>" + auth + "</Auth>\n" +
                 "<UserID>" + username + "</UserID>\n" +
                 "<Realm>" + realm + "</Realm>\n" +
                 "</h:ClientAuth>\n" +
                 "</s:Header>\n" +
                 "<s:Body>\n" +
                "<u:GetCallList xmlns:u=\"urn:dslforum-org:service:X_AVM-DE_OnTel:1\">\n" +
                "</u:GetCallList>\n" +
                 "</s:Body>\n" +
                "</s:Envelope>";
        QString header =
                QStringLiteral("POST /upnp/control/x_contact HTTP/1.1\r\n") +
                "Host: " + server + ":49000\r\n" +
                "Content-Type: text/xml; charset=\"utf-8\"\r\n" +
                "Connection: keep-alive\r\n" +
                "SoapAction: urn:dslforum-org:service:X_AVM-DE_OnTel:1#GetCallList\r\n" +
                "Content-Length: " + QString::number(request.length()) + "\r\n\r\n";
//        qDebug() << "REQUEST\n" << header << "\n" << request;

        socket->write(header.toUtf8());
        socket->write(request.toUtf8());
        status = request_sent;
        return;
    }
    if (status == QVFritz::request_sent) {
        QDomElement e_challenge = e_header.firstChildElement("h:NextChallenge");
        if (e_challenge.isNull()) {
            status = open;
            return;
        }
        QDomElement e_status = e_challenge.firstChildElement("Status");
        if (e_status.isNull()) {
            status = open;
            return;
        }
        QString status = e_status.text();
        if (status == "Authenticated") {
            status = QVFritz::authenticated;
        } else {
            status = QVFritz::open;
            return;
        }
        QDomElement e_nonce = e_challenge.firstChildElement("Nonce");
        if (!e_nonce.isNull()) {
            nonce = e_nonce.text();
        }
        QDomElement e_realm = e_challenge.firstChildElement("Realm");
        if (!e_realm.isNull()) {
            realm = e_realm.text();
        }

        QDomElement e_response = e_body.firstChildElement("u:GetCallListResponse");
        if (e_response.isNull()) {
            qDebug() << "Cannot find u:GetCallListResponse";
            return;
        }
        QDomElement e_url = e_response.firstChildElement("NewCallListURL");
        QString url = e_url.text();
        int pos = url.indexOf("?sid=");
        if (pos > 0) {
            sid = url.mid(pos+5);
            int pos2 = sid.indexOf("&");
            if (pos2 > 0) {
                sid = sid.left(pos2);
            }
        }
//qDebug() << "SID" << sid;
        if (!sid.isEmpty()) {
            dl_request = QStringLiteral("") +
                    "GET /calllist.lua?sid=" + sid + "&max=100 HTTP/1.1\r\n" +
                    "Host: " + server + ":49000\r\n" +
                    "Connection: close\r\n" +
                    "\r\n";
            status = QVFritz::calllist_pending;
//            dl_socket->moveToThread(new QThread());
            dl_socket->connectToHost(server,49000);
        }
        return;
    }
    if (status == QVFritz::calllist_pending) {
        qDebug() << "Call list" << soap.nodeName();
    }
}

void QVFritz::onDLConnected() {
    qDebug() << "DL connected";
    dl_buffer.clear();
    dl_socket->write(dl_request.toUtf8());
}

void QVFritz::onDLDisconnected() {
    if (dl_buffer.length() == 0) {
        return;
    }
    int sep = dl_buffer.indexOf("<?xml");
    if (sep <= 0) {
        qDebug() << "Unexpected response to download request";
        return;
    }

    dl_buffer.remove(0,sep);
    QDomDocument e_dom;
    e_dom.setContent(dl_buffer);
    QDomElement e_doc = e_dom.documentElement();

    emit updateCallList(e_doc);
}

void QVFritz::onDLDataReceived() {
    dl_buffer.append(dl_socket->readAll());
    if (dl_buffer.indexOf("</root>") > 0) {
        dl_socket->close();
    }
}


void QVFritz::resizeEvent(QResizeEvent *e) {
    qDebug() << "Fritz resizes";
    e->accept();
}

void QVFritz::onValueChanged(QString,QString) {
}

void QVFritz::onInitCompleted() {
    qDebug() << "Fritz init complete";
    if (socket == 0) {
        return;
    }
    status = QVFritz::opening;
    socket->connectToHost(server,49000);
    socket_status = QVFritz::socket_idle;
}

void QVFritz::popupClosed() {
    qDebug() << "Fritz popup closed";
}
