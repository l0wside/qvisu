/* QVisu (c) 2015 Maximilian Gauger mgauger@kalassi.de
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "qvmainwindow.h"
#include "qvelement.h"
#include "elements/qvselector.h"
#include "qvdriver.h"

#include <QString>
#include <QMenu>
#include <QList>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QPalette>
#include <QSpacerItem>
#include <QSvgWidget>
#include <QToolBar>
#include <QMouseEvent>

QVMainWindow::QVMainWindow(uint dim_x, uint dim_y, QDomElement xml_data, QWidget *parent) :
    QMainWindow(parent)
{
    QVDriver *qv_driver;

    QFrame *frame = new QFrame(this);
    setCentralWidget(frame);
    frame->setObjectName("main");

    this->dim_x = dim_x;
    this->dim_y = dim_y;

    main_fallback_time = 0;
    main_fallback_timer = 0;

    QDomNodeList containers = xml_data.elementsByTagName("container");
    for (int n=0; n < containers.count(); n++) {
        QDomElement container = containers.at(n).toElement();
        if (container.isNull() || container.attribute("name").isNull()) {
            qDebug() << "Invalid container";
            continue;
        }

        if ((container.attribute("name") == "main") && (container.hasAttribute("fallback"))) {
            bool ok;
            main_fallback_time = container.attribute("fallback").toInt(&ok);
            if ((!ok) || (main_fallback_time <= 0)) {
                main_fallback_time = 0;
            } else {
                main_fallback_timer = new QTimer();
                QObject::connect(main_fallback_timer,SIGNAL(timeout()),this,SLOT(onMainFallbackTimer()));
                main_fallback_timer->setInterval(1000);
                main_fallback_timer->start();
                main_fallback_time_counter = main_fallback_time;
            }
        }
        for (QDomElement xml_elem = container.firstChildElement("element"); !xml_elem.isNull(); xml_elem = xml_elem.nextSiblingElement("element")) {
            QVElement *elem = QVElement::createQVElement(xml_elem,container.attribute("name"),this);
            if (elem != NULL) {
                elements.append(elem);
            }
        }
    }

    QDomElement e_disable_mouse = xml_data.firstChildElement("disable-mouse");
    if (!e_disable_mouse.isNull()) {
        if (e_disable_mouse.text().toLower() == "true") {
            setCursor(Qt::BlankCursor);
        }
    }

    QStringList item_list;

    for (QList<QVElement*>::iterator iter = elements.begin(); iter != elements.end(); iter++) {
        QList<int> geometry = (*iter)->getGeometry();
        if (geometry.count() < 4) {
            continue;
        }

        item_list.append((*iter)->getItemList());
    }
    item_list.removeDuplicates();

    qv_driver = new QVDriver(xml_data,item_list,this);

    /* Find all selectors */
    QList<QVSelector*> selectors;
    for (QList<QVElement*>::iterator iter = elements.begin(); iter != elements.end(); iter++) {
        if ((*iter)->metaObject()->className() != QStringLiteral("QVSelector")) {
            continue;
        }
        selectors.append((QVSelector*)(*iter));
    }

    /* Connect signals and slots, show only elements of main container */
    for (QList<QVElement*>::iterator iter = elements.begin(); iter != elements.end(); iter++) {
        QObject::connect(qv_driver,SIGNAL(valueChanged(QString,QString)),*iter,SLOT(onValueChanged(QString,QString)));
        QObject::connect(this,SIGNAL(valueGenerated(QString,QString)),*iter,SLOT(onValueChanged(QString,QString)));
        QObject::connect(*iter,SIGNAL(valueModified(QString,QString)),qv_driver,SLOT(onValueModified(QString,QString)));
        QObject::connect(qv_driver,SIGNAL(seriesReceived(QString,QMap<double,double>)),*iter,SLOT(onSeriesReceived(QString,QMap<double,double>)));
        QObject::connect(qv_driver,SIGNAL(initialized()),*iter,SLOT(onInitCompleted()));
        QObject::connect(*iter,SIGNAL(requestSeries(QString,QString,QString)),qv_driver,SLOT(onSeriesRequest(QString,QString,QString)));
        QObject::connect(this,SIGNAL(containerChanged(QString)),*iter,SLOT(onContainerChanged(QString)));
        for (QList<QVSelector*>::iterator iter_sel = selectors.begin(); iter_sel != selectors.end(); iter_sel++) {
            QObject::connect(*iter_sel,SIGNAL(containerChanged(QString)),*iter,SLOT(onContainerChanged(QString)));
        }
        (*iter)->onContainerChanged("main");
    }

    timer = new QTimer();
    timer->setInterval(2000-QTime::currentTime().msec());
    QObject::connect(timer,SIGNAL(timeout()),this,SLOT(onTimerTimeout()));
    timer->start();

    QCoreApplication::instance()->installEventFilter(this);

    /* Hide menu bars */
    QList<QToolBar *> allToolBars = findChildren<QToolBar *>();
    foreach(QToolBar *tb, allToolBars) {
        removeToolBar(tb);;
    }
    qDebug() << "QVMainWindow constructor done";
}

