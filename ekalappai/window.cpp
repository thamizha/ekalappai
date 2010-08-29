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

    //store the valid keys to use in the processkey function
    //numbers
    valid_keys << 0x30 << 0x31 << 0x32 << 0x33 << 0x34 << 0x35 << 0x36 << 0x37 << 0x38 << 0x39;
    //alphabets
    valid_keys << 0x41 << 0x42 << 0x43 << 0x44 << 0x45 << 0x46 << 0x47 << 0x48 << 0x49 << 0x4A << 0x4B << 0x4C << 0x4D << 0x4E << 0x4F << 0x50 << 0x51 << 0x52 << 0x53 << 0x54 << 0x55 << 0x56 << 0x57 << 0x58 << 0x59 << 0x5A;
    // funcion keys F1 to F12
    valid_keys << 0x70 << 0x71 << 0x72 << 0x73 << 0x74 << 0x75 << 0x76 << 0x77 << 0x78 << 0x79 << 0x7A << 0x7B;
    //special characters ESC ~ - + [ ] ; ' \ , . /
    valid_keys << 0x1B << 0xC0 << 0xBB << 0xBD << 0xDB << 0xDD << 0xBA << 0xDE << 0xDC << 0xBC << 0xBF << 0xBE;

    //spacebar, enterkey, backspace key
    valid_keys << 0x20 << 0x0D << 0x08;

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
    controlkey_pressed = FALSE;
    altkey_pressed = FALSE;
    keyboard_status = TRUE;
    character_pressed = 0;
    prev1_character_pressed = 0;
    prev2_character_pressed = 0;
    prev3_character_pressed = 0;
    prev4_character_pressed = 0;
    prev5_character_pressed = 0;

    //This value will replace all the prev character_pressed variables. This will store upto 20 keys presssed, the last one being the latest key pressed.
    charpressed_string20 = "";
    prev_unicode_character_length = 0;
    current_unicode_character_length = 0;

    settings = new QSettings( "settings.ini", QSettings::IniFormat );

    keyrules = new QSettings( "phonetic.ini", QSettings::IniFormat );

    shortcut_modifier_key = settings->value("shortcut_modifier").toString();
    short_cut_key = settings->value("shortcut").toString();
    short_cut_key_hex = 0x0;

    createIconGroupBox();
    createShortcutGroupBox();

    // call set shortcut2 function so that the short_cut_key_hex value is populated as per the ini settings
    setShortcut2(0);

    createActions();
    createTrayIcon();

    connect(iconComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setIcon(int)));

    connect(shortcutComboBox1, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setShortcut1(int)));

    connect(shortcutComboBox2, SIGNAL(currentIndexChanged(int)),
            this, SLOT(setShortcut2(int)));

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(iconGroupBox);
    mainLayout->addWidget(shortcutGroupBox);
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
    loadKeyBoard();
}

// This function is called when the shortcut modifier combo is changed
void Window::setShortcut1(int index)
{
    settings->setValue("shortcut_modifier", shortcutComboBox1->currentText());

    //if none is selected, the allowed single key shortcuts should change
    if(index == 0){
        shortcutComboBox2->clear();
        shortcutComboBox2->addItem(tr("ESC"));
        shortcutComboBox2->addItem(tr("F1"));
        shortcutComboBox2->addItem(tr("F2"));
        shortcutComboBox2->addItem(tr("F3"));
        shortcutComboBox2->addItem(tr("F4"));
        shortcutComboBox2->addItem(tr("F5"));
        shortcutComboBox2->addItem(tr("F6"));
        shortcutComboBox2->addItem(tr("F7"));
        shortcutComboBox2->addItem(tr("F8"));
        shortcutComboBox2->addItem(tr("F9"));
        shortcutComboBox2->addItem(tr("F10"));
    }
    else {
        shortcutComboBox2->clear();
        shortcutComboBox2->addItem(tr("1"));
        shortcutComboBox2->addItem(tr("2"));
        shortcutComboBox2->addItem(tr("3"));
        shortcutComboBox2->addItem(tr("4"));
        shortcutComboBox2->addItem(tr("5"));
        shortcutComboBox2->addItem(tr("6"));
        shortcutComboBox2->addItem(tr("7"));
        shortcutComboBox2->addItem(tr("8"));
        shortcutComboBox2->addItem(tr("9"));
        shortcutComboBox2->addItem(tr("0"));
    }
}

