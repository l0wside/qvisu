#include <QFile>
#include <QCryptographicHash>
#include "qvfritz.h"

QVFritz::QVFritz(QDomElement xml_desc, QString container, QWidget *parent) :
    QVElement(xml_desc,container,parent)
{
    if (w < 2) {
        w=2;
    }
    if (h < 1) {
        h = 1;
    }

    status = QVFritz::closed;

    QDomElement e_server = xml_desc.firstChildElement("server");
    QDomElement e_username = xml_desc.firstChildElement("username");
    QDomElement e_password = xml_desc.firstChildElement("password");
    if (e_server.isNull() || e_username.isNull() || e_password.isNotation()) {
        qDebug() << "FB Authentication data incomplete";
        socket = 0;
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

    max_entries = 100;
    QDomElement e_entries = xml_desc.firstChildElement("max-entries");
    if (!e_entries.isNull()) {
        bool ok;
        int m = e_entries.text().toInt(&ok);
        if (ok && (m > 0)) {
            max_entries = m;
        }
    }

    max_days = 0;
    QDomElement e_days = xml_desc.firstChildElement("max-days");
    if (!e_days.isNull()) {
        bool ok;
        int m = e_days.text().toInt(&ok);
        if (ok && (m > 0)) {
            max_days = m;
        }
    }
    qDebug() << "FitzFon max days" << max_days;

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
    layout->setColumnStretch(0,1);
    layout->setColumnStretch(1,1);

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
    qDebug() << "Fritz constructor done";
}

QVFritz::~QVFritz() {
    return;
    if (socket && socket->isOpen()) {
        socket->close();
    }
}

void QVFritz::onCallListUpdated(QDomElement e_data) {
    for (int n=0; n < labels.count(); n++) {
        labels[n]->setText("");
    }

    QDomNodeList e_call_list = e_data.elementsByTagName("Call");
    if (e_call_list.count() <= 0) {
        return;
    }
#if 0
    for (int n=0; n < e_call_list.count(); n++) {
        qDebug() << e_call_list.at(n).firstChildElement("Date").text()
                 << e_call_list.at(n).firstChildElement("Type").text()
                 << e_call_list.at(n).firstChildElement("Caller").text()
                 << e_call_list.at(n).firstChildElement("Name").text();

    }
#endif
    QDomElement e_date = e_call_list.at(0).firstChildElement("Date");
    QDateTime reftime;
    if (e_date.isNull() || !(reftime = QDateTime::fromString(e_date.text(),"dd.MM.yy hh:mm")).isValid()) {
        return;
    }
    while (reftime.date().year() < 2000) {
        reftime = reftime.addYears(100);
    }

    for (int n=0; n < e_call_list.count(); n++) {
        QDomElement e_call = e_call_list.at(n).toElement();
        bool ok;
        int type = e_call.firstChildElement("Type").text().toInt(&ok);
        if (!ok) {
            continue;
        }

        QDateTime when = QDateTime::fromString(e_call.firstChildElement("Date").text(),"dd.MM.yy hh:mm");
        if (!when.isValid()) {
            continue;
        }
        while (when.date().year() < 2000) {
            when = when.addYears(100);
        }

        QString partner;
        if (!e_call.firstChildElement("Name").text().isEmpty()) {
            partner = e_call.firstChildElement("Name").text();
        } else if ((type == 1) || (type == 2) || (type == 10)) {
            partner = e_call.firstChildElement("Caller").text();
        } else {
            partner = e_call.firstChildElement("Called").text();
        }
#if 0
        qDebug() << "Name" << e_call.firstChildElement("Name").text();
        qDebug()  << "Caller" << e_call.firstChildElement("Caller").text();
        qDebug()  << "Called" << e_call.firstChildElement("Called").text();
        qDebug() << "Number" << e_call.firstChildElement("Number").text();
        qDebug() << "Partner" << partner;
#endif
        if ((type == 1) || (type == 3)) {
            if (last_successful_calls.contains(partner)) {
                if (when > last_successful_calls[partner]) {
                    last_successful_calls[partner] = when;
                }
            } else {
                last_successful_calls.insert(partner,when);
            }
        }
    }

    missed_calls_count.clear();
    for (int n=0; n < e_call_list.count(); n++) {
        QDomElement e_call = e_call_list.at(n).toElement();
        bool ok;
        int type = e_call.firstChildElement("Type").text().toInt(&ok);
        if (!ok) {
            continue;
        }

        QDateTime when = QDateTime::fromString(e_call.firstChildElement("Date").text(),"dd.MM.yy hh:mm");
        if (!when.isValid()) {
            continue;
        }
        while (when.date().year() < 2000) {
            when = when.addYears(100);
        }

        QString partner;
        if (!e_call.firstChildElement("Name").text().isEmpty()) {
            partner = e_call.firstChildElement("Name").text();
        } else if ((type == 1) || (type == 2) || (type == 10)) {
            partner = e_call.firstChildElement("Caller").text();
        } else {
            partner = e_call.firstChildElement("Called").text();
        }
#if 0
        QDomNodeList ll = e_call.childNodes();
        for (int n=0; n < ll.count(); n++) {
            if (ll.at(n).isElement()) {
                qDebug() << "#" << ll.at(n).nodeName() << ll.at(n).toElement().text();
            }
        }
#endif
        if ((type == 2) || (type == 10)) {
//            qDebug() << "Missed Call" << partner << when;
            if ((last_successful_calls.contains(partner) && (last_successful_calls[partner] < when))
            || (!last_successful_calls.contains(partner))) {
                missed_calls_count[partner]++;
            }

            if (!last_missed_calls.contains(partner)) {
                last_missed_calls.insert(partner,when);
            }

            if (last_missed_calls[partner] < when) {
                last_missed_calls[partner] = when;
            }
        }
    }

//    qDebug() << "Missed\n" << last_missed_calls;
 //   qDebug() << "Success\n" << last_successful_calls;

    QMap<QDateTime,QString> missed_calls_reversed;
    QList<QDateTime> missed_call_keys;
    for (int n=0; n < last_missed_calls.count(); n++) {
        QString partner = last_missed_calls.keys().at(n);
        missed_call_keys.append(last_missed_calls[partner]);
        missed_calls_reversed.insert(last_missed_calls[partner],partner);
    }

//    qDebug() << missed_calls_reversed;
    std::sort(missed_call_keys.begin(),missed_call_keys.end());

    int row = 0;
    for (int n=missed_call_keys.count()-1; n >= 0; n--) {
        QDateTime when = missed_call_keys[n];

        if ((max_days > 0) && (when.daysTo(QDateTime::currentDateTime()) > max_days)) {
            continue;
        }
        qDebug() << when;


        QString partner = missed_calls_reversed[when];
        int count = missed_calls_count[partner];
        if (count == 0) {
            continue;
        }

        if (2*row+1 >= labels.count()) {
            break;
        }
        labels[2*row]->setText(partner);
        QString s_when;
        if (when.date() == QDateTime::currentDateTime().date()) {
            s_when = when.toString("h.mm");
        } else if (when.date().daysTo(QDateTime::currentDateTime().date()) < 7 ) {
            s_when = when.toString("ddd h:mm");
        } else {
            s_when = when.date().toString("d. MMM");
        }
        labels[2*row+1]->setText(s_when + " (" + QString::number(count) + ")");
        row++;
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
            "<UserID>" + username + "</UserID>\n" +
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
            timer.setInterval(1000);
            QObject::connect(&timer,SIGNAL(timeout()),this,SLOT(requestCallList()));
            timer.setSingleShot(false);
            timer_counter = 0;
            timer.start();
        }
        return;
    }
    if (status == QVFritz::calllist_pending) {
        qDebug() << "Call list" << soap.nodeName();
    }
}