QVMainWindow::~QVMainWindow()
{
    qDebug() << "Destructor";
}

void QVMainWindow::resizeEvent(QResizeEvent *) {
    int w = width();
    int h = height();

    int raster_w = (int)((w-(dim_x-1)*spacing)/dim_x);
    int raster_h = (int)((h-(dim_y-1)*spacing)/dim_y);

    int raster;
    if (raster_w < raster_h) {
        raster = raster_w;
    } else {
        raster = raster_h;
    }

    int eff_w = (raster+spacing) * dim_x - spacing;
    int eff_h = (raster+spacing) * dim_y - spacing;
    int ofs_x = (int)((w-eff_w)/2);
    int ofs_y = (int)((h-eff_h)/2);
    qDebug() << "w/h" << w << h << "ofs" << ofs_x << ofs_y;

    for (QList<QVElement*>::iterator iter = elements.begin(); iter != elements.end(); iter++) {
        QList<int> geometry = (*iter)->getGeometry();
        if ((geometry[0]+geometry[2] > dim_x) || ((geometry[1]+geometry[3] > dim_y))) {
            qDebug() << "Element outside of raster space" << dim_x << dim_y << geometry[0] << geometry[1] << geometry[2] << geometry[3];
            continue;
        }
        (*iter)->move(geometry[0]*(raster+spacing)+ofs_x,geometry[1]*(raster+spacing)+ofs_y);
        (*iter)->setFixedSize(geometry[2]*raster + (geometry[2]-1)*spacing,geometry[3]*raster + (geometry[3]-1)*spacing);
    }
}

bool QVMainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (((event->type() == QEvent::MouseButtonPress) || (event->type() == QEvent::MouseButtonRelease) || (event->type() == QEvent::MouseMove))
            && (main_fallback_time > 0)) {
        main_fallback_time_counter = main_fallback_time;
    }
    return QObject::eventFilter(obj,event);
}

void QVMainWindow::onTimerTimeout() {
    if (timer->interval() != 1000) {
        timer->setInterval(1000);
    }
    emit valueGenerated("#time",QTime::currentTime().toString("hh:mm"));
    emit valueGenerated("#time-seconds",QTime::currentTime().toString("hh:mm:ss"));
    emit valueGenerated("#date-de",QDateTime::currentDateTime().toString("dd.MM.yyyy"));
    emit valueGenerated("#date-int",QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    emit valueGenerated("#date-de-weekday",QDateTime::currentDateTime().toString("ddd dd.MM.yyyy"));
    emit valueGenerated("#date-int-weekday",QDateTime::currentDateTime().toString("ddd yyyy-MM-dd"));
}

void QVMainWindow::onMainFallbackTimer() {
    qDebug() << "oMFT" << main_fallback_time << main_fallback_time_counter;
    if (main_fallback_time <= 0) {
        return;
    }
    if (main_fallback_time_counter > 0) {
        main_fallback_time_counter--;
        return;
    }
    emit containerChanged("main");
    main_fallback_time_counter = main_fallback_time;
}
