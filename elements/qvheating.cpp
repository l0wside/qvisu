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
 *
 * Alle Icons sind unter einer Creative Commons Lizenz vom Typ Namensnennung -
 * Weitergabe unter gleichen Bedingungen 3.0 Deutschland zugänglich.
 * Um eine Kopie dieser Lizenz einzusehen, konsultieren Sie
 * http://creativecommons.org/licenses/by-sa/3.0/de/ oder wenden Sie
 * sich brieflich an
 * Creative Commons, Postfach 1866, Mountain View, California, 94042, USA.
 */

#include "qvheating.h"
#include "../qvswitchicon.h"


QVHeating::QVHeating(QDomElement xml_desc, QString container, QWidget *parent) :
    QVElement(xml_desc,container,parent)
{
    if (w <= 0) {
        w = 2;
    }
    h = 2;

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
            w_icon->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
            QObject::connect(w_icon,SIGNAL(clicked(double,double)),this,SLOT(svgPressed()));
        }
    }

    xml_elem = xml_desc.firstChildElement("text");
    if (xml_elem.isNull()) {
        w_title = new QLabel("",this);
    } else {
        w_title = new QLabel(xml_elem.text(),this);
    }

    w_temp_value = new QLabel("[waiting]",this);
    w_temp_value->setAlignment(Qt::AlignHCenter);

    QDomNodeList items = xml_desc.elementsByTagName("item");
    xml_elem.clear();
    for (int n=0; n < items.count(); n++) {
        if (items.at(n).toElement().attribute("action") == "temp-setpoint") {
            xml_elem = items.at(n).toElement();
            break;
        }
    }
    if (!xml_elem.isNull()) {
        if (!xml_elem.hasAttribute("step")) {
            temp_setpoint_step = 0.5;
        } else {
            bool ok;
            temp_setpoint_step = xml_elem.attribute("step").toDouble(&ok);
            if (!ok || (temp_setpoint_step < 0.1)) {
                temp_setpoint_step = 0.5;
            }
        }
        w_temp_setpoint = new QLabel("[waiting]",this);
        w_temp_setpoint->setObjectName("mini");
        w_temp_setpoint->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        w_temp_setpoint_minus = new QVSvgWidget(":/icons/minus.svg",this);
        w_temp_setpoint_plus = new QVSvgWidget(":/icons/plus.svg",this);
        QObject::connect(w_temp_setpoint_minus,SIGNAL(clicked(double,double)),this,SLOT(svgPressed()));
        QObject::connect(w_temp_setpoint_plus,SIGNAL(clicked(double,double)),this,SLOT(svgPressed()));
    } else {
        w_temp_setpoint = 0;
    }

    items = xml_desc.elementsByTagName("item");
//    qDebug() << "items " << items.count();
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
            heating_value_t *hv = new heating_value_t();
            hv->item = xml_elem.text();
            hv->widget = new QLabel(this);
            ((QLabel*)(hv->widget))->setAlignment(Qt::AlignHCenter);
            if (xml_elem.hasAttribute("label")) {
                hv->label = xml_elem.attribute("label");
            } else {
                hv->label.clear();
            }
            if (xml_elem.hasAttribute("unit")) {
                hv->unit = xml_elem.attribute("unit");
            } else {
                hv->unit.clear();
            }
            if (xml_elem.hasAttribute("precision")) {
                bool ok;
                hv->precision = xml_elem.attribute("precision").toInt(&ok);
                if ((hv->precision < 0) || !ok) {
                    hv->precision = 0;
                }
            } else {
                hv->precision = 0;
            }
            sw_contents.append(hv);
            continue;
        }
        if ((action == "switch") || (action == "status")) {
            heating_value_t *hv = new heating_value_t();
            if (xml_elem.hasAttribute("icon")) {
                hv->widget = new QVSwitchIcon(xml_elem.attribute("icon"),xml_elem.attribute("icon-color"),xml_elem.attribute("icon-active-color"),xml_elem.attribute("icon-color-mode"),this);
            } else {
                hv->widget = 0;
            }
            hv->value = 0.0;
            hv->item = xml_elem.text();

            if (action == "switch") {
//                qDebug() << "connect to svgPressed: " << hv->item;
                QObject::connect((QVSwitchIcon*)(hv->widget),SIGNAL(clicked(double,double)),this,SLOT(svgPressed()));
            }
            sw_contents.append(hv);
            continue;
        }
    }
}


