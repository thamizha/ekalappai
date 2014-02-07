#include "ekeventfilter.h"
#include <QDebug>
#include <windows.h>

EKEventFilter::EKEventFilter(){

}

 bool EKEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
    Q_DECL_OVERRIDE
 {
    const MSG &msg = *static_cast<MSG *>(message);
    UINT msgInt = msg.message;

    switch ( msgInt )
    {
        case WM_USER+755:
           window->processKeypressEvent();
           break;

        default:
            break;
    }

    return false;

 }
