/*
* Copyright (C) 2010, Muguntharaj Subramanian
*
* This file is part of eKalappai.
*
* eKalappai is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* eKalappai is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License version 3
* along with eKalappai.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QtGui>
#include <QtWidgets>

#include "window.h"
#include <windows.h>


int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(ekalappai);

    QApplication app(argc, argv);

    QPixmap pix(":/images/intro.png");

    QSplashScreen splash(pix);
    splash.show();
    Sleep(2000);
    splash.hide();

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        return 1;
    }
    QApplication::setQuitOnLastWindowClosed(false);

    Window window;
    return app.exec();

}