void QVHeating::resizeEvent(QResizeEvent * event) {
    if (event->oldSize().height() == height()) {
        return;
    }

    /* Rows from top to bottom:
     * Upper part (min 80% of total):
     *   40% Actual temperature
     *   30% Setpoint and +/-
     *   30% Icons for status/switches
     * max 20% icon and label */

    /* Height for icon and label */
    int label_height = 0, icon_height = 0, icon_width = 0;
    if (w_icon != 0) {
        label_height = height() * 0.2;
        icon_height = label_height;
        icon_width = icon_height / w_icon->aspectRatio();
        if (icon_width > width() * max_icon_width()) {
            icon_width = (int)(width() * max_icon_width());
        }
    }
    if (label_height < w_title->sizeHint().height()) {
        label_height = w_title->sizeHint().height();
    }

    /* Other heights (values see above) */
    int content_height = height() - label_height;
    int temp_height = (int)(content_height * 0.4); int temp_ofs = ofs_y();
    int setpoint_height = (int)(content_height * 0.3); int setpoint_ofs = ofs_y() + temp_height;
    int switch_height = (int)(content_height * 0.3); int switch_ofs = setpoint_ofs + setpoint_height;

    /* Actual temperature */
    w_temp_value->setFixedWidth(width());
    w_temp_value->move(ofs_x(),temp_ofs+(int)((temp_height-w_temp_value->sizeHint().height())/2));

    /* Setpoint and +/- */
    if (w_temp_setpoint != 0) {
        int icon_pm_height = 0, icon_pm_width = 0, w_setpoint_width = 0;

        icon_pm_height = setpoint_height;
        icon_pm_width = (int)(icon_pm_height * w_temp_setpoint_minus->aspectRatio());
        if (icon_pm_width > width() * max_icon_width()) {
            icon_pm_width = (int)(width() * max_icon_width());
            icon_pm_height = (int)(icon_pm_width / w_temp_setpoint_minus->aspectRatio());
        }
        w_setpoint_width = w_temp_setpoint->sizeHint().height()*8;
        int w_setpoint_ofs = (int)((icon_pm_height-w_temp_setpoint->sizeHint().height())/2);
        if (w_setpoint_ofs < 0) {
            w_setpoint_ofs = 0;
        }
        if (w_setpoint_width > width()-2*icon_pm_width) {
            w_setpoint_width = width()-2*icon_pm_width;
        }
        w_temp_setpoint_minus->setFixedSize(icon_pm_width,icon_pm_height);
        w_temp_setpoint_minus->move(ofs_x()+(int)((width()-w_setpoint_width)/2)-icon_pm_width,setpoint_ofs);
        w_temp_setpoint->setFixedWidth(w_setpoint_width);
        w_temp_setpoint->move(ofs_x()+(int)((width()-w_setpoint_width)/2),setpoint_ofs+w_setpoint_ofs);
        w_temp_setpoint_plus->setFixedSize(icon_pm_width,icon_pm_height);
        w_temp_setpoint_plus->move(ofs_x()+(int)((width()+w_setpoint_width)/2),setpoint_ofs);
    }

    int sw_width_max;
    if (sw_contents.count() == 0) {
        sw_width_max = 0;
    } else {
        sw_width_max = (int)(width()/sw_contents.count());
    }
    int sw_height_max = switch_height;
    int n = 0;
    for (QList<heating_value_t*>::iterator iter = sw_contents.begin(); iter != sw_contents.end(); iter++) {
        int sw_height, sw_width;
        if ((*iter)->widget == 0) {
            n++;
            continue;
        }
        if (QString((*iter)->widget->metaObject()->className()) == "QVSwitchIcon") {
            sw_height = sw_width_max * ((QVSwitchIcon*)(*iter)->widget)->aspectRatio();
            if (sw_height > sw_height_max) {
                sw_height = sw_height_max;
                sw_width = sw_height/((QVSwitchIcon*)(*iter)->widget)->aspectRatio();
            }
        } else {
            sw_height = (*iter)->widget->sizeHint().height();
            sw_width = sw_width_max;
            if (sw_height > sw_height_max) {
                sw_height = sw_height_max;
            }
        }

        (*iter)->widget->setFixedSize(sw_width,sw_height);
        (*iter)->widget->move(n*sw_width_max+(int)((sw_width_max-sw_width)/2)+ofs_x(),switch_ofs);
        n++;
    }

    if (w_icon) {
        w_icon->resize(icon_width,icon_height);
        w_icon->move(ofs_x()+(int)((width()-w_title->sizeHint().width()-icon_width)/2),ofs_y()+content_height);
    }
    int label_ofs = (int)((icon_height-w_title->sizeHint().height())/2);
    if (label_ofs < 0) {
        label_ofs = 0;
    }
    w_title->move(ofs_x()+(int)((width()-w_title->sizeHint().width()+icon_width)/2),ofs_y()+content_height+label_ofs);
}

