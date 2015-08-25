#include <QDateTime>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include "qvefa.h"

QVEfa::QVEfa(QDomElement xml_desc, QString container, QWidget *parent) :
    QVElement(xml_desc,container,parent)
{
    if (w < 2) {
        w = 2;
    }

    network_manager = new QNetworkAccessManager();
    QObject::connect(network_manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(replyReceived(QNetworkReply*)));

    QDomElement xml_elem;
    xml_elem = xml_desc.firstChildElement("base-url");
    if (!xml_elem.isNull()) {
        base_url = xml_elem.text();
        if (base_url.indexOf("?") < 0) {
            base_url += "?";
        } else if (base_url.indexOf("?") != base_url.length()-1) {
            if (base_url.lastIndexOf("&") != base_url.length()-1) {
                base_url += "&";
            }
        }
    }

    xml_elem = xml_desc.firstChildElement("stopName");
    if (!xml_elem.isNull()) {
        haltestelle = xml_elem.text();
    }

    xml_elem = xml_desc.firstChildElement("time-offset");
    if (!xml_elem.isNull()) {
        time_offset = xml_elem.text().toInt();
    } else {
        time_offset = 0;
    }

    QDomNodeList e_lines = xml_desc.elementsByTagName("line-number");
    for (int n=0; n < e_lines.count(); n++) {
        QString line = e_lines.at(n).toElement().text();
        QString direction;
        if (e_lines.at(n).toElement().hasAttribute("direction")) {
            direction = e_lines.at(n).toElement().attribute("direction");
        }
        if (lines.contains(line,direction)) {
            continue;
        }
        lines.insert(line,direction);
    }

    layout = new QGridLayout(this);
    setLayout(layout);
    layout->setColumnStretch(3,1);

    timer.setSingleShot(false);
    timer.setInterval(1000);
    QObject::connect(&timer,SIGNAL(timeout()),this,SLOT(onTimer()));
}

QVEfa::~QVEfa() {
    timer.stop();
    delete network_manager;
}

void QVEfa::resizeEvent(QResizeEvent*) {
    int n_rows = (int)(height()/((double)(QLabel(this).sizeHint().height())));
    int row = (int)(labels.count()/4);

    if (row > 0) {
        layout->setRowStretch(row,0);
    }
    while (row < n_rows) {
        labels.append(new QLabel(this));
        layout->addWidget(labels.last(),row,0);
        labels.append(new QLabel(this));
        layout->addWidget(labels.last(),row,1);
        labels.append(new QLabel(this));
        layout->addWidget(labels.last(),row,2);
        labels.append(new QLabel(this));
        layout->addWidget(labels.last(),row,3);
        row++;
    }
    layout->setRowStretch(row,1);
    redraw();
}

void QVEfa::onValueChanged(QString, QString) {

}



void QVEfa::onInitCompleted() {
    redraw_counter = 0;
    reload_counter = 0;
    timer.start();
}

void QVEfa::onTimer() {
    QDateTime now = QDateTime::currentDateTime();

    if (redraw_counter > 0) {
        redraw_counter--;
    } else {
        redraw_counter = 60-now.time().second();
        if (reload_counter > 0) {
            redraw();
        }
    }
    if (reload_counter > 0) {
        reload_counter--;
        return;
    }
    reload_counter = 120-now.time().second();
    timer.start();


    QUrl url(base_url /*QStringLiteral("http://www2.vvs.de/vvs/widget/XML_DM_REQUEST?") */
             + "zocationServerActive=1&"
             + "lsShowTrainsExplicit=1&"
             + "stateless=1&"
             + "language=de&"
             + "SpEncId=0&"
             + "anySigWhenPerfectNoOtherMatches=1&"
             + "limit=20&"
             + "depArr=departure&"
             + "type_dm=any&"
             + "anyObjFilter_dm=2&"
             + "deleteAssignedStops=1&"
             + "name_dm=" + haltestelle + "&"
             + "mode=direct&"
             + "dmLineSelectionAll=1&"
             + "itdDateYear=" + QString::number(now.date().year()) + "&"
             + "itdDateMonth=" + QString::number(now.date().month()) + "&"
             + "itdDateDay=" + QString::number(now.date().day()) + "&"
             + "itdTimeHour=" + QString::number(now.time().hour()) + "&"
             + "itdTimeMinute=" + QString::number(now.time().minute()) + "&"
             + "useRealtime=1");
    QNetworkRequest request(url);
    network_manager->get(request);
}

