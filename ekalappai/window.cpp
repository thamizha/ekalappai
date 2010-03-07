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

    //define meiezhuthukal keystrokes array for tamil99
    //?   ? 	   ?   ?  ?  ? 	?  ?   ?   ?   ?  ?   ?  ?  ?   ?  ?  ?
    //h   b   [   ]  o   p  l  ;  j  k   '  m  n  v   /  y  u  i
    meiezhuthukkal << 0x48 << 0x42 << 0xDB << 0xDD << 0x4F << 0x50 << 0x4C << 0xBA << 0x4A << 0x4B << 0xDE << 0x4D << 0x4E << 0x56 << 0xBF << 0x59 << 0x55 << 0x49;

    // meiezhuthu keystrokes for phonetic, it includes the grantha ezhuthukal also
    //w r t y p s d f g h  j k l z x c v b n m
    //shift+s , shift+l , shift + r , shift+ N
    meiezhuthukkal_phonetic << 0x57 << 0x52 << 0x54 << 0x59 << 0x50 << 0x53 << 0x44 << 0x46 << 0x47 << 0x48 << 0x4A << 0x4B << 0x4C << 0x5A << 0x58 << 0x43 << 0x56 << 0x42 << 0x4E << 0x4D ;

//initialise the current keycode
    current_vkCode = 0x0;
    previous_1_vkCode = 0x0;
    shiftkey_pressed = FALSE;

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
    settingsAction->setEnabled(isMaximized() || !visible);
    QDialog::setVisible(visible);
}

void Window::closeEvent(QCloseEvent *event)
{
    if (trayIcon->isVisible()) {
        hide();
        event->ignore();
    }
}


bool Window::winEvent( MSG* message, long* result )
{
    UINT msg = message->message;
    WPARAM wparam = message->wParam;
    LPARAM lparam = message->lParam;

//    PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT) (lp);

    switch ( msg )
    {
        case WM_USER+755:
            processKeypressEvent();
            break;

        default:
            break;
    }

    return false;
}

// This function is called when the tray icon is clicked
void Window::setIcon(int index)
{
    QIcon icon = iconComboBox->itemIcon(index);
    trayIcon->setIcon(icon);
    setWindowIcon(icon);
    trayIcon->setToolTip(iconComboBox->itemText(index));


    //call remove hook before cecking for the keyboard choosen .
    removeHook();

    //logic to start a keyboard hook or remove keyboard hook based on the keyboard choosen
    callHook(index);
    showTrayMessage(index);
}


// This function is called when the tray icon is clicked
void Window::changeKeyboard(int index)
{
    QIcon icon = iconComboBox->itemIcon(index);
    trayIcon->setIcon(icon);
    setWindowIcon(icon);
    trayIcon->setToolTip(iconComboBox->itemText(index));

    //call remove hook before cecking for the keyboard choosen .
    removeHook();

    //logic to start a keyboard hook or remove keyboard hook based on the keyboard choosen
        callHook(index);
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
    trayIcon->showMessage("eKalappai 3.0",message, icon, 100);
}


void Window::createIconGroupBox()
{
    iconGroupBox = new QGroupBox(tr("Keyboards"));

    iconLabel = new QLabel("Keyboard:");

    iconComboBox = new QComboBox;
    iconComboBox->addItem(QIcon(":/images/ekalappai_icons_en.png"), tr("No Keyboard"));
    iconComboBox->addItem(QIcon(":/images/ekalappai_icons_tn99.png"), tr("Tamil99"));
    iconComboBox->addItem(QIcon(":/images/ekalappai_icons_anjal.png"), tr("Phonetic"));

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

    settingsAction = new QAction(tr("&Settings"), this);
    connect(settingsAction, SIGNAL(triggered()), this, SLOT(showNormal()));

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}

void Window::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(settingsAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
}

void Window::callHook(int kb_index){
    MyPrototype myFunction;
    bool keyboard_enabled = true;

    //keyboard enabled set to false if no keyboard values is selected
    if(kb_index == 0){
        keyboard_enabled = false;
    }
    else {
        keyboard_enabled = true;
        selected_keyboard = kb_index;
    }

    myFunction = (MyPrototype) myLib->resolve( "Init_nokeyboard" );
    current_keyboard = kb_index;

    if ( myFunction ) {
        hkb = myFunction(GetModuleHandle(0),keyboard_enabled, this->winId());
    }

}

