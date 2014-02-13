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

#include <QtWidgets>
#include <QSplashScreen>
#include <QWindow>
#include <QSystemTrayIcon>

#include <window.h>
#include <ekeventfilter.h>

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(ekalappai);

    QApplication app(argc, argv);

    app.setApplicationName("eKalappai");
    app.setApplicationVersion("3.1");   

    //Allow only one instance of eKalappai at any time
    //This solution is based on solution provided in this article http://stackoverflow.com/questions/4087235/how-to-force-my-application-to-open-one-exe-only-qt-linux
    QSharedMemory shared("59698760-43bb-44d9-8121-181ecbb70e4d");
    if( !shared.create( 512, QSharedMemory::ReadWrite) )
    {
      qWarning() << "Cannot start more than one instance of eKalappai any time.";
      exit(0);
    }

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

    EKEventFilter* const ekEventFilter = new EKEventFilter;
    ekEventFilter->window = &window;
    app.installNativeEventFilter(ekEventFilter);
    return app.exec();

}