void QVEfa::replyReceived(QNetworkReply *reply) {
    QDomDocument doc;
    if (!doc.setContent(reply->readAll())) {
        return;
    }

    QDateTime now = QDateTime::currentDateTime();

    req_departures.clear();
    req_destinations.clear();
    req_lines.clear();
    req_mot_types.clear();

    QDomElement e_root = doc.documentElement();
    if (e_root.isNull()) {
        qDebug() << "itdRequest not found";
        return;
    }
    QDomElement e_monitor = e_root.firstChildElement("itdDepartureMonitorRequest");
    if (e_monitor.isNull()) {
        qDebug() << "itdDepartureMonitorRequest not found";
        return;
    }
    QDomElement e_departures = e_monitor.firstChildElement("itdDepartureList");
    if (e_departures.isNull()) {
        qDebug() << "itdDepartureList not found";
        return;
    }
    QDomNodeList e_departure_list = e_departures.elementsByTagName("itdDeparture");
    for (int n=0; n < e_departure_list.count(); n++) {
        QDomElement e_departure = e_departure_list.at(n).toElement();
        if (e_departure.isNull()) {
            qDebug() << "itdDeparture not found";
            continue;
        }
        QDomElement e_line = e_departure.firstChildElement("itdServingLine");

        if (e_line.isNull()) {
            qDebug() << "itdServingLine not found";
            continue;
        }
        QString line_number = e_line.attribute("number");
        if (!lines.contains(line_number,e_line.attribute("direction")) && !lines.contains(e_line.attribute("number"),"") && (!lines.isEmpty())) {
            continue;
        }

        QDomElement e_datetime = e_departure.firstChildElement("itdDateTime");
        QDomElement e_datetime_rt = e_departure.firstChildElement("itdRTDateTime");
        if (e_datetime.isNull() || e_line.isNull()) {
            continue;
        }
        QDomElement e_date = e_datetime.firstChildElement("itdDate");
        QDomElement e_time = e_datetime.firstChildElement("itdTime");
        if (e_date.isNull() || e_time.isNull()) {
            continue;
        }
        QDomElement e_date_rt, e_time_rt;
        if (!e_datetime_rt.isNull()) {
            e_date_rt = e_datetime_rt.firstChildElement("itdDate");
            e_time_rt = e_datetime_rt.firstChildElement("itdTime");
        }
        QDateTime departure, departure_rt;
        departure.setDate(QDate(e_date.attribute("year").toInt(),e_date.attribute("month").toInt(),e_date.attribute("day").toInt()));
        departure.setTime(QTime(e_time.attribute("hour").toInt(),e_time.attribute("minute").toInt()));
        if (!e_date_rt.isNull() && !e_time_rt.isNull()) {
            departure_rt.setDate(QDate(e_date_rt.attribute("year").toInt(),e_date_rt.attribute("month").toInt(),e_date_rt.attribute("day").toInt()));
            departure_rt.setTime(QTime(e_time_rt.attribute("hour").toInt(),e_time_rt.attribute("minute").toInt()));
        }

        if (departure_rt.isValid() && (now.addSecs(time_offset*60) > departure_rt)) {
            continue;
        }
        if (now.addSecs(time_offset*60) > departure) {
            continue;
        }

        if (departure_rt.isValid()) {
            req_departures.append(departure_rt);
        } else {
            req_departures.append(departure);
        }

        if (departure_rt.isValid() && (departure_rt > departure)) {
            req_delayed.append(true);
        } else {
            req_delayed.append(false);
        }

        int mot_type = 1;
        if (e_line.hasAttribute("motType")) {
            bool ok;
            mot_type = e_line.attribute("motType").toInt(&ok);
            if (!ok) {
                mot_type = -1;
            }
        }
        req_mot_types.append(mot_type);

        req_lines.append(line_number);

        req_destinations.append(e_line.attribute("direction"));

        redraw();
    }
    reload_counter = 120-now.time().second();
    timer.start();
}

void QVEfa::redraw() {
    int row = 0;

    for (int n=0; n < labels.count(); n++) {
        labels[n]->setText("");
        labels[n]->setPixmap(QPixmap());
    }

    for (int n=0; n < req_departures.count(); n++) {
        if (labels.count() <= 4*row+3) {
            break;
        }

        QString pix_file;
        QDateTime now = QDateTime::currentDateTime();
        int mins = (int)(now.secsTo(req_departures[n])/60);
        if (mins < time_offset) {
            continue;
        }

        labels[4*row]->setText(QString::number(mins)+" min");

        switch (req_mot_types[n]) {
        case 0:
        case 13:
        case 14:
        case 15:
        case 16:
        case 18:
            pix_file = ":/efa/zug.png";
            break;
        case 1:
            pix_file = ":/efa/s-bahn.png";
            break;
        case 2:
        case 3:
            pix_file = ":/efa/u-bahn.png";
            break;
        case 4:
            pix_file = ":/efa/straba.png";
            break;
        case 5:
        case 6:
        case 7:
        case 10:
        case 17:
            pix_file = ":/efa/bus.png";
            break;
        }

        if (pix_file.isEmpty()) {
            labels[4*row+1]->clear();
        } else {
            labels[4*row+1]->setScaledContents(true);
            QPixmap pixmap(pix_file);
            labels[4*row+1]->setPixmap(pixmap);
            labels[4*row+1]->setFixedSize((int)(labels[4*row]->height()*(double)pixmap.width()/(double)pixmap.height()),labels[4*row]->height());
        }
        labels[4*row+2]->setText(req_lines[n]);
        labels[4*row+3]->setText(req_destinations[n]);
        row++;
    }
}
