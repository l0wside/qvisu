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

#include <math.h>
#include <limits>
#include <cmath>
#include "qvplot.h"

QVPlot::QVPlot(QDomElement xml_desc, QString container, QWidget *parent) :
    QVElement(xml_desc, container, parent)
{
    if (w <= 0) {
        w = 4;
    }

    if (h <= 0) {
        h = 2;
    }

    bool ok;

    has_axis1 = has_axis2 = false;

    QDomElement xml_elem;

    xml_elem = xml_desc.firstChildElement("time-min");
    if (!xml_elem.isNull()) {
        series_start = xml_elem.text();
    } else {
        series_start = QStringLiteral("24h");
    }

    xml_elem = xml_desc.firstChildElement("background-color");
    if (!xml_elem.isNull()) {
        background_color = colorString(xml_elem.text());
    }

    common_zero = false;
    if (xml_desc.hasAttribute("common-zero")) {
        common_zero = true;
        if ((xml_desc.attribute("common-zero").toLower() == "false") || (xml_desc.attribute("common-zero") == "0")) {
            common_zero = false;
        }
    }

    xgrid = false;
    if (xml_desc.hasAttribute("xgrid")) {
        xgrid = true;
        if ((xml_desc.attribute("xgrid").toLower() == "false") || (xml_desc.attribute("xgrid") == "0")) {
            xgrid = false;
        }
    }


    grid1 = grid2 = false;

    /* Plots on axis 1 */
    QDomElement xml_axis1 = xml_desc.firstChildElement("axis1");
    if (!xml_axis1.isNull()) {
        ok = false;
        axis1_min = std::numeric_limits<double>::quiet_NaN();
        axis1_max = std::numeric_limits<double>::quiet_NaN();

        if (xml_axis1.hasAttribute("ymin")) {
            double v = xml_axis1.attribute("ymin").toDouble(&ok);
            if (ok) {
                axis1_min = v;
            }
        }
        if (xml_axis1.hasAttribute("ymax")) {
            double v = xml_axis1.attribute("ymax").toDouble(&ok);
            if (ok) {
                axis1_max = v;
            }
        }
        if (xml_axis1.hasAttribute("grid")) {
            grid1 = true;
            if ((xml_axis1.attribute("grid").toLower() == "false") || (xml_axis1.attribute("grid") == "0")) {
                grid1 = false;
            }
        }


        for (xml_elem = xml_axis1.firstChildElement("item"); !xml_elem.isNull(); xml_elem = xml_elem.nextSiblingElement("item")) {
            has_axis1 = true;
            addPlot(xml_elem,1);
        }
    }

    /* Plots on axis 2 */
    QDomElement xml_axis2 = xml_desc.firstChildElement("axis2");
    if (!xml_axis2.isNull()) {
        ok = false;
        axis2_min = std::numeric_limits<double>::quiet_NaN();
        axis2_max = std::numeric_limits<double>::quiet_NaN();

        if (xml_axis2.hasAttribute("ymin")) {
            double v = xml_axis2.attribute("ymin").toDouble(&ok);
            if (ok) {
                axis2_min = v;
            }
        }
        if (xml_axis2.hasAttribute("ymax")) {
            double v = xml_axis2.attribute("ymax").toDouble(&ok);
            if (ok) {
                axis2_max = v;
            }
        }
        if (xml_axis2.hasAttribute("grid")) {
            grid2 = true;
            if ((xml_axis2.attribute("grid").toLower() == "false") || (xml_axis2.attribute("grid") == "0")) {
                grid2 = false;
            }
        }
        for (xml_elem = xml_axis2.firstChildElement("item"); !xml_elem.isNull(); xml_elem = xml_elem.nextSiblingElement("item")) {
            has_axis2 = true;
            addPlot(xml_elem,2);
        }
    }

    plot_widget = new QVSvgWidget(this);

    w_title = new QLabel(this);
    if (!xml_desc.firstChildElement("text").isNull()) {
        w_title->setText(xml_desc.firstChildElement("text").text());
    }
    w_icon = 0;
    if (!xml_desc.firstChildElement("icon").isNull()) {
        w_icon = new QVIconWidget(findFilePath(xml_desc.firstChildElement("icon").text()),this);
    }
}