void QVFritz::requestCallList() {
//    qDebug() << "QVFritz::requestCallList" << timer_counter;
    if (timer_counter > 0) {
        timer_counter--;
        return;
    } else {
        timer_counter = 60;
	}

    dl_request = QStringLiteral("") +
            "GET /calllist.lua?sid=" + sid + "&max=" + QString::number(max_entries) + " HTTP/1.1\r\n" +
            "Host: " + server + ":49000\r\n" +
            "Connection: close\r\n" +
            "\r\n";
    status = QVFritz::calllist_pending;
//            dl_socket->moveToThread(new QThread());
    dl_socket->connectToHost(server,49000);
    qDebug() << "QVFritz: connecting DL socket";
}

void QVFritz::onDLConnected() {
    qDebug() << "QVFritz: DL connected";
    dl_buffer.clear();
    dl_socket->write(dl_request.toUtf8());
}

void QVFritz::onDLDisconnected() {
    qDebug() << "QVFritz: DL finished";
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

    int lheight = QLabel("W").sizeHint().height();
    QLabel *l = new QLabel("W");
    l->setObjectName("mini");
    int lheight_mini = l->sizeHint().height();
    delete l;

    int count = (int)floor(((double)height())/(lheight+lheight_mini));
    for (int n = (int)floor(labels.count()/2); n < count; n++) {
        labels.append(new QLabel(this));
        layout->addWidget(labels.last(),2*n,0,1,1,Qt::AlignLeft);
        labels.append(new QLabel(this));
        labels.last()->setObjectName("mini");
        layout->addWidget(labels.last(),2*n+1,0,1,1,Qt::AlignLeft);
    }
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
