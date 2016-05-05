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
#include <QFileInfo>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

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
    if (!globalParams)
        globalParams = new GlobalParams();
}



class PJLFileStreamData
{
public:
    PJLFileStreamData(const QString &fileName, qint64 startPos, qint64 endPos):
        mFileName(fileName),
        mStartPos(startPos),
        mEndPos(endPos),
        mValid(true)
    {
        initPoppler();

        if (!mEndPos)
            mEndPos = QFileInfo(fileName).size();

        mLength = mEndPos - mStartPos;
        openFile();
    }

    ~PJLFileStreamData()
    {
        closeFile();
    }


#if POPPLER_VERSION < 2300
    FILE *mFile;
    void openFile()
    {
        mFile = fopen(mFileName.toLocal8Bit(), "rb");
        if (!mFile)
        {
            mErrorString = QString::fromLocal8Bit(strerror(errno));
            mValid = false;
            mFile = stdout; // Fake for preserve segfaults in Poppler::FileStream
        }
    }

    void closeFile()
    {
        fclose(mFile);
    }

#else
    GooFile *mFile;
    void openFile()
    {
        GooString f(mFileName.toLocal8Bit());
        mFile = GooFile::open(&f);

        if (!mFile)
        {
            mValid = false;
            mErrorString = QString::fromLocal8Bit(strerror(errno));
        }
    }

    void closeFile()
    {
        delete mFile;
    }
#endif

    QString mFileName;
    qint64 mStartPos;
    qint64 mEndPos;
    qint64 mLength;
    QString mErrorString;
    bool mValid;
};


class PJLFileStream: protected PJLFileStreamData, public FileStream
{
public:
    PJLFileStream(const QString &fileName, qint64 startPos, qint64 endPos):
        PJLFileStreamData(fileName, startPos, endPos),
        FileStream(mFile, mStartPos, true, mLength, new Object())
    {
    }

    ~PJLFileStream()
    {
        close();
    }


    bool isValid() const { return mValid; }
    QString errorString() const { return mErrorString; }

};


/************************************************
 *
 * ***********************************************/
BoomagaPDFDoc::BoomagaPDFDoc(const QString &fileName, qint64 startPos, qint64 endPos):
    PDFDoc(new PJLFileStream(fileName, startPos, endPos)),
    mValid(true)
{    
    PJLFileStream *stream = static_cast<PJLFileStream*>(getBaseStream());
    if (!stream->isValid())
    {
        mErrorString = stream->errorString();
        mValid = false;
        return;
    }

    if (!isOk())
    {
        mErrorString = QObject::tr("PDF file \"%1\" is damaged.").arg(fileName);
        mValid = false;
        return;
    }

    if (isEncrypted())
    {
        mErrorString = QObject::tr("PDF file \"%1\" is encripted.").arg(fileName);
        mValid = false;
        return;
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
                result = QString::fromUtf16((ushort*)s->getCString(), s->getLength() / 2);
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
GooString *createGooString(const QString &string)
{
    return new GooString(string.toLocal8Bit(), string.length());
}
