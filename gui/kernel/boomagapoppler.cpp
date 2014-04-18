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

#if POPPLER_VERSION >= 2300
#include <poppler/goo/gfile.h>
#endif


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




class PJLFileStreamData
{
public:
    PJLFileStreamData(const QString &fileName, qint64 startPos, qint64 endPos):
        mStartPos(startPos),
        mEndPos(endPos),
        mLength(endPos - startPos)
    {
        initPoppler();
#if POPPLER_VERSION < 2300
        mFile = fopen(fileName.toLocal8Bit(), "rb");
#else
        GooString f(fileName.toLocal8Bit());
        mFile = GooFile::open(&f);
#endif

        if (!mFile)
            mErrorString = QString::fromLocal8Bit(strerror(errno));

    }

    ~PJLFileStreamData()
    {
#if POPPLER_VERSION < 2300
        fclose(mFile);
#else
         delete mFile;
#endif
    }

    qint64 mStartPos;
    qint64 mEndPos;
    qint64 mLength;
#if POPPLER_VERSION < 2300
    FILE *mFile;
#else
    GooFile *mFile;
#endif
    QString mErrorString;
};


class PJLFileStream: protected PJLFileStreamData, public FileStream
{
public:
    PJLFileStream(const QString &fileName, qint64 startPos, qint64 endPos):
        PJLFileStreamData(fileName, startPos, endPos),
        FileStream(mFile, mStartPos, false, mLength, new Object())
    {
    }

    ~PJLFileStream()
    {
        close();
    }

#if POPPLER_VERSION < 2300
    virtual int     getPos()
#else
    virtual Goffset getPos()
#endif
    {
        return FileStream::getPos() - mStartPos;
    }

#if POPPLER_VERSION < 2300
    virtual void setPos(Guint   pos, int dir = 0)
#else
    virtual void setPos(Goffset pos, int dir = 0)
#endif
    {
        if (dir<0)
            FileStream::setPos(mEndPos - pos, 0);
        else
            FileStream::setPos(pos, dir);
    }

    virtual void close()
    {
        FileStream::close();

    }
};


/************************************************
 *
 * ***********************************************/
BoomagaPDFDoc::BoomagaPDFDoc(const QString &fileName, qint64 startPos, qint64 endPos):
    PDFDoc(new PJLFileStream(fileName, startPos, endPos)),
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



/************************************************
 *
 * ***********************************************/
GooString *createGooString(const QString string)
{
    return new GooString(string.toLocal8Bit(), string.length());
}