void QVPlot::addPlot(QDomElement xml_elem, int axis) {
    if (!xml_elem.hasAttribute("action") || (xml_elem.attribute("action").isEmpty()) || (xml_elem.text().isEmpty())) {
        return;
    }
    series_t *series_desc = new series_t();
    series_desc->type = xml_elem.attribute("action");
    if (xml_elem.hasAttribute("name")) {
        series_desc->name = xml_elem.attribute("name");
    }

    if (xml_elem.hasAttribute("color")) {
        series_desc->color = colorString(xml_elem.attribute("color"));
    } else {
        series_desc->color = QStringLiteral("#FFFFFF");
    }
    series_desc->axis = axis;
    series.insert(xml_elem.text(),series_desc);
}


void QVPlot::resizeEvent(QResizeEvent * event) {
    if ((QWidget::height() == event->oldSize().height()) && (QWidget::width() == event->oldSize().width())) {
        return;
    }

    int icon_height = (int)(max_label_height() * height());
    if (h > 2) {
        icon_height = (int)(icon_height*2/h);
    }
    int icon_width;
    if (w_icon) {
         icon_width = icon_height / w_icon->aspectRatio();
         if (icon_width > max_icon_width() * width()) {
             icon_width = (int)(max_icon_width() * width());
             icon_height = (int)(icon_width * w_icon->aspectRatio());
         }
         w_icon->resize(icon_width,icon_height);
         w_icon->move(ofs_x()+(int)((width()-icon_width-w_title->sizeHint().width())/2),ofs_y()+height()-icon_height);
    } else {
        icon_width = 0;
        icon_height = w_title->sizeHint().height();
    }
    w_title->move(ofs_x()+(int)((width()+icon_width-w_title->sizeHint().width())/2),ofs_y()+height()-(int)((icon_height+w_title->sizeHint().height())/2));


    plot_widget->resize(width(),height()-icon_height-padding);
    plot_widget->move(ofs_x(),ofs_y());
    redrawSeries();
}

void QVPlot::mousePressEvent(QMouseEvent*) {
    qDebug() << "plot mousepress";

}

void QVPlot::onInitCompleted() {
    for (QMap<QString,series_t*>::iterator iter = series.begin(); iter != series.end(); iter++) {
        emit requestSeries(iter.key(),(*iter)->type,series_start);
    }
}

void QVPlot::onSeriesReceived(QString item,QMap<double,double> data) {
    qDebug() << "series received: " << item << data.count();
    if (!series.contains(item)) {
        return;
    }
    qDebug() << "evaluating series " + item;

    series.value(item)->data = data;

    redrawSeries();
}

/** Redraws the SVG with all series defined in the XML config.
 * Steps: calculate y scaling and offset for defined axes (target range: 0...1).
 * Then, calculate scaling and offset for all graphs which have no axis.
 * Finally, generate the SVG and draw it.
 * Unassigned variables are set to NaN.
 */
