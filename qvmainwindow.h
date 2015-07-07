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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtXml>
#include <QMap>
#include <QList>
#include <QThread>
#include "qvelement.h"
#include "qvdriver.h"

namespace Ui {
class MainWindow;
}

class QVMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit QVMainWindow(uint dim_x, uint dim_y, QDomElement xml_data, QWidget *parent = 0);
    ~QVMainWindow();

signals:
    void valueGenerated(QString,QString);

protected slots:
    void onTimerTimeout();

private:
    void resizeEvent(QResizeEvent *);
    int dim_x, dim_y;

    Ui::MainWindow *ui;

    QList<QVElement*> elements;

    QVDriver *qv_driver;
    static const int spacing = 5;

    QTimer *timer;

};

#endif // MAINWINDOW_H