void QVHeating::svgPressed() {
    /* Temperature setpoint modification (plus/minus) */
    if (QObject::sender() == w_temp_setpoint_minus) {
        double steps = temp_setpoint/temp_setpoint_step;
        if ((steps-floor(steps)) > 1e-4) {
           temp_setpoint = temp_setpoint_step * floor(steps);
        } else {
            temp_setpoint -= temp_setpoint_step;
        }
        w_temp_setpoint->setText(QString::number(temp_setpoint,'f',1) + "°C");
        emit valueModified(getItemByAction("temp-setpoint"),QString::number(temp_setpoint,'f',3));
        return;
    }
    if (QObject::sender() == w_temp_setpoint_plus) {
        double steps = temp_setpoint/temp_setpoint_step;
        if (steps-floor(steps) > 1e-4) {
           temp_setpoint = temp_setpoint_step * floor(steps+1);
        } else {
            temp_setpoint += temp_setpoint_step;
        }
        w_temp_setpoint->setText(QString::number(temp_setpoint,'f',1) + "°C");
        emit valueModified(getItemByAction("temp-setpoint"),QString::number(temp_setpoint,'f',3));
        return;
    }

    /* Other buttons from list (Switches) */
    for (QList<heating_value_t*>::iterator iter = sw_contents.begin(); iter != sw_contents.end(); iter++) {
        if (QString((*iter)->widget->metaObject()->className()) != "QVSwitchIcon") {
            continue;
        }
        if ((*iter)->widget != QObject::sender()) {
            continue;
        }
        if ((*iter)->value < 1e-4) {
            (*iter)->value = 1;
        } else {
            (*iter)->value = 0;
        }
//        qDebug() << "clicked on toolbar " << (*iter)->item << "now" << QString::number((*iter)->value,'f',0);
        onValueChanged((*iter)->item,QString::number((*iter)->value,'f',0));
        emit valueModified((*iter)->item,QString::number((*iter)->value,'f',0));
    }

}

void QVHeating::onValueChanged(QString item, QString value) {
    bool ok;
    double value_double;

    if (!items.contains(item)) {
        return;
    }

    QMap<QString,QVItem>::iterator iter = items.find(item);
    QString action = iter->getAction();
    if (action == "temp-value") {
        value_double = value.toDouble(&ok);
        if (!ok) {
            return;
        }
        temp_value = value_double;
        w_temp_value->setText(QString::number(temp_value,'f',1) + "°C");
        return;
    }
    if ((action == "temp-setpoint") && (w_temp_setpoint != 0)) {
        value_double = value.toDouble(&ok);
        if (!ok) {
            return;
        }
        temp_setpoint = value_double;
        w_temp_setpoint->setText(QString::number(temp_setpoint,'f',1) + "°C");
        return;
    }

    heating_value_t *hv = 0;
    for (QList<heating_value_t*>::iterator iter = sw_contents.begin(); iter != sw_contents.end(); iter++) {
//        qDebug() << "Search " << (*iter)->item << item;
        if ((*iter)->item == item) {
            hv = *iter;
            break;
        }
    }
    if (hv == 0) {
        return;
    }

    if (QString(hv->widget->metaObject()->className()) == QStringLiteral("QVSwitchIcon")) {
        int value_int = value.toInt(&ok);
        if (!ok) {
            return;
        }
        ((QVSwitchIcon*)(hv->widget))->setStatus(value_int);
    } else {
        QString text;
        double value_double = value.toDouble(&ok);
        if (!ok) {
            return;
        }
        if (!hv->label.isEmpty()) {
            text = hv->label + " ";
        }
        text += QString::number(value_double,'f',hv->precision);
        if (!hv->unit.isEmpty()) {
            text += hv->unit;
        }
        ((QLabel*)hv->widget)->setText(text);
//        qDebug() << "set text " << text;
    }
}
