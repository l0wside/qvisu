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

#ifndef QVELEMENT_H
#define QVELEMENT_H

#include <QFrame>
#include <QPushButton>
#include <QList>
#include <QString>
#include <QtXml>
#include <QMap>
#include <QResizeEvent>
#include "qvitem.h"


/** The base class for all elements which can be displayed. Actual elements need to be subclassed from QVElement.\n
 * Takes care of position, size, container and list of items.\n
 * All other functionaliy is to be implemented in the subclass.
 */
class QVElement : public QFrame
{
    Q_OBJECT
public:
    /** Basic constructor for all elements.
     * \param xml_desc The XML ```<element>``` node
     * \param container The parent container
     * \param parent Parent widget, as in all Qt widgets
     *
     * Will determine width, height, position, color and active color.
     * Extracts all items which are direct children of the ```<element>``` node.
     * They are then accessible via getItemList()
     */
    explicit QVElement(QDomElement xml_desc,QString container,QWidget *parent = 0);

    /** Return a list of all items which have been configured inside the ```<element>``` node. */
    QList<QString> getItemList();
    /** Returns x,y,w,h in this order. */
    QList<int> getGeometry();

    /** Try to find the file name in one of the directories specified by <path> */
    static QString findFilePath(QString name);
    /** Convert a Windows 8 color string to the #xxxxxx format. */
    static QString colorString(QString);

protected:
    /** Unless you use a layout, overload this method to place Qt widgets inside the element */
    virtual void resizeEvent(QResizeEvent * event);
    /** Returns the item name which corresponds to the action.
     * \param action The action as defined in <item action="...">
     * \param index If the action is contained several times in the ```<element>``` node, retrieves the n-th
     * entry. Counting starts at 0.
     * \retval An empty string if the action is not configured or index is out of range. */
    QString getItemByAction(QString action, int index = 0);
    /** Width in the grid. May be overwritten in the constructor of the derived class. */
    int w;
    /** Height in the grid. May be overwritten in the constructor of the derived class. */
    int h;
    /** Container in which the element is contained */
    QString container;
    /** List of items. Key is the item name, content is a QVItem */
    QMap<QString,QVItem> items;

    /** Height of the content in pixels */
    int height();
    /** Width of the content in pixels */
    int width();
    /** Content margin (x direction) in pixels */
    int ofs_x();
    /** Content margin (y direction) in pixels */
    int ofs_y();

    /* Defines (disguised as methods) */
    double max_icon_width() { return 0.2; } /* Ratio of icon width to total width */
    double max_label_height() { return 0.2; } /* Ratio of maximum label (icon+text) height to total height */

signals:
    /** Emitted when a value has been changed actively in the element */
    void valueModified(QString,QString);
    /** Emitted to request a series. Used only by the plot element. */
    void requestSeries(QString item, QString series_type, QString start);

public slots:
    /** Handle incoming values. Will also be called after the element itself emits valueModified().
     * To be overloaded in a child class.
     */
    virtual void onValueChanged(QString,QString);
    /** Handle an incoming series. To be overloaded in QVPlot */
    virtual void onSeriesReceived(QString,QMap<double,double>);
    /** Called on ontainer change. Handled by QVElement(), to be overloaded only with reason (e.g. video element) */
    void onContainerChanged(QString);
    /** Called when the websocket connection is established. */
    virtual void onInitCompleted();

protected:
    QString color, active_color;

private:
    int x;
    int y;

};

QVElement *createQVElement(QDomElement xml_desc, QString container, QWidget *parent = 0);

#endif // QVELEMENT_H
