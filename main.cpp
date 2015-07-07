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
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QDebug>
#include <QMenuBar>
#include <QtXml>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile css_file;
    if (argc > 2) {
        css_file.setFileName(argv[2]);
    } else {
        css_file.setFileName(QStringLiteral(":/qtvisu.css"));
    }

    if (css_file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&css_file);
        a.setStyleSheet(stream.readAll());
        css_file.close();
    }

    QDomDocument xml_doc;
    QFile xml_file;
    if (argc > 1) {
        xml_file.setFileName(argv[1]);
    } else {
        return 0;
    }
    if (!xml_file.open(QIODevice::ReadOnly)) {
        qDebug() << "XML configuration file " + xml_file.fileName() + " not found";
        return 1;
    }

    if (!xml_doc.setContent(&xml_file)) {
        qDebug() << "Cannot parse XML configuration file " + xml_file.fileName();
        return 1;
    }

    QDomElement root_elem = xml_doc.documentElement();
    if (root_elem.isNull() || root_elem.nodeName().compare("QVisu")) {
        return 2;
    }
    QDomElement dim_elem = root_elem.firstChildElement("dimensions");
    if (dim_elem.isNull()) {
        return 3;
    }
    QDomElement dim_x_elem = dim_elem.firstChildElement("w");
    QDomElement dim_y_elem = dim_elem.firstChildElement("h");
    if (dim_x_elem.isNull() || dim_y_elem.isNull()) {
        return 4;
    }
    uint dim_x = dim_x_elem.text().toUInt();
    uint dim_y = dim_y_elem.text().toUInt();
    if ((dim_x == 0) || (dim_y == 0)) {
        return 5;
    }

    QDomNodeList containers = root_elem.elementsByTagName("container");
    if (containers.count() == 0) {
        return 6;
    }
    bool ok = false;
    for (int n=0; n < containers.count(); n++) {
        if (containers.at(n).attributes().namedItem("name").isNull()) {
            return 7;
        } else if (containers.at(n).attributes().namedItem("name").nodeValue() == "main") {
            ok = true;
        }
    }
    if (!ok) {
        return 8;
    }

    QDomNodeList xml_paths = root_elem.elementsByTagName("path");
    QStringList paths;
    for (int n=0; n < xml_paths.count(); n++) {
        paths.append(xml_paths.at(n).toElement().text());
    }
    a.setProperty("paths",paths);

    QVMainWindow w(dim_x,dim_y,root_elem);

    xml_file.close();

    w.setStatusBar(0);
    w.setMenuWidget(0);
    w.showFullScreen();

    return a.exec();
}