// This function is called when the shortcut combo is changed
void Window::setShortcut2(int index)
{

    short_cut_key =   shortcutComboBox2->currentText();
    settings->setValue("shortcut", short_cut_key);

    if (short_cut_key == "ESC"){
        short_cut_key_hex = 0x1B;
        }
    else if (short_cut_key == "F1"){
        short_cut_key_hex = 0x70;
    }
    else if (short_cut_key == "F2"){
        short_cut_key_hex = 0x71;
    }
    else if (short_cut_key == "F3"){
        short_cut_key_hex = 0x72;
    }
    else if (short_cut_key == "F4"){
        short_cut_key_hex = 0x73;
    }
    else if (short_cut_key == "F5"){
        short_cut_key_hex = 0x74;
    }
    else if (short_cut_key == "F6"){
        short_cut_key_hex = 0x75;
    }
    else if (short_cut_key == "F7"){
        short_cut_key_hex = 0x76;
    }
    else if (short_cut_key == "F8"){
        short_cut_key_hex = 0x77;
    }
    else if (short_cut_key == "F9"){
        short_cut_key_hex = 0x78;
    }
    else if (short_cut_key == "F10"){
        short_cut_key_hex = 0x79;
    }
    else if (short_cut_key == "1"){
        short_cut_key_hex = 0x31;
    }
    else if (short_cut_key == "2"){
        short_cut_key_hex = 0x32;
    }
    else if (short_cut_key == "3"){
        short_cut_key_hex = 0x33;
    }
    else if (short_cut_key == "4"){
        short_cut_key_hex = 0x34;
    }
    else if (short_cut_key == "5"){
        short_cut_key_hex = 0x35;
    }
    else if (short_cut_key == "6"){
        short_cut_key_hex = 0x36;
    }
    else if (short_cut_key == "7"){
        short_cut_key_hex = 0x37;
    }
    else if (short_cut_key == "8"){
        short_cut_key_hex = 0x38;
    }
    else if (short_cut_key == "9"){
        short_cut_key_hex = 0x39;
    }
    else if (short_cut_key == "0"){
        short_cut_key_hex = 0x30;
    }
}


