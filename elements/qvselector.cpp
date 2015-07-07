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

#include "qvselector.h"
#include "../qvswitchicon.h"

QVSelector::QVSelector(QDomElement xml_desc, QString container, QWidget *parent) :
    QVElement(xml_desc,container,parent)
{
    if (w <= 0) {
        w = 4;
    }
    h = 1;

    QDomElement xml_elem;

    w_icon = 0;
    xml_elem = xml_desc.firstChildElement("icon");
    if (!xml_elem.isNull()) {
        QFile file_icon(findFilePath(xml_elem.text()));
        if (file_icon.open(QIODevice::ReadOnly)) {
            svg_icon = file_icon.readAll();
            file_icon.close();
        }
        if (!svg_icon.isEmpty()) {
            w_icon = new QVIconWidget (this);
            w_icon->load(svg_icon);
            QObject::connect(w_icon,SIGNAL(clicked(double,double)),this,SLOT(onSvgPressed()));
        }
    }

    xml_elem = xml_desc.firstChildElement("text");
    w_text = new QLabel(xml_elem.text(),this);
    w_text->setAlignment(Qt::AlignCenter);

    xml_elem = xml_desc.firstChildElement("target");
    if (xml_elem.isNull()) {
        target = "";
    } else {
        target = xml_elem.text();
    }

    if (target == "main") {
        setStyleSheet(active_color);
    } else {
        setStyleSheet(color);
    }

    /* Create widgets for status display */
    QDomNodeList items = xml_desc.elementsByTagName("item");
    for (int n=0; n < items.count(); n++) {
        if (!items.at(n).isElement()) {
            continue;
        }
        QDomElement xml_elem = items.at(n).toElement();
        if (!xml_elem.hasAttribute("action")) {
            continue;
        }
        if (xml_elem.text().isEmpty()) {
            continue;
        }
        QString action = xml_elem.attribute("action");
        if ((action == "temp-setpoint") || (action == "temp-value")) {
            continue;
        }
        if (action == "value") {
            display_value_t *dv = new display_value_t();
            dv->item = xml_elem.text();
            dv->widget = new QLabel(this);
            ((QLabel*)(dv->widget))->setAlignment(Qt::AlignHCenter);
            dv->widget->setObjectName("mini");
            if (xml_elem.hasAttribute("label")) {
                dv->label = xml_elem.attribute("label");
            } else {
                dv->label.clear();
            }
            if (xml_elem.hasAttribute("unit")) {
                dv->unit = xml_elem.attribute("unit");
            } else {
                dv->unit.clear();
            }
            if (xml_elem.hasAttribute("precision")) {
                bool ok;
                dv->precision = xml_elem.attribute("precision").toInt(&ok);
                if ((dv->precision < 0) || !ok) {
                    dv->precision = 0;
                }
            } else {
                dv->precision = 0;
            }
            disp_contents.append(dv);
            continue;
        }
        if (action == "status") {
            display_value_t *dv = new display_value_t();
            if (xml_elem.hasAttribute("icon")) {
                dv->widget = new QVSwitchIcon(xml_elem.attribute("icon"),xml_elem.attribute("icon-color"),xml_elem.attribute("icon-active-color"),xml_elem.attribute("icon-color-mode"),this);
                QObject::connect((QVSwitchIcon*)(dv->widget),SIGNAL(clicked(double,double)),this,SLOT(onSvgPressed()));
            } else {
                dv->widget = 0;
            }
            dv->value = 0.0;
            dv->item = xml_elem.text();

            disp_contents.append(dv);
            continue;
        }
    }
}

void QVSelector::mousePressEvent(QMouseEvent *event) {
    if (target.isEmpty()) {
        return;
    }
    if (event) {
        event->accept();
    }

    emit containerChanged(target);
}

void QVSelector::onContainerChanged(QString container) {
    if (container == target) {
        setStyleSheet(active_color);
    } else {
        setStyleSheet(color);
    }
}

void QVSelector::onSvgPressed() {
    mousePressEvent(0);
}

