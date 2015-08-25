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

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QDebug>
#include <QMenuBar>
#include <QtXml>
#include <QThread>
#include <QMutex>
#include <QStandardPaths>
#include "qvmainwindow.h"
#include "qvdeployserver.h"

QTextStream *log_stream = 0;

void logOutput(QtMsgType type, const QMessageLogContext&, const QString &msg) {
    if (log_stream == 0) {
        return;
    }

    QString debugdate = QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss");
    switch (type) {
    case QtDebugMsg:
        debugdate += "[D]";
        break;
    case QtWarningMsg:
        debugdate += "[W]";
        break;
    case QtCriticalMsg:
        debugdate += "[C]";
        break;
    case QtFatalMsg:
        debugdate += "[F]";
    }
    (*log_stream) << debugdate << " " << msg << endl;

    if (QtFatalMsg == type) {
        abort();
    }
}


int main(int argc, char *argv[])
{
    bool use_deployserver = false;
    QVDeployServer *deployserver;

    /* Logging */
    for (int n=1; n < argc; n++) {
        if (QString(argv[n]) == "--debug") {
            if ((n == argc-1) || QString(argv[n+1]).startsWith("--")) {
                log_stream = new QTextStream(stderr);
                argc--;
                for (int m=n; m < argc; m++) {
                    argv[m] = argv[m+1];
                }
                break;
            } else {
                QString logfilename = QString(argv[n+1]);
                if (logfilename.toLower().endsWith(".xml")) {
                    log_stream = new QTextStream(stderr);
                    argc--;
                    for (int m=n; m < argc; m++) {
                        argv[m] = argv[m+1];
                    }
                    break;
                }
                QFile *log_file = new QFile(logfilename);
                if (log_file->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
                    log_stream = new QTextStream(log_file);
                }
                argc -= 2;
                for (int m=n; m < argc; m++) {
                    argv[m] = argv[m+2];
                }
                break;
            }
        }
    }
    qInstallMessageHandler(logOutput);

    QString xml_filename;

    /* Deployment Server */
    for (int n=1; n < argc; n++) {
        if (QString(argv[n]) == "--deployserver") {
            use_deployserver = true;
                argc--;
                for (int m=n; m < argc; m++) {
                    argv[m] = argv[m+1];
                }
                break;
        }
    }
    if (use_deployserver) {
#if QT_VERSION >= 0x050400
        QString appdir = QStandardPaths::locate(QStandardPaths::AppDataLocation,".",QStandardPaths::LocateDirectory);
#else
        QString appdir = QStandardPaths::locate(QStandardPaths::GenericConfigLocation,".",QStandardPaths::LocateDirectory);
#endif
        if (!QDir(appdir + "/QVisu").exists()) {
            if (!QDir(appdir).mkpath("QVisu")) {
                qFatal(strdup(QString("Cannot create deployment dir " + appdir + "/QVisu").toLocal8Bit().data()));
                return(0);
            }
        }
        xml_filename = appdir + "/QVisu/qvisu.xml";

        deployserver = new QVDeployServer(appdir + "/QVisu");
    }

    QApplication a(argc, argv);

    do {
        /* Functionality */
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
        if (!use_deployserver) {
            if (argc > 1) {
                xml_filename = argv[1];
            } else {
                qFatal("No XML filename given, and deployment mode not active. Use --deployserver to configure from QVisuDesigner");
                return 0;
            }
        }
        xml_file.setFileName(xml_filename);

        if (!xml_file.open(QIODevice::ReadOnly)) {
            qDebug() << "XML configuration file " + xml_file.fileName() + " not found";
            return 1;
        }

        if (!xml_doc.setContent(&xml_file)) {
            qDebug() << "Cannot parse XML configuration file " + xml_file.fileName();
            if (!use_deployserver) {
                return 1;
            }
        }

        QDomElement root_elem = xml_doc.documentElement();
        if (root_elem.isNull() || root_elem.nodeName().compare("QVisu")) {
            if (!use_deployserver) {
            return 2;
}
        }
        QDomElement dim_elem = root_elem.firstChildElement("dimensions");
        if (dim_elem.isNull() && !use_deployserver) {
            return 3;
        }
        QDomElement dim_x_elem = dim_elem.firstChildElement("w");
        QDomElement dim_y_elem = dim_elem.firstChildElement("h");
        if ((dim_x_elem.isNull() || dim_y_elem.isNull()) && !use_deployserver) {
            return 4;
        }
        uint dim_x = dim_x_elem.text().toUInt();
        uint dim_y = dim_y_elem.text().toUInt();
        if (((dim_x == 0) || (dim_y == 0)) && !use_deployserver) {
            return 5;
        }

        QSize size_force;
        QDomElement size_force_elem = root_elem.firstChildElement("force-screensize");
        if (!size_force_elem.isNull()) {
            QStringList l = size_force_elem.text().split("x");
            if (l.count() == 2) {
                bool ok1, ok2;
                int fw = l[0].toInt(&ok1);
                int fh = l[1].toInt(&ok2);
                if (ok1 && ok2 && (fw > 0) && (fh > 0)) {
                    size_force = QSize(fw,fh);
                }
            }
        }

        QDomNodeList containers = root_elem.elementsByTagName("container");
        if ((containers.count() == 0) && !use_deployserver) {
            return 6;
        }
        bool ok = false;
        for (int n=0; n < containers.count(); n++) {
            if (containers.at(n).attributes().namedItem("name").isNull()) {
                if (!use_deployserver) {
                    return 7;
                }
            } else if (containers.at(n).attributes().namedItem("name").nodeValue() == "main") {
                ok = true;
            }
        }
        if (!ok && !use_deployserver) {
            return 8;
        }

        QDomNodeList xml_paths = root_elem.elementsByTagName("path");
        QStringList paths;
        for (int n=0; n < xml_paths.count(); n++) {
            paths.append(xml_paths.at(n).toElement().text());
        }
        a.setProperty("paths",paths);

        QVMainWindow w(dim_x,dim_y,root_elem);
        if (size_force.isValid()) {
            w.setFixedSize(size_force);
        }
        if (use_deployserver) {
            QObject::connect(deployserver,SIGNAL(restart()),&w,SLOT(close()));
        }

        xml_file.close();

        w.setStatusBar(0);
        w.setMenuWidget(0);
        w.showFullScreen();

        a.exec();
    } while (use_deployserver);
    return 0;
}
