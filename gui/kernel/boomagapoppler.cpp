/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2014 Boomaga team https://github.com/Boomaga
 * Authors:
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */


#include "boomagapoppler.h"

#include <QDebug>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

//#include <poppler/PDFDoc.h>
#include <poppler/GlobalParams.h>
#include <poppler/poppler-config.h>

#ifdef __GNUC__
#define GCC_VARIABLE_IS_USED __attribute__ ((unused))
#else
#define GCC_VARIABLE_IS_USED
#endif

static GBool GCC_VARIABLE_IS_USED printVersion = true;
static GBool GCC_VARIABLE_IS_USED printHelp = true;

void initPoppler()
{
    static bool popplerGlobalParamsInited(false);
    if (!popplerGlobalParamsInited)
    {
        globalParams = new GlobalParams();
        popplerGlobalParamsInited = true;
    }
}


class PJLFileStream: public FileStream
{
public:
    PJLFileStream(FILE *fA, Guint startA, GBool limitedA, Guint lengthA, Object *dictA):
        FileStream(fA, startA, limitedA, lengthA, dictA),
        mStartPos(startA),
        mEndPos(startA + lengthA),
        mLength(lengthA),
        mFile(fA)
    {
    }

    ~PJLFileStream()
    {
        close();
    }

    virtual int getPos()
    {
        return FileStream::getPos() - mStartPos;
    }

    virtual void setPos(Guint pos, int dir = 0)
    {
        if (dir<0)
            FileStream::setPos(mEndPos - pos, 0);
        else
            FileStream::setPos(pos, dir);
    }

    virtual void close()
    {
        FileStream::close();
        fclose(mFile);
    }

    FILE *fileHandler() { return mFile; }
private:
    qint64 mStartPos;
    qint64 mEndPos;
    qint64 mLength;
    FILE  *mFile;
};


/************************************************
 *
 * ***********************************************/
PJLFileStream * createStream(const QString &fileName, qint64 startPos, qint64 endPos)
{
    initPoppler();
    Object obj;
    obj.initNull();
    FILE *file = fopen(fileName.toLocal8Bit(), "rb");
    if (file == NULL)
    {
        qWarning() << strerror(errno);
        throw QString(strerror(errno));
        return 0;
    }

    return new PJLFileStream(file,
                             startPos,
                             false,
                             endPos - startPos,
                             &obj);
}


/************************************************
 *
 * ***********************************************/
BoomagaPDFDoc::BoomagaPDFDoc(const QString &fileName, qint64 startPos, qint64 endPos):
    PDFDoc(createStream(fileName, startPos, endPos)),
    f(0),
    mValid(true)
{
    if (!isOk())
    {
        mErrorString = QObject::tr("PDF file \"%1\" is damaged.").arg(fileName);
        mValid = false;
    }

    if (isEncrypted())
    {
        mErrorString = QObject::tr("PDF file \"%1\" is encripted.").arg(fileName);
        mValid = false;
    }
}


/************************************************
 *
 * ***********************************************/
QString BoomagaPDFDoc::getMetaInfo(const char *tag)
{
    QString result;
    Object docInfo;

    getDocInfo(&docInfo);
    if (docInfo.isDict())
    {
        Dict *dict = docInfo.getDict();
        Object obj;
        dict->lookup((char*)tag, &obj);
        if (obj.isString())
        {
            GooString *s = obj.getString();
            if (s->getLength() > 1 &&
                s->getChar(0) == '\xFE' &&
                s->getChar(1) == '\xFF')
            {
                result = QString::fromUtf16((ushort*)s->getCString());
            }
            else
            {
                result = QString::fromLatin1(s->getCString());
            }
        }

        obj.free();

    }
    docInfo.free();

    return result;
}