void QVSelector::resizeEvent(QResizeEvent *event) {
    if (event && (event->oldSize() == size())) {
        return;
    }

    int icon_height, icon_width = 0;
    if (w_icon) {
        icon_height = height();
        icon_width = (int)(icon_height / w_icon->aspectRatio());
        if (icon_width > width() * max_icon_width()) {
            icon_width = (int)(width() * max_icon_width());
            icon_height = (int)(icon_width * w_icon->aspectRatio());
        }
        w_icon->setFixedSize(icon_width,icon_height);
        w_icon->move(ofs_x(),ofs_y()+(int)((height()-icon_height)/2));
    }
    w_text->move(ofs_x()+icon_width,ofs_y()+(int)((height()-w_text->sizeHint().height())/2));

    /* If there is only a single widget, display it vertically centered */
    if (disp_contents.count() == 1) {
        display_value_t *dv = disp_contents.first();
        if (dv->widget->metaObject()->className() == QStringLiteral("QVSwitchIcon")) {
            double ar = ((QVSwitchIcon*)(dv->widget))->aspectRatio();
            h = (int)(height()/2);
            w = (int)(h/ar);
        } else {
            w = dv->widget->sizeHint().width();
            h = dv->widget->sizeHint().height();
            if (h > height()) {
                h = height();
            }
        }
        dv->widget->setFixedSize(w,h);
        dv->widget->move(ofs_x()+width()-w,ofs_y()+(int)((height()-h)/2));
        return;
    }

    /* More than one widget: place list of widgets in two rows. If you feel like improving the (ugly) code, go ahead. */
    int display_height = (int)(height()/2);
    int step_x = 0;
    QList<display_value_t*>::iterator iter = disp_contents.begin();
    while (iter != disp_contents.end()) {
        int w1 = 0, w2 = 0, h1 = 0, h2 = 0;
        display_value_t *dv1 = *iter;
        if (dv1->widget->metaObject()->className() == QStringLiteral("QVSwitchIcon")) {
            double ar = ((QVSwitchIcon*)(dv1->widget))->aspectRatio();
            if (ar > 1) {
                h1 = display_height;
                w1 = (int)(display_height/ar);
            } else {
                w1 = display_height;
                h1 = (int)(display_height*ar);
            }
        } else {
            w1 = dv1->widget->sizeHint().width();
            h1 = dv1->widget->sizeHint().height();
            if (h1 > display_height) {
                h1 = display_height;
            }
        }

        iter++;
        display_value_t *dv2 = 0;
        if (iter != disp_contents.end()) {
            dv2 = *iter;
            if (dv2->widget->metaObject()->className() == QStringLiteral("QVSwitchIcon")) {
                double ar = ((QVSwitchIcon*)(dv2->widget))->aspectRatio();
                if (ar > 1) {
                    h2 = display_height;
                    w2 = (int)(display_height/ar);
                } else {
                    w2 = display_height;
                    h2 = (int)(display_height*ar);
                }
            } else {
                w2 = dv2->widget->sizeHint().width();
                h2 = dv2->widget->sizeHint().height();
                if (h2 > display_height) {
                    h2 = display_height;
                }
            }
            iter++;
        }

        int w = w1;
        if (w < w2) {
            w = w2;
        }
        int ofs_x1 = (int)((w-w1)/2);
        int ofs_x2 = (int)((w-w2)/2);
        int ofs_y1 = (int)((display_height-h1)/2);
        int ofs_y2 = (int)((display_height-h2)/2);

        dv1->widget->setFixedSize(w1,h1);
        dv1->widget->move(width()-w-step_x+ofs_x()+ofs_x1,ofs_y1+ofs_y());
        if (dv2 != 0) {
            dv2->widget->setFixedSize(w2,h2);
            dv2->widget->move(width()-w-step_x+ofs_x()+ofs_x2,display_height+ofs_y2+ofs_y());
        }
        step_x += w;
    }
}

void QVSelector::onValueChanged(QString item,QString value) {
    bool ok;

    if (!items.contains(item)) {
        return;
    }

    display_value_t *dv = 0;
    for (QList<display_value_t*>::iterator iter = disp_contents.begin(); iter != disp_contents.end(); iter++) {
        if ((*iter)->item == item) {
            dv = *iter;
            break;
        }
    }
    if (dv == 0) {
        return;
    }

    if (QString(dv->widget->metaObject()->className()) == QStringLiteral("QVSwitchIcon")) {
        int value_int = value.toInt(&ok);
        if (!ok) {
            return;
        }

        ((QVSwitchIcon*)(dv->widget))->setStatus(value_int);

    } else {
        double value_double = value.toDouble(&ok);
        if (ok) {
            QString text;
            if (!dv->label.isEmpty()) {
                text = dv->label + " ";
            }
            text += QString::number(value_double,'f',dv->precision);
            if (!dv->unit.isEmpty()) {
                text += dv->unit;
            }
            ((QLabel*)dv->widget)->setText(text);
        } else {
            ((QLabel*)dv->widget)->setText(value);
        }
    }
    resizeEvent(0);
}