void Window::loadKeyBoard(){
    QString str1;
    QString str2;
    QString temp1;

    int unicode_value1 = 0;
   //add the last keystroe to the keystrokes array - we will maintain abou 6 keystrokes for now
    //keystrokes.append(current_vkCode);

    //keyrules->setValue(charpressed_string, "3002" );
    //unicode_value1 = keyrules->value(charpressed_string).toInt();

    //file handling code
    QFile file("phonetic.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    in.setAutoDetectUnicode(true);

    bool line_start = FALSE;
    bool line_end = FALSE;

    //Read the file line by line & insert into keyboardmap arrays
    while (!in.atEnd()) {
        QString line = in.readLine();
        str1.clear();
        str2.clear();

        //process_line(line);
        if (line == "BEGIN_TABLE"){
            line_start = true;
            line_end = false;
        }

        if (line == "END_TABLE"){
            line_end = true;
        }

        if ((line_start) && !(line_end)){
            QRegExp sep("\\s+");
            str1 = line.section(sep, 0, 0); // first part
            str2 = line.section(sep, 1, 1) ; // second part
            keyboardmap.insert(str1, str2);
           // keyrules->setValue(str1, str2);
        }

    }


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
        loadKeyBoard();

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

void Window::createShortcutGroupBox()
{
    shortcutGroupBox = new QGroupBox(tr("Shortcut Setting"));

    shortcutLabel1 = new QLabel("Modifier Key:");
    shortcutLabel2 = new QLabel("Shortcut Key:");

    shortcutComboBox1 = new QComboBox;
    shortcutComboBox1->addItem(tr("NONE"));
    shortcutComboBox1->addItem(tr("CTRL"));
   //shortcutComboBox1->addItem(tr("ALT"));

    int index_tmp1 = shortcutComboBox1->findText(shortcut_modifier_key);
    shortcutComboBox1->setCurrentIndex(index_tmp1);

    shortcutComboBox2 = new QComboBox;
    shortcutComboBox2->setMinimumContentsLength(3);

    if(index_tmp1 == 0){
        shortcutComboBox2->addItem(tr("ESC"));
        shortcutComboBox2->addItem(tr("F1"));
        shortcutComboBox2->addItem(tr("F2"));
        shortcutComboBox2->addItem(tr("F3"));
        shortcutComboBox2->addItem(tr("F4"));
        shortcutComboBox2->addItem(tr("F5"));
        shortcutComboBox2->addItem(tr("F6"));
        shortcutComboBox2->addItem(tr("F7"));
        shortcutComboBox2->addItem(tr("F8"));
        shortcutComboBox2->addItem(tr("F9"));
        shortcutComboBox2->addItem(tr("F10"));
    }
    else {
        shortcutComboBox2->addItem(tr("1"));
        shortcutComboBox2->addItem(tr("2"));
        shortcutComboBox2->addItem(tr("3"));
        shortcutComboBox2->addItem(tr("4"));
        shortcutComboBox2->addItem(tr("5"));
        shortcutComboBox2->addItem(tr("6"));
        shortcutComboBox2->addItem(tr("7"));
        shortcutComboBox2->addItem(tr("8"));
        shortcutComboBox2->addItem(tr("9"));
        shortcutComboBox2->addItem(tr("0"));
    }
    int index_tmp2 =  shortcutComboBox2->findText(short_cut_key);
    shortcutComboBox2->setCurrentIndex(index_tmp2);

    QHBoxLayout *shortcutLayout = new QHBoxLayout;
    shortcutLayout->addWidget(shortcutLabel1);
    shortcutLayout->addWidget(shortcutComboBox1);
    shortcutLayout->addWidget(shortcutLabel2);
    shortcutLayout->addWidget(shortcutComboBox2);
    shortcutLayout->addStretch();
    shortcutGroupBox->setLayout(shortcutLayout);
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

       GetKeyPress getkeypress;
       getkeypress = (GetKeyPress) myLib->resolve( "GetKeyPress" );
       current_vkCode = getkeypress();

       GetCharPressed getcharpressed;
       getcharpressed = (GetCharPressed) myLib->resolve( "GetCharPressed" );
       character_pressed = getcharpressed();

  //////// Notes: shiftkey press function should be removed from dll and it should be calculatd inside the exe itself /////////////
       //get shift key info (notes: need to find a way to get all key info together.)
       GetShiftKeyPress getshiftkeypress;
       getshiftkeypress = (GetShiftKeyPress) myLib->resolve( "GetShiftKeyPress" );
       shiftkey_pressed = getshiftkeypress();

       GetControlKeyPress getcontrolkeypress;
       getcontrolkeypress = (GetControlKeyPress) myLib->resolve( "GetControlKeyPress" );
       controlkey_pressed = getcontrolkeypress();

       GetAltKeyPress getaltkeypress;
       getaltkeypress = (GetAltKeyPress) myLib->resolve( "GetAltKeyPress" );
       altkey_pressed = getaltkeypress();


       //toggle the keyboard_enabled flag based on the shortcut key placed
       if((current_vkCode == short_cut_key_hex) && (shortcut_modifier_key == "NONE")) {
            if (keyboard_status)
                 keyboard_status = false;
             else
                 keyboard_status = true;
         }
       //if control key is modifier
       else if ((current_vkCode == short_cut_key_hex) && (shortcut_modifier_key == "CTRL")&& (controlkey_pressed == true ) ){
           if (keyboard_status)
                keyboard_status = false;
            else
                keyboard_status = true;
       }
       // if alt key is modifier
       else if ((current_vkCode == short_cut_key_hex) && (shortcut_modifier_key == "ALT")&& (altkey_pressed == true) ){
           if (keyboard_status)
                keyboard_status = false;
            else
                keyboard_status = true;
       }

       if(keyboard_status){
           if(current_keyboard == 0){
               changeKeyboard(selected_keyboard);
           }

       }else{
             if(current_keyboard > 0){
                   changeKeyboard(0);
              }
       }

       if((keyboard_status)&& !(controlkey_pressed)){      //if keyboard enabled then implement the keyboards
            if(selected_keyboard == 1){
                implementTamil99();
            }
            else if (selected_keyboard == 2 ){
                implementPhonetic_new();
            }
        }
}

void Window::generatekey(int key,bool state){
    generatekeyLib = (GenerateKey) myLib->resolve( "GenerateKey" );
    generatekeyLib(key,state);
    if (state){
        previous_4_character = previous_3_character;
        previous_3_character = previous_2_character;
        previous_2_character = previous_1_character;
        previous_1_character = key;
    }
}

void Window::implementTamil99(){

                switch (current_vkCode) {
                //Q row keys
                case 0x51 : //Q/?
                        if((meiezhuthukkal.contains(previous_1_vkCode)) || (IsPrevkeyGrantha()) ){
                                generatekey(3006,TRUE); //?
                        }
                        else if (shiftkey_pressed){
                                 generatekey(3000,TRUE); //?
                        }
                        else{
                                 generatekey(2950,TRUE); //?
                        }

                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x57 : //W/?
                        if((meiezhuthukkal.contains(previous_1_vkCode)) || (IsPrevkeyGrantha()) ){
                                generatekey(3008,TRUE);
                        }
                        else if (shiftkey_pressed){
                                generatekey(2999,TRUE); //?
                        }
                        else{
                                generatekey(2952,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x45 : //E/?
                        if((meiezhuthukkal.contains(previous_1_vkCode) ) || (IsPrevkeyGrantha()) ){
                                generatekey(3010,TRUE);
                        }
                        else if (shiftkey_pressed){
                                generatekey(2972,TRUE); //?
                        }
                        else{
                                generatekey(2954,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x52 : //R/?
                        if((meiezhuthukkal.contains(previous_1_vkCode)) || (IsPrevkeyGrantha()) ){
                                generatekey(3016,TRUE);
                        }
                        else if (shiftkey_pressed){
                                generatekey(3001,TRUE); //?
                        }
                        else{
                                generatekey(2960,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x54 : //T/?
                        if((meiezhuthukkal.contains(previous_1_vkCode)) || (IsPrevkeyGrantha()) ){
                                generatekey(3015,TRUE);
                        }
                        else if (shiftkey_pressed){
                                generatekey(2965,TRUE); //???
                                generatekey(3021,TRUE);
                                generatekey(2999,TRUE);
                        }
                        else{
                                generatekey(2959,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x59 : //Y/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,TRUE);
                                generatekey(2995,TRUE);
                        }
                        else {
                                generatekey(2995,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x55 : //U/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,TRUE);
                                generatekey(2993,TRUE);
                        }
                        else {
                                generatekey(2993,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x49 : //I/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,TRUE);
                                generatekey(2985,TRUE);
                        }
                        else {
                                generatekey(2985,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x4F : //O/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,TRUE);
                                generatekey(2975,TRUE);
                        }
                        else {
                                generatekey(2975,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x50 : //P/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,TRUE);
                                generatekey(2979,TRUE);
                        }
                        else {
                                generatekey(2979,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0xDB: //[/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,TRUE);
                                generatekey(2970,TRUE);
                        }
                        else {
                                generatekey(2970,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0xDD: //]/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,TRUE);
                                generatekey(2974,TRUE);
                        }
                        else {
                                generatekey(2974,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;


                // A row keys
                case 0x41: //A/?
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        generatekey(2949,TRUE);
                        break;

                case 0x53: //S/?
                        if((meiezhuthukkal.contains(previous_1_vkCode)) || (IsPrevkeyGrantha()) ){
                                generatekey(3007,TRUE);
                        }
                        else{
                                generatekey(2951,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x44: //D/?
                        if((meiezhuthukkal.contains(previous_1_vkCode)) || (IsPrevkeyGrantha()) ){
                                generatekey(3009,TRUE);
                        }
                        else{
                                generatekey(2953,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x46: //F/?
                        if((meiezhuthukkal.contains(previous_1_vkCode)) || (IsPrevkeyGrantha()) ){
                                generatekey(3021,TRUE);
                        }
                        else{
                                generatekey(2947,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x47: //G/?
                        if((meiezhuthukkal.contains(previous_1_vkCode)) || (IsPrevkeyGrantha()) ){
                                generatekey(3014,TRUE);
                        }
                        else{
                                generatekey(2958,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x48: //H/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,TRUE);
                                generatekey(2965,TRUE);
                        }
                        else {
                                generatekey(2965,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x4A: //J/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,TRUE);
                                generatekey(2986,TRUE);
                        }
                        else {
                                generatekey(2986,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x4B: //K/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,TRUE);
                                generatekey(2990,TRUE);
                        }
                        else {
                                generatekey(2990,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x4C: //L/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,TRUE);
                                generatekey(2980,TRUE);
                        }
                        else if(previous_1_vkCode == 0xBA) { //if previous letter is ?, it should be made as mei
                                generatekey(3021,TRUE);
                                generatekey(2980,TRUE);
                        }
                        else {
                                generatekey(2980,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0xBA: //;/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,TRUE);
                                generatekey(2984,TRUE);
                        }
                        else {
                                generatekey(2984,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0xDE: //'/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,TRUE);
                                generatekey(2991,TRUE);
                        }
                        else {
                                generatekey(2991,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                // Z row keys
                case 0x5A: //Z / ?
                        if((meiezhuthukkal.contains(previous_1_vkCode)) || (IsPrevkeyGrantha()) ){
                                generatekey(3020,TRUE);
                        }
                        else{
                                generatekey(2964,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x58: //X/?
                        if((meiezhuthukkal.contains(previous_1_vkCode)) || (IsPrevkeyGrantha()) ){
                                generatekey(3019,TRUE);
                        }
                        else{
                                generatekey(2963,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x43: //C/?
                        if((meiezhuthukkal.contains(previous_1_vkCode)) || (IsPrevkeyGrantha()) ){
                                generatekey(3018,TRUE);
                        }
                        else{
                                generatekey(2962,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x56: //V/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,TRUE);
                                generatekey(2997,TRUE);
                        }
                        else {
                                generatekey(2997,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x42: //B/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,TRUE);
                                generatekey(2969,TRUE);
                        }
                        else {
                                generatekey(2969,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x4E: //N/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,TRUE);
                                generatekey(2994,TRUE);
                        }
                        else {
                                generatekey(2994,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x4D: //M/?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,TRUE);
                                generatekey(2992,TRUE);
                        }
                        else {
                                generatekey(2992,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0xBF: // / /?
                        //??? ??? ???????????? ?????? ???? ???????, ????? ??????? ???????????
                        if((previous_1_vkCode == current_vkCode) && (previous_2_vkCode != previous_1_vkCode) ){
                                generatekey(3021,TRUE);
                                generatekey(2996,TRUE);
                        }
                        else {
                                generatekey(2996,TRUE);
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0xBC: //comma
                        generatekey(44,true); //comma
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0xBE: //full stop
                        generatekey(46,true); //full stop
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0xBD: //- key
                        if (shiftkey_pressed){
                            generatekey(95,true); // underscore _
                        }
                        else {
                            generatekey(45,true); // - or minus
                        }
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                case 0x08: //backspace
                case 0x20: //spacbar
                case 0x0D: //enter key
                        previous_2_vkCode = previous_1_vkCode;
                        previous_1_vkCode = current_vkCode;
                        break;

                default:
                        break;

                } //switch (current_vkCode)

}


void Window::generateUnicodeCharacters(QString characters){
    // store the length of the characters globally
    current_unicode_character_length = characters.length();

    //Delete the previous unicode characters if both previous and current characters are present
    if ((prev_unicode_character_length > 0) && (current_unicode_character_length > 0)){
        int i = 0;
        while (i < prev_unicode_character_length){
            generatekey(8,FALSE);
            i++;
        }
    }

    //generate the unicode characters if the matching character is found in the keyboard file
    if (current_unicode_character_length > 0){
        int j = 0;
        while (j < current_unicode_character_length){
            generatekey(characters.at(j).unicode(),TRUE);
            j++;
        }
    }

    //assign the current character length to the previous length and exit the function
    prev_unicode_character_length = current_unicode_character_length;
}

/*
logic to use:

1. store the typed values in a string continuosly updating it as we type
2. keep the length around 20 chars... - assumption we dont need to store more than 20 chars
3. just check the last 5 chars - this is just hardcoded for simplicity need to make it dynamic by reading the value from scim table format  & match it with the scim table
4. if match found
        { (step 1 starts)

        check the previous number of tamil chars & delete those many chars
        print the tamil characters
        store the number of unicode charaters printed
        (step1 end)
        }

        else
        check the last 4 chars

        if match found
                repeat the steps 1
5. continue till single char is found
6. When single char is matched, dont delete any previous char
*/
void Window::implementPhonetic_new(){

    if (valid_keys.contains(current_vkCode)){

        //character_pressed contains the english alphabet pressed, its obtained from the hookdll
        charpressed_string20 += ((QChar)character_pressed);

        //remove extra previous characters if the string contains more than 20 characters
        if (charpressed_string20.length() > 20){
                charpressed_string20 = charpressed_string20.right(20);
            }


//        QMessageBox msgBox1;
//        msgBox1.setText(charpressed_string20.right(5));
//        msgBox1.exec();

        QString str1;
        QString str2;
        QString temp1;

        int unicode_value1 = 0;
        int unicode_value2 = 0;
        int unicode_value3 = 0;
        int unicode_value4 = 0;
        int unicode_value5 = 0;

        if(keyboardmap.contains(charpressed_string20.right(5))){
            generateUnicodeCharacters(keyboardmap.value(charpressed_string20.right(5)));
        }
        else if (keyboardmap.contains(charpressed_string20.right(4))){
            generateUnicodeCharacters(keyboardmap.value(charpressed_string20.right(4)));
        }
        else if (keyboardmap.contains(charpressed_string20.right(3))){
            generateUnicodeCharacters(keyboardmap.value(charpressed_string20.right(3)));
        }
        else if (keyboardmap.contains(charpressed_string20.right(2))){
            generateUnicodeCharacters(keyboardmap.value(charpressed_string20.right(2)));
        }
        else if (keyboardmap.contains(charpressed_string20.right(1))){

            //if it single character, there is no need to delete the previous character, so prev character length is made 0
            prev_unicode_character_length = 0; //--- this logic doesnt work right.. need to find alternate way
            generateUnicodeCharacters(keyboardmap.value(charpressed_string20.right(1)));
        }

         return;
}
}


void Window::implementPhonetic(){
   // generatekey = (GenerateKey) myLib->resolve( "GenerateKey" );

    switch (current_vkCode) {
    //Q row keys
    case 0x51 : //Q/ஃ
            if (shiftkey_pressed){
                    generatekey(3000,TRUE); //ஸ
            }
            else {
                    generatekey(2947,TRUE); //ஃ
            }

            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x57 : //W/ந்

            generatekey(2984,TRUE); //ந
            generatekey(3021,TRUE); //pulli
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x45 : //E/எ
            if (shiftkey_pressed){
                            if(IsPrevkeyMey(1) ){
                                    generatekey(8,FALSE); //backspace to delete the pulli
                                    generatekey(3015, TRUE); //ே
                            }
                            else {
                                    generatekey(2959,TRUE); //ஏ
                            }
            } //shift pressed
            else if(previous_1_vkCode == current_vkCode) {
                            if(IsPrevkeyMey(2)  ){
                                    generatekey(8,FALSE); //delete previous எ
                                    generatekey(3015, TRUE); //ே
                            }
                            else {
                                    generatekey(8,FALSE); //delete previous எ
                                    generatekey(2959,TRUE); //ஏ
                            }
                    previous_2_vkCode = previous_1_vkCode;
                    previous_1_vkCode = NULL;
                    break;
            }
            else {
                            if(IsPrevkeyMey(1) ){
                                    generatekey(8,FALSE); //backspace to delete the pulli
                                    generatekey(3014, TRUE); //ெ
                            }
                            else {
                                    generatekey(2958,TRUE); //எ
                            }
            }

            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x52 : //R/ர்
            if (shiftkey_pressed){
                    generatekey(2993,TRUE); //ற
                    generatekey(3021,TRUE); //pulli
            }
            else {
                    generatekey(2992,TRUE); //ர
                    generatekey(3021,TRUE); //pulli
            }
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x54 : //T/ட்
            if(previous_1_vkCode == 0x4E){
                generatekey(8,FALSE);
                generatekey(8,FALSE);
                generatekey(2984,TRUE); //ந
                generatekey(3021,TRUE); //pulli
                generatekey(2980,TRUE); //த
                generatekey(3021,TRUE); //pulli
            } else {
                generatekey(2975,TRUE); //ட
                generatekey(3021,TRUE); //pulli
            }
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x59 : //Y/ய்
            generatekey(2991,TRUE); //ய
            generatekey(3021,TRUE); //pulli
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x55 : //U/உ
            if (shiftkey_pressed){
                            if(IsPrevkeyMey(1)  ){
                                    generatekey(8,FALSE); //backspace to delete the pulli
                                    generatekey(3010, TRUE); //ூ
                            }
                            else {
                                    generatekey(2954,TRUE); //ஊ
                            }
            } //shift pressed
            else if(previous_1_vkCode == current_vkCode) {
                            if(IsPrevkeyMey(2) ){
                                    generatekey(8,FALSE); //delete previous உ
                                    generatekey(3010, TRUE); //ூ
                            }
                            else {
                                    generatekey(8,FALSE); //delete previous உ
                                    generatekey(2954,TRUE); //ஊ
                            }
                    previous_2_vkCode = previous_1_vkCode;
                    previous_1_vkCode = NULL;
                    break;
            }
            else {
                            if(IsPrevkeyMey(1)  ){
                                    generatekey(8,FALSE); //backspace to delete the pulli
                                    generatekey(3009, TRUE); //ு
                            }
                            else {

                                if((previous_1_vkCode == 0x41) && !(IsPrevkeyMey(2) ) ){
                                        generatekey(8,FALSE); //delete the previous a character
                                        generatekey(2964, TRUE); //ஔ
                                }
                                else if ((previous_1_vkCode == 0x41) && (IsPrevkeyMey(2) ) ){
                                        generatekey(3020, TRUE); //ௌ
                                }
                                else{
                                    generatekey(2953,TRUE); //உ
                                }
                            }
            }
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x49 : //I/இ
            if (shiftkey_pressed){
                            if(IsPrevkeyMey(1)  ){
                                    generatekey(8,FALSE); //backspace to delete the pulli
                                    generatekey(3008, TRUE); //ீ
                            }
                            else {
                                    generatekey(2952,TRUE); //ஈ
                            }
            } //shift pressed
            else if(previous_1_vkCode == current_vkCode) { //double kuril press
                            if(IsPrevkeyMey(2)  ){
                                    generatekey(8,FALSE);
                                    generatekey(3008, TRUE); //ீ
                            }
                            else {
                                    generatekey(8,FALSE); //delete previous இ
                                    generatekey(2952,TRUE); //ஈ
                            }
                    previous_2_vkCode = previous_1_vkCode;
                    previous_1_vkCode = NULL;
                    break;
            }
            else if ( (previous_1_vkCode == 0x52 ) && (previous_2_vkCode == 0x53 ) && (previous_4_character == 2970 )) { //ஸ்ரீ handling for sri
                generatekey(8,FALSE);
                generatekey(8,FALSE);
                generatekey(8,FALSE);
                generatekey(8,FALSE);
                generatekey(3000,TRUE);
                generatekey(3021,TRUE);
                generatekey(2992,TRUE);
                generatekey(3008,TRUE);
                previous_2_vkCode = previous_1_vkCode;
                previous_1_vkCode = NULL;
                break;
            }
            else {
                            if(IsPrevkeyMey(1) ){
                                    generatekey(8,FALSE); //backspace to delete the pulli
                                    generatekey(3007, TRUE); //ி
                            }
                            else {
                                    if((previous_1_vkCode == 0x41) && !(IsPrevkeyMey(2) ) ){
                                            generatekey(8,FALSE); //delete the previous a character
                                            generatekey(2960, TRUE); //ai
                                    }
                                    else if ((previous_1_vkCode == 0x41) && (IsPrevkeyMey(2) ) ){
                                            generatekey(3016, TRUE); //ை
                                    }
                                    else {
                                            generatekey(2951,TRUE); //இ
                                    }
                            }
            }
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x4F : //O/ஒ
            if (shiftkey_pressed){
                            if(IsPrevkeyMey(1) ){
                                    generatekey(8,FALSE); //backspace to delete the pulli
                                    generatekey(3019, TRUE); //ோ
                            }
                            else {
                                    generatekey(2963,TRUE); //ஓ
                            }
            } //shift pressed
            else if(previous_1_vkCode == current_vkCode) { //double kuril press
                            if(IsPrevkeyMey(2) ){
                                    generatekey(8,FALSE);
                                    generatekey(3019, TRUE); //ோ
                            }
                            else {
                                    generatekey(8,FALSE); //delete previous ஒ
                                    generatekey(2963,TRUE); //ஓ
                            }
            }
            else {
                            if(IsPrevkeyMey(1) ){
                                    generatekey(8,FALSE); //backspace to delete the pulli
                                    generatekey(3018, TRUE); //ொ
                            }
                            else {
                                    generatekey(2962,TRUE); //ஒ
                            }
            }

            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x50 : //P/ப்
            generatekey(2986,TRUE); //ப
            generatekey(3021,TRUE); //pulli
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    // A row keys
    case 0x41: //A/அ
            if (shiftkey_pressed){
                            if(IsPrevkeyMey(1) ){
                                    generatekey(8,FALSE); //backspace to delete the pulli
                                    generatekey(3006, TRUE); //ா
                            }
                            else {
                                    generatekey(2950,TRUE); //ஆ
                            }
            } //shift pressed

            else if(previous_1_vkCode == current_vkCode) {
                            if(IsPrevkeyMey(2) ){
                                    generatekey(3006, TRUE); //ா
                            }

                            else {
                                    generatekey(8,FALSE); //delete previous a
                                    generatekey(2950,TRUE); //ஆ
                            }

                            previous_2_vkCode = previous_1_vkCode;
                            previous_1_vkCode = NULL;
                            break;
            }

            else {
                            if(IsPrevkeyMey(1) ){
                                    generatekey(8,FALSE); //backspace to delete the pulli
                                    generatekey(8,FALSE); //backspace to delete the character
                                    generatekey(previous_2_character, TRUE); // print the character again - this is one due to strang behaviour when only backspace is sent
                            }
                            else {
                                    generatekey(2949,TRUE); //அ
                            }
            }
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = 0x41;
            break;



    case 0x53: //S/ச்
            if (shiftkey_pressed){
                    generatekey(3000,TRUE); //ஸ
                    generatekey(3021,TRUE); //pulli
            }
            else {
                    generatekey(2970,TRUE); //ச
                    generatekey(3021,TRUE); //pulli
            }
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x44: //D/ட்
            generatekey(2975,TRUE); //ட
            generatekey(3021,TRUE); //pulli
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x46: //F/ஃ
            generatekey(2947,TRUE); //ஃ
            generatekey(2986,TRUE); //ப
            generatekey(3021,TRUE); //pulli
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x47: //G/க்
            if (previous_1_vkCode == 0x4E){
                    generatekey(8,FALSE);
                    generatekey(8,FALSE);
                    generatekey(2969,TRUE); //ங
                    generatekey(3021,TRUE); //pulli
            }
            else {
                    generatekey(2965,TRUE); //க
                    generatekey(3021,TRUE); //pulli
            }
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x48: //H/ஹ்
            if( previous_1_vkCode == 0x54){
                    generatekey(8,FALSE);
                    generatekey(8,FALSE); //2 backspaces to delete ட்
                    generatekey(2980,TRUE); //2980
                    generatekey(3021,TRUE); //pulli
            }
            else if(previous_1_vkCode == 0x53) { //previouskey = s
                    generatekey(8,FALSE);
                    generatekey(8,FALSE); //2 backspaces to delete ச்
                    generatekey(2999,TRUE); //ஷ
                    generatekey(3021,TRUE); //pulli
            }
            else {
                    generatekey(3001,TRUE); //ஹ
                    generatekey(3021,TRUE); //pulli
            }
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x4A: //J/ஜ்
            if (previous_1_vkCode == 0x4E){
                    generatekey(8,FALSE);
                    generatekey(8,FALSE);
                    generatekey(2974,TRUE); //ஞ
                    generatekey(3021,TRUE); //pulli
            }
            else {
                    generatekey(2972,TRUE); //ஜ
                    generatekey(3021,TRUE); //pulli
            }

            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x4B: //K/க்
            generatekey(2965,TRUE); //க
            generatekey(3021,TRUE); //pulli
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x4C: //L/ல்
            if (shiftkey_pressed){
                    generatekey(2995,TRUE); //ஸ
                    generatekey(3021,TRUE); //pulli
            }
            else {
                    generatekey(2994,TRUE); //ல்
                    generatekey(3021,TRUE); //pulli
            }
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    // Z row keys
    case 0x5A: //Z / ழ்
            generatekey(2996,TRUE); //ழ
            generatekey(3021,TRUE); //pulli
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x58: //X/க்ஷ்
            generatekey(2965,TRUE); //க
            generatekey(3021,TRUE); //pulli
            generatekey(2999,TRUE); //க்ஷ
            generatekey(3021,TRUE); //pulli
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x43: //C/ச்
            generatekey(2970,TRUE); //ச
            generatekey(3021,TRUE); //pulli
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;


    case 0x56: //V/வ்
            generatekey(2997,TRUE); //வ
            generatekey(3021,TRUE); //pulli
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;


    case 0x42: //B/ப்
            generatekey(2986,TRUE); //ப
            generatekey(3021,TRUE); //pulli
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x4E: //N/ன்
            if (shiftkey_pressed){
                    generatekey(2979,TRUE); //ண
                    generatekey(3021,TRUE); //pulli
            }
            else {
                    generatekey(2985,TRUE); //ன
                    generatekey(3021,TRUE); //pulli
            }
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x4D: //M/ம்
            generatekey(2990,TRUE); //ம
            generatekey(3021,TRUE); //pulli
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0xBC: //comma
            generatekey(44,true); //comma
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0xBE: //full stop
            generatekey(46,true); //full stop
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0x08: //backspace
    case 0x20: //spacbar
    case 0x0D: //enter key
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0xDB: //[{
            generatekey(123,true);
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0xDD: //]}
            generatekey(125,true);
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

   case 0xBF: // / /?
            generatekey(47,true);
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0xBA: //;/?
            generatekey(59,true);
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0xDE: //'/?
            generatekey(39,true);
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    case 0xBD: //- key
            if (shiftkey_pressed){
                generatekey(95,true); // underscore _
            }
            else if(previous_1_vkCode == 0x4E){
                generatekey(8,FALSE);
                generatekey(8,FALSE);
                generatekey(2984,TRUE); //ந
                generatekey(3021,TRUE); //pulli
            }
            else {
                generatekey(45,true); // - or minus
            }
            previous_2_vkCode = previous_1_vkCode;
            previous_1_vkCode = current_vkCode;
            break;

    default:
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


bool Window::IsPrevkeyMey(int pos)
{
    if(pos == 1){
       if(meiezhuthukkal_phonetic.contains(previous_1_vkCode) ||((previous_2_character == 2984)&&(previous_1_character == 3021))){
                return true;
        }
        else{
                return false;
        }
    }
    else if ( pos == 2){
        if(meiezhuthukkal_phonetic.contains(previous_2_vkCode) ||((previous_3_character == 2984)&&(previous_2_character == 3021))){
                 return true;
         }
         else{
                 return false;
         }
    }
}
