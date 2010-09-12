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
#ifndef WINDOW_H
#define WINDOW_H

#include <QSystemTrayIcon>
#include <QDialog>
#include <windows.h>
#include <QMap>

QT_BEGIN_NAMESPACE
class QAction;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QPushButton;
class QSpinBox;
class QTextEdit;
class QLibrary;
class QSettings;
QT_END_NAMESPACE


class Window : public QDialog
{
    Q_OBJECT

public:
    Window();

    void setVisible(bool visible);

    //choosen keyboard
    int selected_keyboard;

    //status of the current keyboard including no keyboard
    int current_keyboard;

    //keyboard status
    bool keyboard_status;

    virtual bool winEvent( MSG* message, long* result );
    QMap<QString, QString> keyboardmap ;


protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void setIcon(int index);
    void setShortcut1(int index);
    void setShortcut2(int index);
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void showTrayMessage(int index);
    void processKeypressEvent();

private:
    void createIconGroupBox();
    void createShortcutGroupBox();
    void createActions();
    void createTrayIcon();
    void callHook(int kb_index);
    void removeHook();
    void changeKeyboard(int index);
    void implementKeyboardLogic();
    void generatekey(int,bool);
    bool SearchArray (DWORD array[], DWORD key, int length);
    void loadKeyBoard();
    void generateUnicodeCharacters(QString characters);

    QGroupBox *iconGroupBox;
    QLabel *iconLabel;
    QComboBox *iconComboBox;
    QCheckBox *showIconCheckBox;

    QGroupBox *shortcutGroupBox;
    QLabel *shortcutLabel1;
    QLabel *shortcutLabel2;
    QComboBox *shortcutComboBox1;
    QComboBox *shortcutComboBox2;

    QAction *minimizeAction;
    QAction *maximizeAction;
    QAction *settingsAction;
    QAction *quitAction;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

    QLibrary *myLib;
    QSettings *settings;
    QSettings *keyrules;

    HHOOK hkb;

    typedef HHOOK (*MyPrototype)(HINSTANCE, bool, HWND);
    typedef void (*CleanupHook)(HHOOK);
    typedef void (*DisableKeyboard)();
    typedef bool (*GetAltKeyPress)();
    typedef DWORD (*GetKeyPress)();
    typedef void (*GenerateKey)(int, bool);
    typedef bool (*GetShiftKeyPress)();
    typedef bool (*GetControlKeyPress)();
    typedef WORD (*GetCharPressed)();

    GenerateKey generatekeyLib;

    BOOL controlkey_pressed;
    BOOL altkey_pressed;

    DWORD short_cut_key_hex;
    QString short_cut_key;
    QString shortcut_modifier_key;

    DWORD current_vkCode;

    //need to store them in an array later so that the input characters length can be handled dynamically
    WORD character_pressed;
    WORD prev1_character_pressed;
    WORD prev2_character_pressed;
    WORD prev3_character_pressed;
    WORD prev4_character_pressed;
    WORD prev5_character_pressed;

    //string that stores upto 20 characters of the english alphabets that corresponds to the keys pressed
    QString charpressed_string20;

    //need to change their names to character typed & also similarly put them in array.
    int previous_1_character;
    int previous_2_character;
    int previous_3_character;
    int previous_4_character;


    //current and previous lengths of the unicode characters printed
    int current_unicode_character_length;
    int prev_unicode_character_length;

    QVector<DWORD> keystrokes;
    QVector<DWORD> valid_keys;

};

#endif
