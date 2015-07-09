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

#ifdef USE_LAYOUT
    QGridLayout *topLayout = new QGridLayout(frame);
    frame->setLayout(topLayout);
    QSpacerItem *spacer_h = new QSpacerItem(0,0,QSizePolicy::Maximum,QSizePolicy::Minimum);
    QSpacerItem *spacer_v = new QSpacerItem(0,0,QSizePolicy::Minimum,QSizePolicy::Maximum);
    topLayout->addItem(spacer_h,0,0,dim_x,1);
    topLayout->addItem(spacer_v,0,0,1,dim_y);
    for (uint n=0; n < dim_x; n++) {
        topLayout->setColumnStretch(n,1);
    }
    for (uint n=0; n < dim_y; n++) {
        topLayout->setRowStretch(n,1);
    }
#endif

    QDomNodeList containers = xml_data.elementsByTagName("container");
    for (int n=0; n < containers.count(); n++) {
        QDomElement container = containers.at(n).toElement();
        if (container.isNull() || container.attribute("name").isNull()) {
            qDebug() << "Invalid container";
            continue;
        }
        for (QDomElement xml_elem = container.firstChildElement("element"); !xml_elem.isNull(); xml_elem = xml_elem.nextSiblingElement("element")) {
            QVElement *elem = QVElement::createQVElement(xml_elem,container.attribute("name"),this);
            if (elem != NULL) {
                elements.append(elem);
            }
        }
    }

    QStringList item_list;

    for (QList<QVElement*>::iterator iter = elements.begin(); iter != elements.end(); iter++) {
        QList<int> geometry = (*iter)->getGeometry();
        if (geometry.count() < 4) {
            continue;
        }
#ifdef USE_LAYOUT
        topLayout->addWidget(*iter,geometry[1],geometry[0],geometry[3],geometry[2]);
#endif
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
        for (QList<QVSelector*>::iterator iter_sel = selectors.begin(); iter_sel != selectors.end(); iter_sel++) {
            QObject::connect(*iter_sel,SIGNAL(containerChanged(QString)),*iter,SLOT(onContainerChanged(QString)));
        }
        (*iter)->onContainerChanged("main");
    }

    timer = new QTimer();
    timer->setInterval(2000-QTime::currentTime().msec());
    QObject::connect(timer,SIGNAL(timeout()),this,SLOT(onTimerTimeout()));
    timer->start();

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

    for (QList<QVElement*>::iterator iter = elements.begin(); iter != elements.end(); iter++) {
        QList<int> geometry = (*iter)->getGeometry();
        if ((geometry[0]+geometry[2] > dim_x) || ((geometry[1]+geometry[3] > dim_y))) {
            qDebug() << "Element outside of raster space" << dim_x << dim_y << geometry[0] << geometry[1] << geometry[2] << geometry[3];
            continue;
        }
        (*iter)->move(geometry[0]*(raster+spacing),geometry[1]*(raster+spacing));
        (*iter)->setFixedSize(geometry[2]*raster + (geometry[2]-1)*spacing,geometry[3]*raster + (geometry[3]-1)*spacing);
    }
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