void Window::removeHook(){
    CleanupHook cleanupHook;
    cleanupHook = (CleanupHook) myLib->resolve( "Cleanup" );
    cleanupHook(hkb);
}

//This function gets called continuously by the application loop(via QTimer class). This function checks the keys pressed info
//from ekhook dll and writes appropriate characters by calling GenerateKey function in dll.
//This function also enables or disables the keyboard by checking the shortcut keypresses.
void Window::processKeypressEvent(){

    //logic to check the keyboard status if enabled or disabled

    bool keyboard_status;
    GetKeyboardStatus getkbstatus;
    getkbstatus = (GetKeyboardStatus) myLib->resolve( "GetKeyboardStatus" );
    keyboard_status = getkbstatus();

    if(keyboard_status){
        if(current_keyboard == 0){
            changeKeyboard(selected_keyboard);
        }

    }else{
          if(current_keyboard > 0){
                changeKeyboard(0);
           }
    }


    if(current_vkCode == 0x0){
       GetKeyPress getkeypress;
       getkeypress = (GetKeyPress) myLib->resolve( "GetKeyPress" );
       current_vkCode = getkeypress();

       //get shift key info (notes: need to find a way to get all key info together.)
       GetShiftKeyPress getshiftkeypress;
       getshiftkeypress = (GetShiftKeyPress) myLib->resolve( "GetShiftKeyPress" );
       shiftkey_pressed = getshiftkeypress();
    }

    //logic for keyboards start here...
    if(current_vkCode > 0x0){
        if(selected_keyboard == 1){
            implementTamil99();
        }
        else if (selected_keyboard == 2 ){
            implementPhonetic();
        }
    }
    current_vkCode = 0x0;

}

