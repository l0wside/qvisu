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

#include <QString>
#include <QtXml>
#include <QSvgRenderer>
#include <QMouseEvent>
#include <QPixmap>
#include "qviconwidget.h"

QVIconWidget::QVIconWidget(QWidget *parent) :
    QFrame(parent)
{
    preserve_aspect_ratio = false;
    pixmap = 0;
    pix_widget = 0;
    svg_widget = 0;

    setContentsMargins(0,0,0,0);
}

QVIconWidget::QVIconWidget(QString file, QWidget *parent) :
    QFrame(parent)
{
    preserve_aspect_ratio = false;
    pixmap = 0;
    pix_widget = 0;
    svg_widget = 0;

    setContentsMargins(0,0,0,0);

    QFile f(file);
    if (!f.open(QIODevice::ReadOnly)) {
        qDebug() << "file not found";
        return;
    }
    load(f.readAll());
    f.close();
}

void QVIconWidget::preserveAspectRatio(bool preserve) {
    preserve_aspect_ratio = preserve;
}

QSize QVIconWidget::defaultSize() const {
    if (svg_widget) {
        return svg_widget->defaultSize();
    }
    if (pixmap && pixmap->isQBitmap()) {
        return pixmap->size();
    }
    return QSize(0,0);
}

float QVIconWidget::aspectRatio() const {
    QSize size = defaultSize();
    if ((size.width() == 0) || (size.height() == 0)) {
        return 1.0;
    }
    return (float)(size.height())/(float)(size.width());
}

void QVIconWidget::onSvgClicked(double xrel, double yrel) {
    qDebug() << "onSvgClicked";
    emit clicked(xrel,yrel);
}

void QVIconWidget::onSvgDragged(double xrel, double yrel) {
    emit dragged(xrel,yrel);
}

void QVIconWidget::onSvgReleased(double xrel, double yrel) {
    emit released(xrel,yrel);
}

void QVIconWidget::mousePressEvent(QMouseEvent *e) {
    double xrel = (double)(e->x())/width();
    double yrel = (double)(e->y())/height();
    emit clicked(xrel,yrel);
}

void QVIconWidget::resizeEvent(QResizeEvent *e) {
    if (e->oldSize() == size()) {
        return;
    }
    qDebug() << "res" << size();

    if (!pix_widget == 0) {
        int w = width()-frameWidth();
        int h = height()-frameWidth();

        pix_widget->setPixmap(pixmap->scaled(w-10,h-10,preserve_aspect_ratio?Qt::KeepAspectRatio:Qt::IgnoreAspectRatio));
        e->accept();
        return;
    }
    if (!svg_widget == 0) {
        svg_widget->setFixedSize(size());
    }
}

void QVIconWidget::load(const QByteArray& data) {
    /* Check for pixmap */
    if (data.startsWith(QByteArray("\x89\x50\x4e\x47\x0d\x0a\x1a\x0a")) || data.startsWith(QByteArray("\x47\x49\x46\x38")) || data.startsWith(QByteArray("\xff\xd8\xff\xe0"))) {
        /* PNG, GIF, JPG */
        if (svg_widget != 0) {
            svg_widget->deleteLater();
            svg_widget = 0;
        }
        if (pix_widget == 0) {
            pix_widget = new QLabel(this);
            pix_widget->move(contentsRect().left(),contentsRect().top());
            pix_widget->setFixedSize(size());
            pixmap = new QPixmap();
        }
        pixmap->loadFromData(data);
        if (preserve_aspect_ratio) {
            int h = pixmap->height();
            int w = pixmap->width();
            if (h/w > height()/width()) {
                h = height();
                w = height()/h*w;
            } else {
                w = width();
                h = width()/w*h;
            }
            pix_widget->setPixmap(pixmap->scaled(w,h));
        } else {
            pix_widget->setPixmap(pixmap->scaled(size()));
        }
        pix_widget->show();
        return;
    }

    if (pix_widget != 0) {
        pix_widget->deleteLater();
        delete pixmap;
        pix_widget = 0;
        pixmap = 0;
    }
    if (svg_widget == 0) {
        svg_widget = new QVSvgWidget(this);
        QObject::connect(svg_widget,SIGNAL(clicked(double,double)),this,SLOT(onSvgClicked(double,double)));
        QObject::connect(svg_widget,SIGNAL(dragged(double,double)),this,SLOT(onSvgDragged(double,double)));
        QObject::connect(svg_widget,SIGNAL(released(double,double)),this,SLOT(onSvgReleased(double,double)));
        svg_widget->move(contentsRect().left(),contentsRect().top());
        svg_widget->setFixedSize(size());
    }
    svg_widget->load(data);
    svg_widget->show();
}

void QVIconWidget::setPathStyle(QString property, QString value, QString id, bool force_insert) {
    if (svg_widget != 0) {
        svg_widget->setPathStyle(property,value,id,force_insert);
    }
}
