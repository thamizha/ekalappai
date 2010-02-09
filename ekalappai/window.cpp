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
#include "window.h"

//#define _WIN32_WINNT 0x0500
#define WINVER 0x0400

#include <winuser.h>

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

Window::Window()
{
    //load ekalappai hook dll
    myLib = new QLibrary( "ekhook.dll" );

    createIconGroupBox();

    createActions();
    createTrayIcon();

    connect(iconComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setIcon(int)));

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(iconGroupBox);
    setLayout(mainLayout);

    iconComboBox->setCurrentIndex(1);
    trayIcon->show();

    setWindowTitle(tr("eKalappai 3.0"));
}

void Window::setVisible(bool visible)
{
    minimizeAction->setEnabled(visible);
    maximizeAction->setEnabled(!isMaximized());
    restoreAction->setEnabled(isMaximized() || !visible);
    QDialog::setVisible(visible);
}

void Window::closeEvent(QCloseEvent *event)
{
    if (trayIcon->isVisible()) {
        hide();
        event->ignore();
    }
}


// This function is called when the tray icon is clicked (which causes increment in the index value)
void Window::setIcon(int index)
{
    QIcon icon = iconComboBox->itemIcon(index);
    trayIcon->setIcon(icon);
    setWindowIcon(icon);
    trayIcon->setToolTip(iconComboBox->itemText(index));

    //logic to start keyboard hook or remove keyboard hook based on the keyboard choosen
    if(index == 0){
        //when no keyboard is selected stop the keyboard hook
        removeHook();
    }
    else{
        //start keyboard hook
        callHook();
    }
    showTrayMessage(index);
}

void Window::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        iconComboBox->setCurrentIndex((iconComboBox->currentIndex() + 1)
                                      % iconComboBox->count());
        break;
    case QSystemTrayIcon::MiddleClick:
        showTrayMessage(iconComboBox->currentIndex());
        break;
    default:
        ;
    }
}


void Window::showTrayMessage(int index)
{
    QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(0);

    QString message;

    message = iconComboBox->itemText(index)+ " set";
    trayIcon->showMessage("eKalappai 3.0",message, icon, 20 * 500);
}


void Window::createIconGroupBox()
{
    iconGroupBox = new QGroupBox(tr("Keyboards"));

    iconLabel = new QLabel("Keyboard:");

    iconComboBox = new QComboBox;
    iconComboBox->addItem(QIcon(":/images/nokey.png"), tr("No Keyboard"));
    iconComboBox->addItem(QIcon(":/images/logo.png"), tr("Tamil99"));

    QHBoxLayout *iconLayout = new QHBoxLayout;
    iconLayout->addWidget(iconLabel);
    iconLayout->addWidget(iconComboBox);
    iconLayout->addStretch();
    iconGroupBox->setLayout(iconLayout);
}

void Window::createActions()
{
    minimizeAction = new QAction(tr("Mi&nimize"), this);
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

    maximizeAction = new QAction(tr("Ma&ximize"), this);
    connect(maximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));

    restoreAction = new QAction(tr("&Restore"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}

void Window::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(maximizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
}

void Window::callHook(){
    MyPrototype myFunction;
    myFunction = (MyPrototype) myLib->resolve( "Init" );

    if ( myFunction ) {
        hkb = myFunction(GetModuleHandle(0));
    }
}

void Window::removeHook(){
    CleanupHook cleanupHook;
    cleanupHook = (CleanupHook) myLib->resolve( "Cleanup" );
    cleanupHook(hkb);
}