void Window::implementTamil99(){

    generatekey = (GenerateKey) myLib->resolve( "GenerateKey" );
                switch (current_vkCode) {
                //Q row keys
                case 0x51 : //Q/?
                        if((meiezhuthukkal.contains(previous_1_vkCode)) || (IsPrevkeyGrantha()) ){
                                generatekey(3006,FALSE); //?
                        }
                        else if (shiftkey_pressed){
                                 generatekey(3000,FALSE); //?
                        }
                        else{
                                 generatekey(2950,FALSE); //?
                        }

                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x57 : //W/?
                        if((meiezhuthukkal.contains(previous_1_vkCode)) || (IsPrevkeyGrantha()) ){
                                generatekey(3008,FALSE);
                        }
                        else if (shiftkey_pressed){
                                generatekey(2999,FALSE); //?
                        }
                        else{
                                generatekey(2952,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x45 : //E/?
                        if((meiezhuthukkal.contains(previous_1_vkCode) ) || (IsPrevkeyGrantha()) ){
                                generatekey(3010,FALSE);
                        }
                        else if (shiftkey_pressed){
                                generatekey(2972,FALSE); //?
                        }
                        else{
                                generatekey(2954,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x52 : //R/?
                        if((meiezhuthukkal.contains(previous_1_vkCode)) || (IsPrevkeyGrantha()) ){
                                generatekey(3016,FALSE);
                        }
                        else if (shiftkey_pressed){
                                generatekey(3001,FALSE); //?
                        }
                        else{
                                generatekey(2960,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x54 : //T/?
                        if((meiezhuthukkal.contains(previous_1_vkCode)) || (IsPrevkeyGrantha()) ){
                                generatekey(3015,FALSE);
                        }
                        else if (shiftkey_pressed){
                                generatekey(2965,FALSE); //???
                                generatekey(3021,FALSE);
                                generatekey(2999,FALSE);
                        }
                        else{
                                generatekey(2959,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x59 : //Y/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,FALSE);
                                generatekey(2995,FALSE);
                        }
                        else {
                                generatekey(2995,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x55 : //U/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,FALSE);
                                generatekey(2993,FALSE);
                        }
                        else {
                                generatekey(2993,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x49 : //I/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,FALSE);
                                generatekey(2985,FALSE);
                        }
                        else {
                                generatekey(2985,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x4F : //O/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,FALSE);
                                generatekey(2975,FALSE);
                        }
                        else {
                                generatekey(2975,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x50 : //P/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,FALSE);
                                generatekey(2979,FALSE);
                        }
                        else {
                                generatekey(2979,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0xDB: //[/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,FALSE);
                                generatekey(2970,FALSE);
                        }
                        else {
                                generatekey(2970,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0xDD: //]/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,FALSE);
                                generatekey(2974,FALSE);
                        }
                        else {
                                generatekey(2974,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;


                // A row keys
                case 0x41: //A/?
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        generatekey(2949,FALSE);
                        break;

                case 0x53: //S/?
                        if((meiezhuthukkal.contains(previous_1_vkCode)) || (IsPrevkeyGrantha()) ){
                                generatekey(3007,FALSE);
                        }
                        else{
                                generatekey(2951,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x44: //D/?
                        if((meiezhuthukkal.contains(previous_1_vkCode)) || (IsPrevkeyGrantha()) ){
                                generatekey(3009,FALSE);
                        }
                        else{
                                generatekey(2953,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x46: //F/?
                        if((meiezhuthukkal.contains(previous_1_vkCode)) || (IsPrevkeyGrantha()) ){
                                generatekey(3021,FALSE);
                        }
                        else{
                                generatekey(2947,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x47: //G/?
                        if((meiezhuthukkal.contains(previous_1_vkCode)) || (IsPrevkeyGrantha()) ){
                                generatekey(3014,FALSE);
                        }
                        else{
                                generatekey(2958,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x48: //H/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,FALSE);
                                generatekey(2965,FALSE);
                        }
                        else {
                                generatekey(2965,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x4A: //J/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,FALSE);
                                generatekey(2986,FALSE);
                        }
                        else {
                                generatekey(2986,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x4B: //K/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,FALSE);
                                generatekey(2990,FALSE);
                        }
                        else {
                                generatekey(2990,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x4C: //L/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,FALSE);
                                generatekey(2980,FALSE);
                        }
                        else if(previous_1_vkCode == 0xBA) { //if previous letter is ?, it should be made as mei
                                generatekey(3021,FALSE);
                                generatekey(2980,FALSE);
                        }
                        else {
                                generatekey(2980,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0xBA: //;/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,FALSE);
                                generatekey(2984,FALSE);
                        }
                        else {
                                generatekey(2984,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0xDE: //'/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,FALSE);
                                generatekey(2991,FALSE);
                        }
                        else {
                                generatekey(2991,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                // Z row keys
                case 0x5A: //Z / ?
                        if((meiezhuthukkal.contains(previous_1_vkCode)) || (IsPrevkeyGrantha()) ){
                                generatekey(3020,FALSE);
                        }
                        else{
                                generatekey(2964,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x58: //X/?
                        if((meiezhuthukkal.contains(previous_1_vkCode)) || (IsPrevkeyGrantha()) ){
                                generatekey(3019,FALSE);
                        }
                        else{
                                generatekey(2963,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x43: //C/?
                        if((meiezhuthukkal.contains(previous_1_vkCode)) || (IsPrevkeyGrantha()) ){
                                generatekey(3018,FALSE);
                        }
                        else{
                                generatekey(2962,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x56: //V/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,FALSE);
                                generatekey(2997,FALSE);
                        }
                        else {
                                generatekey(2997,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x42: //B/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,FALSE);
                                generatekey(2969,FALSE);
                        }
                        else {
                                generatekey(2969,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x4E: //N/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,FALSE);
                                generatekey(2994,FALSE);
                        }
                        else {
                                generatekey(2994,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x4D: //M/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,FALSE);
                                generatekey(2992,FALSE);
                        }
                        else {
                                generatekey(2992,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0xBF: // / /?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,FALSE);
                                generatekey(2996,FALSE);
                        }
                        else {
                                generatekey(2996,FALSE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0xBC: // ,
                        generatekey(44,FALSE);
                        break;

                case 0xBE: // ,
                        generatekey(46,FALSE);
                        break;

                default:
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;

                } //switch (current_vkCode)

}

void Window::implementPhonetic(){

    generatekey = (GenerateKey) myLib->resolve( "GenerateKey" );

    switch (current_vkCode) {
    //Q row keys
    case 0x51 : //Q/ஃ
            if (shiftkey_pressed){
                    generatekey(3000,FALSE); //ஸ
            }
            else {
                    generatekey(2947,FALSE); //ஃ
            }

            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x57 : //W/ந்

            generatekey(2984,FALSE); //ந
            generatekey(3021,FALSE); //pulli
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x45 : //E/எ
            if (shiftkey_pressed){
                            if(meiezhuthukkal_phonetic.contains(previous_1_vkCode) ){
                                    generatekey(8,FALSE); //backspace to delete the pulli
                                    generatekey(3015, FALSE); //ே
                            }
                            else {
                                    generatekey(2959,FALSE); //ஏ
                            }
            } //shift pressed
            else if(previous_1_vkCode == current_vkCode) {
                            if(meiezhuthukkal_phonetic.contains(previous_2_vkCode) ){
                                    generatekey(8,FALSE); //delete previous எ
                                    generatekey(3015, FALSE); //ே
                            }
                            else {
                                    generatekey(8,FALSE); //delete previous எ
                                    generatekey(2959,FALSE); //ஏ
                            }
                    previous_2_vkCode = previous_1_vkCode;
                    previous_1_vkCode = NULL;
                    break;
            }
            else {
                            if(meiezhuthukkal_phonetic.contains(previous_1_vkCode) ){
                                    generatekey(8,FALSE); //backspace to delete the pulli
                                    generatekey(3014, FALSE); //ெ
                            }
                            else {
                                    generatekey(2958,FALSE); //எ
                            }
            }

            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x52 : //R/ர்
            if (shiftkey_pressed){
                    generatekey(2993,FALSE); //ற
                    generatekey(3021,FALSE); //pulli
            }
            else {
                    generatekey(2992,FALSE); //ர
                    generatekey(3021,FALSE); //pulli
            }
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x54 : //T/ட்
            generatekey(2975,FALSE); //ட
            generatekey(3021,FALSE); //pulli
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x59 : //Y/ய்
            generatekey(2991,FALSE); //ய
            generatekey(3021,FALSE); //pulli
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x55 : //U/உ
            if (shiftkey_pressed){
                            if(meiezhuthukkal_phonetic.contains(previous_1_vkCode) ){
                                    generatekey(8,FALSE); //backspace to delete the pulli
                                    generatekey(3010, FALSE); //ூ
                            }
                            else {
                                    generatekey(2954,FALSE); //ஊ
                            }
            } //shift pressed
            else if(previous_1_vkCode == current_vkCode) {
                            if(meiezhuthukkal_phonetic.contains(previous_2_vkCode) ){
                                    generatekey(8,FALSE); //delete previous உ
                                    generatekey(3010, FALSE); //ூ
                            }
                            else {
                                    generatekey(8,FALSE); //delete previous உ
                                    generatekey(2954,FALSE); //ஊ
                            }
                    previous_2_vkCode = previous_1_vkCode;
                    previous_1_vkCode = NULL;
                    break;
            }
            else {
                            if(meiezhuthukkal_phonetic.contains(previous_1_vkCode) ){
                                    generatekey(8,FALSE); //backspace to delete the pulli
                                    generatekey(3009, FALSE); //ு
                            }
                            else {
                                    generatekey(2953,FALSE); //உ
                            }
            }
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x49 : //I/இ
            if (shiftkey_pressed){
                            if(meiezhuthukkal_phonetic.contains(previous_1_vkCode) ){
                                    generatekey(8,FALSE); //backspace to delete the pulli
                                    generatekey(3008, FALSE); //ீ
                            }
                            else {
                                    generatekey(2952,FALSE); //ஈ
                            }
            } //shift pressed
            else if(previous_1_vkCode == current_vkCode) { //double kuril press
                            if(meiezhuthukkal_phonetic.contains(previous_2_vkCode) ){
                                    generatekey(8,FALSE);
                                    generatekey(3008, FALSE); //ீ
                            }
                            else {
                                    generatekey(8,FALSE); //delete previous இ
                                    generatekey(2952,FALSE); //ஈ
                            }
                    previous_2_vkCode = previous_1_vkCode;
                    previous_1_vkCode = NULL;
                    break;
            }
            else {
                            if(meiezhuthukkal_phonetic.contains(previous_1_vkCode) ){
                                    generatekey(8,FALSE); //backspace to delete the pulli
                                    generatekey(3007, FALSE); //ி
                            }
                            else {
                                    if((previous_1_vkCode == 0x41) && !(meiezhuthukkal_phonetic.contains(previous_2_vkCode)) ){
                                            generatekey(8,FALSE); //delete the previous a character
                                            generatekey(2960, FALSE); //ai
                                    }
                                    else if ((previous_1_vkCode == 0x41) && (meiezhuthukkal_phonetic.contains(previous_2_vkCode)) ){
                                            generatekey(3016, FALSE); //ை
                                    }
                                    else {
                                            generatekey(2951,FALSE); //இ
                                    }
                            }
            }
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x4F : //O/ஒ
            if (shiftkey_pressed){
                            if(meiezhuthukkal_phonetic.contains(previous_1_vkCode) ){
                                    generatekey(8,FALSE); //backspace to delete the pulli
                                    generatekey(3019, FALSE); //ோ
                            }
                            else {
                                    generatekey(2963,FALSE); //ஓ
                            }
            } //shift pressed
            else if(previous_1_vkCode == current_vkCode) { //double kuril press
                            if(meiezhuthukkal_phonetic.contains(previous_2_vkCode) ){
                                    generatekey(8,FALSE);
                                    generatekey(3019, FALSE); //ோ
                            }
                            else {
                                    generatekey(8,FALSE); //delete previous ஒ
                                    generatekey(2963,FALSE); //ஓ
                            }
                    previous_2_vkCode = previous_1_vkCode;
                    previous_1_vkCode = NULL;
                    break;
            }
            else {
                            if(meiezhuthukkal_phonetic.contains(previous_1_vkCode) ){
                                    generatekey(8,FALSE); //backspace to delete the pulli
                                    generatekey(3018, FALSE); //ொ
                            }
                            else {
                                    generatekey(2962,FALSE); //ஒ
                            }
            }

            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x50 : //P/ப்
            generatekey(2986,FALSE); //ப
            generatekey(3021,FALSE); //pulli
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    // A row keys
    case 0x41: //A/அ
            if (shiftkey_pressed){
                            if(meiezhuthukkal_phonetic.contains(previous_1_vkCode) ){
                                    generatekey(8,FALSE); //backspace to delete the pulli
                                    generatekey(3006, FALSE); //ா
                            }
                            else {
                                    generatekey(2950,FALSE); //ஆ
                            }
            } //shift pressed
            else if(previous_1_vkCode == current_vkCode) {
                            if(meiezhuthukkal_phonetic.contains(previous_2_vkCode) ){
                                    generatekey(3006, FALSE); //ா
                            }
                            else {
                                    generatekey(8,FALSE); //delete previous a
                                    generatekey(2950,FALSE); //ஆ
                            }
                    previous_2_vkCode = previous_1_vkCode;
                    previous_1_vkCode = NULL;

                    break;
            }
            else {
                            if(meiezhuthukkal_phonetic.contains(previous_1_vkCode) ){
                                    generatekey(8,FALSE); //backspace to delete the pulli
                            }
                            else {
                                    generatekey(2949,FALSE); //அ
                            }
            }
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;

            break;

    case 0x53: //S/ச்
            if (shiftkey_pressed){
                    generatekey(3000,FALSE); //ஸ
                    generatekey(3021,FALSE); //pulli
            }
            else {
                    generatekey(2970,FALSE); //ச
                    generatekey(3021,FALSE); //pulli
            }
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x44: //D/ட்
            generatekey(2975,FALSE); //ட
            generatekey(3021,FALSE); //pulli
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x46: //F/ஃ
            generatekey(2947,FALSE); //ஃ
            generatekey(2986,FALSE); //ப
            generatekey(3021,FALSE); //pulli
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x47: //G/க்
            if (previous_1_vkCode == 0x4E){
                    generatekey(8,FALSE);
                    generatekey(8,FALSE);
                    generatekey(2969,FALSE); //ங
                    generatekey(3021,FALSE); //pulli
            }
            else {
                    generatekey(2965,FALSE); //க
                    generatekey(3021,FALSE); //pulli
            }
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x48: //H/ஹ்
            if( previous_1_vkCode == 0x54){
                    generatekey(8,FALSE);
                    generatekey(8,FALSE); //2 backspaces to delete ட்
                    generatekey(2980,FALSE); //2980
                    generatekey(3021,FALSE); //pulli
            }
            else {
                    generatekey(3001,FALSE); //ஹ
                    generatekey(3021,FALSE); //pulli
            }
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x4A: //J/ஜ்
            if (previous_1_vkCode == 0x4E){
                    generatekey(8,FALSE);
                    generatekey(8,FALSE);
                    generatekey(2974,FALSE); //ஞ
                    generatekey(3021,FALSE); //pulli
            }
            else {
                    generatekey(2972,FALSE); //ஜ
                    generatekey(3021,FALSE); //pulli
            }

            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x4B: //K/க்
            generatekey(2965,FALSE); //க
            generatekey(3021,FALSE); //pulli
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x4C: //L/ல்
            if (shiftkey_pressed){
                    generatekey(2995,FALSE); //ஸ
                    generatekey(3021,FALSE); //pulli
            }
            else {
                    generatekey(2994,FALSE); //ல்
                    generatekey(3021,FALSE); //pulli
            }
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    // Z row keys
    case 0x5A: //Z / ழ்
            generatekey(2996,FALSE); //ழ
            generatekey(3021,FALSE); //pulli
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x58: //X/க்ஷ்
            generatekey(2965,FALSE); //க
            generatekey(3021,FALSE); //pulli
            generatekey(2999,FALSE); //க்ஷ
            generatekey(3021,FALSE); //pulli
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x43: //C/ச்
            generatekey(2970,FALSE); //ச
            generatekey(3021,FALSE); //pulli
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;


    case 0x56: //V/வ்
            generatekey(2997,FALSE); //வ
            generatekey(3021,FALSE); //pulli
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;


    case 0x42: //B/ப்
            generatekey(2986,FALSE); //ப
            generatekey(3021,FALSE); //pulli
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x4E: //N/ன்
            if (shiftkey_pressed){
                    generatekey(2979,FALSE); //ண
                    generatekey(3021,FALSE); //pulli
            }
            else {
                    generatekey(2985,FALSE); //ன
                    generatekey(3021,FALSE); //pulli
            }
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x4D: //M/ம்
            generatekey(2990,FALSE); //ம
            generatekey(3021,FALSE); //pulli
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0xBC: //comma
            generatekey(44,FALSE); //comma
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0xBE: //full stop
            generatekey(46,FALSE); //full stop
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x08: //backspace
            generatekey(8,FALSE); //backspace
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    default:
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    } //switch (current_vkCode)

}

//Helper functions//
bool Window::SearchArray (DWORD array[], DWORD key, int length)
{
        int index = 0;
        while( (index < length) && (key != array[index]))
        {
                if (array[index] != key)
                        index++;
        }

        if (index < length){
                return true;
        }
        else{
                return false;
        }
}


bool Window::IsPrevkeyGrantha()
{
        //define grantha array
        //?  ? ? ? ???
        int granthaezhuthukkal[] = {3000, 2999, 2972, 3001};

        int index = 0;
        while( (index < 4 ) && (previous_1_character != granthaezhuthukkal[index]))
        {
                if (granthaezhuthukkal[index] != previous_1_character)
                        index++;
        }

        if (index < 4){
                return true;
        }
        else{
                return false;
        }
}
