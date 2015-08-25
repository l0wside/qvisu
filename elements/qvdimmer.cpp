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

#include <cmath>
#include "qvdimmer.h"

QVDimmer::QVDimmer(QDomElement xml_desc, QString container, QWidget *parent) :
    QVElement(xml_desc,container,parent)
{
    if (w <= 0) {
        w = 4;
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

    w_dimmer = new QVSvgWidget(this);
    QFile file_dimmer(":/icons/dimmer.svg");
    if (file_dimmer.open(QIODevice::ReadOnly)) {
        svg_dimmer = file_dimmer.readAll();
        file_dimmer.close();
    }
    QObject::connect(w_dimmer,SIGNAL(dragged(double,double)),this,SLOT(dimmerDragged(double,double)));
    QObject::connect(w_dimmer,SIGNAL(clicked(double,double)),this,SLOT(dimmerClicked(double,double)));



    w_sw_icon = new QVSwitchIcon(xml_desc.firstChildElement("sw-icon"),this);
    QObject::connect(w_sw_icon,SIGNAL(clicked(double,double)),this,SLOT(svgPressed()));

    xml_elem = xml_desc.firstChildElement("text");
    if (xml_elem.isNull()) {
        w_text = new QLabel(this);
    } else {
        w_text = new QLabel(xml_elem.text(),this);
    }

    QString max_value_str;
    QString type_str;
    bool ok;
    if (items.contains(getItemByAction("dimmer"))) {
        QVItem item = items.value(getItemByAction("dimmer"));
        max_value_str = item.getAttribute("max-value");
        type_str = item.getAttribute("type");
    }
    if ((type_str.toLower() == "double") || (type_str.toLower() == "float")) {
        data_type = type_float;
    } else {
        data_type = type_int;
    }
    max_value = max_value_str.toDouble(&ok);
    if (!ok || (max_value < 0.1)) {
        switch (data_type) {
        case type_float: max_value = 100.0; break;
        case type_int: max_value = 255.0; break;
        default: max_value = 255.0;
        }
        if (data_type == type_int) {
            max_value = floor(max_value);
        }
    }

    switch_value = false;
    dim_value = max_value/2;
    display_ratio = 0;
}


void QVDimmer::resizeEvent(QResizeEvent * event) {
    if (event->oldSize().height() == height()) {
        return;
    }

    int icon_height = 0, icon_width = 0, icon_sw_height = 0, icon_sw_width = 0;

    if (w_icon != 0) {
        icon_height = (int)(height() * max_label_height());
        icon_width = (int)(icon_height/w_icon->aspectRatio());
        if (icon_width > width() * max_icon_width()) {
            icon_width = (int)(width() * max_icon_width());
            icon_height = (int)(icon_width * w_icon->aspectRatio());
        }
        if (icon_width == 0) {
            w_icon->hide();
        }
    }

    int content_height, label_height;

    if (w_text->sizeHint().height() > icon_height) {
        label_height = w_text->sizeHint().height();
    } else {
        label_height = icon_height;
    }
    content_height = height() - label_height;



    if (w_sw_icon != 0) {
        icon_sw_height = content_height;
        icon_sw_width = (int)(icon_sw_height*w_sw_icon->aspectRatio());
        if (icon_sw_width > width() * max_icon_width()) {
            icon_sw_width = (int)(width() * max_icon_width());
            icon_sw_height = (int)(icon_sw_width * w_sw_icon->aspectRatio());
        }
        if (icon_sw_width == 0) {
            w_sw_icon->hide();
        }
    }

//    qDebug() << "SS" << icon_sw_height << icon_sw_width << height() << content_height <<

    int dimmer_height = content_height;
    if (dimmer_height > height() * max_dimmer_height()) {
        dimmer_height = (int)(height() * max_dimmer_height());
    }
    int dimmer_y = (int)((content_height-dimmer_height)/2);
/*    if (dimmer_y + dimmer_height > height()-w_text->sizeHint().height()) {
        dimmer_y = height()-w_text->sizeHint().height()-dimmer_height;
    }
    if (dimmer_y < 0) {
        dimmer_y = 0;
    } */
    w_dimmer->move(ofs_x(),ofs_y()+dimmer_y);
    w_dimmer->resize(width()-icon_sw_width-10,dimmer_height);

    if (!svg_dimmer.isEmpty()) {
        display_ratio = ((double)(height()-w_text->sizeHint().height()))/((double)(width()-icon_width-icon_sw_width-20))*10.0;
        updateDimmerWidget(dim_value/max_value);
//        qDebug() << "width" << ((double)(width()-icon_width-icon_sw_width-20)) << "height" << ((double)(height()-w_text->sizeHint().height())) << "ratio" << display_ratio;
    }

    if (w_sw_icon) {
        w_sw_icon->resize(icon_sw_width,icon_sw_height);
        w_sw_icon->move(ofs_x()+width()-icon_sw_width,ofs_y()+(int)((content_height-icon_sw_height)/2));
    }

    if (w_icon) {
        w_icon->resize(icon_width,icon_height);
        w_icon->move(ofs_x()+((int)(width()-w_text->sizeHint().width()-icon_width)/2),ofs_y()+content_height);
    }

    int w_text_ofs = (int)((icon_height-w_text->sizeHint().height())/2);
    if (w_text_ofs < 0) {
        w_text_ofs = 0;
    }

    w_text->setFixedSize(w_text->sizeHint());
    w_text->move(ofs_x()+((int)(width()-w_text->sizeHint().width()+icon_width)/2),ofs_y()+content_height+w_text_ofs);
}


void QVDimmer::mousePressEvent(QMouseEvent *event) {
    if (event) {
        event->accept();
    }
    switch_value = !switch_value;

    QString item = getItemByAction("switch");
    if (item.isNull()) {
        return;
    }

    w_sw_icon->setStatus(switch_value);

    qDebug() << "Emitting " << item << " to " << QString::number(switch_value);
    emit valueModified(item,QString::number(switch_value));
}

void QVDimmer::updateDimmerWidget(double value) {
    QString svg_string = svg_dimmer;
#if 0
   /* Round dimmer, uses dimmer-orange.svg, SmartVisu-style */
    svg_string.replace("#START#",QString::number(1+2*display_ratio,'f',3));
    svg_string.replace("#LEN#",QString::number(98-4*display_ratio,'f',3));
    svg_string.replace("#RAD#",QString::number(3*display_ratio,'f',3));
    svg_string.replace("#RAD_S#",QString::number(2,'f',3));
    svg_string.replace("#POS#",QString::number(value*92,'f',3));
#else
    svg_string.replace("#POS#",QString::number(value*100,'f',3));
    double slpos = value*100 - 1.5*display_ratio;
    if (slpos > 100-0.75*display_ratio) {
        slpos = 100-0.75*display_ratio;
    }
    svg_string.replace("#SLPOS#",QString::number(slpos,'f',3));
    svg_string.replace("#SLW#",QString::number(1.5*display_ratio,'f',3));
#endif

    w_dimmer->load(svg_string.toUtf8());

//qDebug() << "START" << 2*display_ratio << "LEN" << 92-4*display_ratio << "RAD" << 3*display_ratio << "POS" << value*100*display_ratio;
}

void QVDimmer::svgPressed() {
    mousePressEvent(0);
}

void QVDimmer::dimmerClicked(double xrel, double) {
//    qDebug() << "clicked" << xrel;
    dimmerDragged(xrel,0);
}

void QVDimmer::dimmerDragged(double xrel, double) {
//    qDebug() << "dragged" << xrel;
    if (xrel < 0.04) {
        xrel = 0.04;
    }
    if (xrel > 0.96) {
        xrel = 0.96;
    }
    updateDimmerWidget(xrel/0.92);
    QString item = getItemByAction("dimmer");
    if (item.isNull()) {
        return;
    }

    double value = max_value*(xrel-0.04)/0.92;
    if ((dim_value - value) < 1e-4) {
        return;
    }
    dim_value = value;

    if (data_type == type_int) {
#ifdef __GNUC__
        emit valueModified(item,QString::number((int)round(dim_value)));
#else /* MSVC */
        emit valueModified(item,QString::number((int)floor(dim_value+0.5)));
#endif
    } else {
        emit valueModified(item,QString::number(dim_value,'f',3));
    }
}

void QVDimmer::onValueChanged(QString item, QString value) {
    if (!items.contains(item)) {
        return;
    }
    QMap<QString,QVItem>::iterator iter = items.find(item);
    QString action = iter->getAction();
    if (action == "switch") {
        bool ok;
        int value_int = value.toInt(&ok);
        if (!ok) {
            return;
        }
        if (value_int == 0) {
            this->switch_value = false;
        } else {
            this->switch_value = true;
        }

        w_sw_icon->setStatus(switch_value);
    }
    if (action == "dimmer") {
        bool ok;
        double value_double = value.toDouble(&ok);
        if (!ok) {
            return;
        }
        dim_value = value_double;
        updateDimmerWidget(dim_value/max_value);
    }
}
