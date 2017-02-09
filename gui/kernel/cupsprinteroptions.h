#ifndef CUPSPRINTEROPTIONS_H
#define CUPSPRINTEROPTIONS_H

#include <QString>
#include <QSizeF>


class CupsPrinterOptions
{
public:
    CupsPrinterOptions(const QString &printerName);

    QString printerName() const {return  mPrinterName; }
    QString deviceURI() const {return  mDeviceURI; }

    QString grayScaleOption() const { return mGrayScaleOption; }
    QString colorOption() const { return mColorOption; }
    bool duplex() const { return mDuplex; }


    QSizeF paperSize() const { return mPaperSize; }

    qreal leftMargin()   const { return mLeftMargin; }
    qreal rightMargin()  const { return mRightMargin; }
    qreal topMargin()    const { return mTopMargin; }
    qreal bottomMargin() const { return mBottomMargin; }

private:
    QString mPrinterName;
    QString mDeviceURI;

    QString mColorOption;
    QString mGrayScaleOption;
    bool mDuplex;
    QSizeF mPaperSize;
    qreal mLeftMargin;
    qreal mRightMargin;
    qreal mTopMargin;
    qreal mBottomMargin;
};

#endif // CUPSPRINTEROPTIONS_H
