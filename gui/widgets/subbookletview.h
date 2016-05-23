#ifndef SUBBOOKLETVIEW_H
#define SUBBOOKLETVIEW_H

#include <QWidget>
#include "pagelistview.h"

class SubBookletView : public PagesListView
{
    Q_OBJECT
public:
    explicit SubBookletView(QWidget *parent = 0);

protected:
    QList<ItemInfo> getPages() const;

};

#endif // SUBBOOKLETVIEW_H