void QVPlot::redrawSeries() {
    if (series.count() == 0) {
        return;
    }

    double ymin1 = std::numeric_limits<double>::infinity(),
            ymax1 = -std::numeric_limits<double>::infinity(),
            ymin2 = std::numeric_limits<double>::infinity(),
            ymax2 = -std::numeric_limits<double>::infinity();

    double xmin = std::numeric_limits<double>::infinity(), xmax = -std::numeric_limits<double>::infinity();

    for (QMap<QString,series_t*>::iterator iter = series.begin(); iter != series.end(); iter++) {
        series_t *s = *iter;

        /* Determine min/max values for both axes */
        for (QMap<double,double>::iterator iter_v = s->data.begin(); iter_v != s->data.end(); iter_v++) {
            if (xmin > iter_v.key()) {
                xmin = iter_v.key();
            }
            if (xmax < iter_v.key()) {
                xmax = iter_v.key();
            }
            switch(s->axis) {
            case 1:
                if (*iter_v > ymax1) {
                    ymax1 = *iter_v;
                }
                if (*iter_v < ymin1) {
                    ymin1 = *iter_v;
                }
                break;
            case 2:
                if (*iter_v > ymax2) {
                    ymax2 = *iter_v;
                }
                if (*iter_v < ymin2) {
                    ymin2 = *iter_v;
                }
                break;
            default:
                break;
            }
        }

        if (xmin >= xmax) {
            return;
        }

        /* Use default values unless they are NaN. (a == a) is a NaN test */
        if (axis1_min == axis1_min) {
            ymin1 = axis1_min;
        }
        if (axis1_max == axis1_max) {
            ymax1 = axis1_max;
        }
        if (axis2_min == axis2_min) {
            ymin2 = axis2_min;
        }
        if (axis2_max == axis2_max) {
            ymax2 = axis2_max;
        }
    }

    /* Calculate scaling and offset for both axes */
    double yrange1 = ymax1-ymin1;
    if (yrange1 < 1e-3) {
        yrange1 = 1;
    }
    double yrange2 = ymax2-ymin2;
    if (yrange2 < 1e-3) {
        yrange2 = 1;
    }

    double scale1 = 1.0, scale2 = 1.0, ofs1 = 0.0, ofs2 = 0.0, axis_step1 = 1.0, axis_step2 = 1.0;
    int axis_pow1, axis_pow2;

    if (has_axis1 && has_axis2 && common_zero) {
        /* Scale to common y axis.
         * Cases:
         *   (a) axis1 and axis2 cross 0
         *   (b) axis1 > 0, axis2 < 0  [each axis in its own half]
         *   (c) axis1 > 0, axis2 > 0  or  axis1 < 0, axis2 < 0  [0 never crossed]
         *   (d) axis1 crosses 0, axis2 > 0   or   axis2 crosses 0, axis1 < 0  [only axis1 crosses 0]
         *   (e) axis1 > 0, axis2 crosses 0   or   axis1 < 0, axis2 crosses 0  [only axis2 crosses 0]
         */
        bool has_zero1 = false, has_zero2 = false;
        if ((ymin1 <= 0) && (ymax1 >= 0)) {
            has_zero1 = true;
        }
        if ((ymin2 <= 0) && (ymax2 >= 0)) {
            has_zero2 = true;
        }

        if (has_zero1 && has_zero2) {
            /* Case (a) */
            qDebug() << "Case (a)";
            scale1 = 1.0/yrange1;
            scale2 = 1.0/yrange2;
            double ymin1_scale = ymin1 * scale1;
            double ymax1_scale = ymax1 * scale1;
            double ymin2_scale = ymin2 * scale2;
            double ymax2_scale = ymax2 * scale2;
            double ymin_scale, ymax_scale;
            if (ymin1_scale < ymin2_scale) {
                ymin_scale = ymin1_scale;
            } else {
                ymin_scale = ymin2_scale;
            }
            if (ymax1_scale > ymax2_scale) {
                ymax_scale = ymax1_scale;
            } else {
                ymax_scale = ymax2_scale;
            }

            double rescale;
            if (ymax_scale - ymin_scale > 1e-8) {
                rescale = 1.0/(ymax_scale-ymin_scale);
            } else {
                rescale = 1.0;
            }
            scale1 *= rescale;
            scale2 *= rescale;
            ofs1 = ofs2 = -ymin_scale * rescale;

            double axis_step = (ymax_scale-ymin_scale)/10.0;
            axis_pow1 = (int)ceil(log10(axis_step/scale1));
            axis_step1 = exp(log(10)*axis_pow1);
            if (axis_step1*scale1 > 0.2) {
                axis_step1 /= 2;
                axis_pow1--;
            }
            axis_pow2 = (int)ceil(log10(axis_step/scale2));
            axis_step2 = exp(log(10)*axis_pow2);
            if (axis_step2*scale2 > 0.2) {
                axis_step2 /= 2;
                axis_pow2--;
            }
        } else if (!has_zero1 && !has_zero2 && (((ymin1 < 0.0) && (ymax2 > 0.0)) || ((ymin2 < 0.0) && (ymax2 > 0.0)))) {
            /* Case (b) */
            qDebug() << "Case (b)";
            scale1 = 0.5/yrange1;
            scale2 = 0.5/yrange2;
            ofs1 = ofs2 = 0.5;

            axis_pow1 = (int)ceil(log10(yrange1/5.0));
            axis_step1 = exp(log(10)*axis_pow1);
            if (axis_step1 > 0.1) {
                axis_step1 /= 2;
                axis_pow1--;
            }
            axis_pow2 = (int)ceil(log10(yrange2/5.0));
            axis_step2 = exp(log(10)*axis_pow2);
            if (axis_step2 > 0.1) {
                axis_step2 /= 2;
                axis_pow2--;
            }
        } else if (!has_zero1 && !has_zero2) {
            /* Case (c) */
            qDebug() << "Case (c)";
            if (ymin1 > 0.0) {
                ymin1 = 0.0;
            }
            if (ymax1 < 0.0) {
                ymax1 = 0.0;
            }
            yrange1 = (ymax1-ymin1);
            if (yrange1 < 1e-8) {
                yrange1 = 1.0;
            }

            if (ymin2 > 0.0) {
                ymin2 = 0.0;
            }
            if (ymax2 < 0.0) {
                ymax2 = 0.0;
            }
            yrange2 = (ymax2-ymin2);
            if (yrange2 < 1e-8) {
                yrange2 = 1.0;
            }

            scale1 = 1.0/yrange1;
            scale2 = 1.0/yrange2;
            if (ymin1 == 0.0) {
                ofs1 = ofs2 = 0.0;
            } else {
                ofs1 = ofs2 = 1.0;
            }

            axis_pow1 = (int)ceil(log10(yrange1/10.0));
            axis_step1 = exp(log(10)*axis_pow1);
            if (axis_step1 > 0.1) {
                axis_step1 /= 2;
                axis_pow1--;
            }
            axis_pow2 = (int)ceil(log10(yrange2/10.0));
            axis_step2 = exp(log(10)*axis_pow2);
            if (axis_step2 > 0.1) {
                axis_step2 /= 2;
                axis_pow2--;
            }
        } else if (has_zero1 || has_zero2) {
            /* Case (d) and (e) */
            qDebug() << "Case (d)/(e)";
            if (ymin1 > 0.0) {
                ymin1 = 0.0;
            }
            if (ymax1 < 0.0) {
                ymax1 = 0.0;
            }
            yrange1 = (ymax1-ymin1);
            if (yrange1 < 1e-8) {
                yrange1 = 1.0;
            }

            if (ymin2 > 0.0) {
                ymin2 = 0.0;
            }
            if (ymax2 < 0.0) {
                ymax2 = 0.0;
            }
            yrange2 = (ymax2-ymin2);
            if (yrange2 < 1e-8) {
                yrange2 = 1.0;
            }
            /* ..rest is like (a) */
            scale1 = 1.0/yrange1;
            scale2 = 1.0/yrange2;
            double ymin1_scale = ymin1 * scale1;
            double ymax1_scale = ymax1 * scale1;
            double ymin2_scale = ymin2 * scale2;
            double ymax2_scale = ymax2 * scale2;
            double ymin_scale, ymax_scale;
            if (ymin1_scale < ymin2_scale) {
                ymin_scale = ymin1_scale;
            } else {
                ymin_scale = ymin2_scale;
            }
            if (ymax1_scale > ymax2_scale) {
                ymax_scale = ymax1_scale;
            } else {
                ymax_scale = ymax2_scale;
            }

            qDebug() << "range1" << yrange1 << "range2" << yrange2 << "scale1" << scale1 << "scale2" << scale2 << "yrange scaled" << ymin_scale << ymax_scale;
            double rescale;
            if (ymax_scale - ymin_scale > 1e-8) {
                rescale = 1.0/(ymax_scale-ymin_scale);
            } else {
                rescale = 1.0;
            }
            scale1 *= rescale;
            scale2 *= rescale;
            ofs1 = ofs2 = -ymin_scale * rescale;
            qDebug() << "rescale" << rescale << "scales" << scale1 << scale2 << "ofs" << ofs1 << ofs2;

            double axis_step = (ymax_scale-ymin_scale)/10.0;
            axis_pow1 = (int)ceil(log10(axis_step/scale1));
            axis_step1 = exp(log(10)*axis_pow1);
            if (axis_step1*scale1 > 0.2) {
                axis_step1 /= 2;
                axis_pow1--;
            }
            axis_pow2 = (int)ceil(log10(axis_step/scale2));
            qDebug() << "** AS" << axis_step << scale2 << axis_pow2;
            axis_step2 = exp(log(10)*axis_pow2);
            if (axis_step2*scale2 > 0.2) {
                axis_step2 /= 2;
                axis_pow2--;
            }
            qDebug() << axis_step2;
        }
    } else {
        /* No common x axis. Scale both y axes separately */
        qDebug() << "No common axis";
        if (has_axis1) {
            scale1 = 1.0/yrange1;
            ofs1 = -ymin1*scale1;

            axis_pow1 = (int)ceil(log10(yrange1/10.0));
            axis_step1 = exp(log(10)*axis_pow1);
            if (axis_step1 > 0.2) {
                axis_step1 /= 2;
                axis_pow1--;
            }
        }
        if (has_axis2) {
            scale2 = 1.0/yrange2;
            ofs2 = -ymin2*scale2;

            axis_pow2 = (int)ceil(log10(yrange2/10.0));
            axis_step2 = exp(log(10)*axis_pow2);
            if (axis_step2 > 0.2) {
                axis_step2 /= 2;
                axis_pow2--;
            }
        }
    }

    double w = plot_widget->width();
    double h = plot_widget->height() - label_size - padding;
    double xscale = w/(xmax-xmin);
    ofs1 *= h;
    ofs2 *= h;
    scale1 *= h;
    scale2 *= h;
    QString color_scale1, color_scale2;

    QString svg = "<svg version=\"1.1\" viewBox=\"0 0 " + QString::number(w,'f',3) + " " + QString::number(h+label_size+padding,'f',3) +"\">\n";

    if (!background_color.isEmpty()) {
        svg += "<path style=\"stroke:none;fill:" + background_color + ";\" d=\"M0,0 l" + QString::number(w,'f',3) + ",0 l0," + QString::number(h,'f',3) + " l" + QString::number(-w,'f',3) + ",0z\" />\n";
    }

    for (QMap<QString,series_t*>::iterator iter = series.begin(); iter != series.end(); iter++) {
        series_t *s = *iter;
        if (s->data.isEmpty()) {
            continue;
        }
        double scale, ofs;
        switch (s->axis) {
        case 1:
            scale = scale1;
            if (color_scale1.isEmpty()) {
                color_scale1 = colorString(s->color);
            }
            ofs = ofs1;
            break;
        case 2:
            scale = scale2;
            if (color_scale2.isEmpty()) {
                color_scale2 = colorString(s->color);
            }
            ofs = ofs2;
            break;
        default:
            continue;
        }

        /* Plot actual graph */
        svg += "<polyline style=\"stroke-width:1;stroke:" + s->color + ";fill:none\" id=\"" + iter.key() + "\" ";
        svg += "points=\"";
        for (QMap<double,double>::iterator iter_s = s->data.begin(); iter_s != s->data.end(); iter_s++) {
           svg += QString::number((iter_s.key()-xmin)*xscale,'f',3) + "," + QString::number(h-((*iter_s)*scale+ofs)) + " ";
        }
        svg += "\" />\n";
        /* Label it */
        svg += "<text style=\"fill:" + s->color + ";font-size:10px\" text-anchor=\"end\" "
                + "x=\"" + QString::number(w-30,'f',3) + "\" y=\"" + QString::number(h-(s->data.last()*scale+ofs),'f',3) + "\">"
                + s->name
                + "</text>\n";
    }

    /* x axis (or x axes) */
    if (common_zero) {
        svg += "<path style=\"stroke-width:2;stroke:#000000;fill:none\" d=\"M0," + QString::number(h-ofs1,'f',3) + " l" + QString::number(w,'f',3) + ",0\" />\n";
    } else {
        if (has_axis1) {
            svg += "<path style=\"stroke-width:2;stroke:" + color_scale1 + ";fill:none\" d=\"M0," + QString::number(h-ofs1,'f',3) + " l" + QString::number(w,'f',3) + ",0\" />\n";
        }
        if (has_axis2) {
            svg += "<path style=\"stroke-width:2;stroke:" + color_scale2 + ";fill:none\" d=\"M0," + QString::number(h-ofs2,'f',3) + " l" + QString::number(w,'f',3) + ",0\" />\n";
        }
    }

    if (has_axis1) {
        /* Labels on left axis */
        double axis_val = axis_step1*ceil(ymin1/axis_step1);
        while (axis_val*scale1+ofs1 > label_size+padding+axis_step1) {
            axis_val -= axis_step1;
        }
        double pow = -axis_pow1;
        if (pow < 0) {
            pow = 0;
        }
        while (axis_val*scale1+ofs1 < h) {
            if (fabs(axis_val) < 1e-8) {
                axis_val = 1e-8;
            }
            int text_y_pos;
            if (grid1) {
                svg += "<path style=\"stroke-width:1;stroke-dasharray:1,2;stroke:" + color_scale1 + ";fill:none\" d=\"M0," + QString::number(h-(axis_val*scale1+ofs1),'f',3) + " l" + QString::number(w,'f',3) + ",0\" />\n";
                text_y_pos = (int)(h-axis_val*scale1-ofs1);
            } else {
                svg += "<path style=\"stroke-width:2;stroke:" + color_scale1 + ";fill:none\" d=\"M0," + QString::number(h-(axis_val*scale1+ofs1),'f',3) + "l5,0\" />\n";
                text_y_pos = (int)(h+label_size/2-axis_val*scale1-ofs1);
            }
            if ((text_y_pos > label_size*0.5) && (text_y_pos < h-label_size*0.5)) {
                svg += "<text style=\"fill:" + color_scale1 + ";font-size:" + QString::number(label_size,'f',1) + "px\" x=\"8\" y=\"" + QString::number(text_y_pos,'f',3) + "\">" + QString::number(axis_val,'f',pow) + "</text>\n";
            }
            axis_val += axis_step1;
        }
        svg += "<path style=\"stroke-width:4;stroke:#000000;fill:none\" d=\"M0,0L0," + QString::number(h,'f',3) + "\" />\n";
    }
    if (has_axis2) {
        /* Labels on right axis */
        double axis_val = axis_step2*ceil(ymin2/axis_step2);

        while (axis_val*scale2+ofs2 > label_size+padding+axis_step2*scale2) {
            axis_val -= axis_step2;
        }
        qDebug() << "AV2" << axis_val << axis_val*scale2+ofs2;
        double pow = -axis_pow2;
        if (pow < 0) {
            pow = 0;
        }
        while (axis_val*scale2+ofs2 < h) {
            if (fabs(axis_val) < 1e-8) {
                axis_val = 1e-8;
            }
            int text_y_pos;
            if (grid2) {
                svg += "<path style=\"stroke-width:1;stroke-dasharray:1,2;stroke:" + color_scale2 + ";fill:none\" d=\"M0," + QString::number(h-(axis_val*scale1+ofs1),'f',3) + " l" + QString::number(w,'f',3) + ",0\" />\n";
                text_y_pos = (int)(h-axis_val*scale1-ofs1);
            } else {
                svg += "<path style=\"stroke-width:2;stroke:" + color_scale2 + ";fill:none\" d=\"M" + QString::number(w,'f',3) + "," + QString::number(h-(axis_val*scale2+ofs2),'f',3) + "l-5,0\" />\n";
                text_y_pos = (int)(h+label_size/2-axis_val*scale2-ofs2);
            }

            if ((text_y_pos > label_size*0.5) && (text_y_pos < h-label_size*0.5)) {
                svg += "<text style=\"fill:" + color_scale2 + ";font-size:" + QString::number(label_size,'f',1) + "px\" text-anchor=\"end\" x=\"" + QString::number(w-8,'f',3) + "\" y=\"" + QString::number(text_y_pos,'f',3) + "\">" + QString::number(axis_val,'f',pow) + "</text>\n";
            }
            axis_val += axis_step2;
        }

        svg += "<path style=\"stroke-width:4;stroke:#000000;fill:none\" d=\"M" + QString::number(w,'f',3) + ",0L" + QString::number(w,'f',3) + "," + QString::number(h,'f',3) + "\" />\n";
    }

    /* Label time axis */
    double timespan = (xmax-xmin)/1000.0; // timespan in seconds
    int ticks = (int)1.5*QVElement::w;
    double timetick = timespan/(ticks-1);
    QDateTime d_time_start = QDateTime::fromMSecsSinceEpoch(xmin);
//    qDebug() << "tick" << ticks << timetick << "start" << d_time_start;
    int hour = d_time_start.time().hour();
    int minute = d_time_start.time().minute();

    if (timetick < 60) {
        timetick = 60;
    } else if (timetick < 300) {
        timetick = 300;
        d_time_start = d_time_start.addSecs(300-fmod(minute*60,300));
    } else if (timetick < 900) {
        timetick = 900;
        d_time_start = d_time_start.addSecs(900-fmod(minute*60,900));
    } else if (timetick < 1800) {
        timetick = 1800;
        d_time_start = d_time_start.addSecs(1800-fmod(minute*60,1800));
    } else if (timetick < 3600) {
        timetick = 3600;
        if (minute > 0) {
            d_time_start = d_time_start.addSecs(3600-fmod(minute*60,3600));
        }
    } else if (timetick < 24*3600) {
        timetick += (3600.0-fmod(timetick,3600.0));
        if (minute > 0) {
            d_time_start = d_time_start.addSecs(3600-fmod(minute*60,3600));
        }
    } else {
        timetick += (24.0*3600.0-fmod(timetick,24.0*3600.0));
        if (minute > 0) {
            d_time_start = d_time_start.addSecs(3600-fmod(minute*60,3600));
        }
        if (hour > 0) {
            d_time_start = d_time_start.addSecs(24*3600-fmod(hour*24*60,24*3600));
        }
    }

    double timepos = d_time_start.toMSecsSinceEpoch();
    while (timepos < xmax) {
        double pos = (timepos-xmin)/(timespan*1000.0)*w;
        timepos += 1000.0*timetick;
        if (xgrid) {
            svg += "<path style=\"stroke-width:1;stroke:#808080;stroke-dasharray:1,2;fill:none;\" d=\"M" + QString::number(pos,'f',3) + ",0" + " l0," + QString::number(h,'f',3) + "\" />\n";
        } else {
            svg += "<path style=\"stroke-width:1;stroke:#000000;fill:none;\" d=\"M" + QString::number(pos,'f',3) + "," + QString::number(h,'f',3) + "l0,-5\" />\n";
        }
        QDateTime dt = QDateTime::fromMSecsSinceEpoch(timepos);
        QString dt_string;
        if (timetick < 24.0*3600.0) {
            if (dt.date().day() != QDateTime::currentDateTime().date().day()) {
                dt_string = dt.toString("ddd hh:mm");
            } else {
                dt_string = dt.toString("hh:mm");
            }
        } else {
            dt_string = dt.toString("dd. MMM");
        }
        svg += "<text x=\"" + QString::number(pos,'f',3) + "\" y=\"" + QString::number(h+padding+(int)(label_size/2),'f',3) + "\" style=\"font-size:" + QString::number(label_size,'f',1) + "px;\" >" + dt_string + "</text>\n";
    }

    qDebug() << "start" << d_time_start;

    svg += "</svg>";

//    qDebug() << svg;
    plot_widget->load(svg.toUtf8());



}
