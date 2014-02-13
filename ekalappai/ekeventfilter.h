#ifndef EEVENTFILTER_H
#define EEVENTFILTER_H

#include <QAbstractNativeEventFilter>
#include <window.h>

class EKEventFilter : public  QAbstractNativeEventFilter
{
public :
   EKEventFilter();
   Window* window;

    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);
};

#endif // EEVENTFILTER_H
