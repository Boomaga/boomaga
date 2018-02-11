/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2017 Boomaga team https://github.com/Boomaga
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


#include "pdfobject.h"
#include "pdferrors.h"
#include "pdfvalue.h"
#include <zlib.h>

#include <QDebug>

namespace PDF {
class FlateDecodeStream: public QByteArray
{
public:
    FlateDecodeStream(const PDF::Dict &parameters, const QByteArray &source);

private:
    void unCompress(const QByteArray &source);
    void applyPNGPredictor();

    int mPredictor;
    int mColors;
    int mBitsPerComponent;
    int mColumns;
};

} // namespace PDF
using namespace PDF;


/************************************************
 *
 * PNG predictors - http://www.w3.org/TR/PNG/#9Filters
 ************************************************/
FlateDecodeStream::FlateDecodeStream(const Dict &parameters, const QByteArray &source)
{
    unCompress(source);

    // A code that selects the predictor algorithm, if any. If the value
    // of this entry is 1, the filter assumes that the normal algorithm
    // was used to encode the data, without prediction.
    // If the value is greater than 1, the filter assumes that the data
    // was differenced before being encoded, and Predictor selects the
    // predictor algorithm.
    mPredictor = parameters.value("Predictor").asNumber().value(1);
    if (mPredictor == 1)
        return;

    // (Used only if Predictor is greater than 1) The number of interleaved
    // color components per sample. Valid values are 1 to 4 in PDF 1.2 or
    // earlier and 1 or greater in PDF 1.3 or later. Default value: 1.
    mColors = parameters.value("Colors").asNumber().value(1);

    // (Used only if Predictor is greater than 1) The number of bits used
    // to represent each color component in a sample.
    // Valid values are 1, 2, 4, 8, and 16. Default value: 8.
    mBitsPerComponent = parameters.value("BitsPerComponent").asNumber().value(8);

    // (Used only if Predictor is greater than 1) The number of samples
    // in each row. Default value: 1.
    mColumns = parameters.value("Columns").asNumber().value(1);

    switch (mPredictor)
    {
    case 1:     // No prediction
        break;

    case 2:     // TIFF Predictor 2
        throw Error("FlateDecode Predictor 2 not implemented yet.");
        break;

    case 10:    // PNG prediction (on encoding, PNG None on all rows)
    case 11:    // PNG prediction (on encoding, PNG Sub on all rows)
    case 12: 	// PNG prediction (on encoding, PNG Up on all rows)
    case 13: 	// PNG prediction (on encoding, PNG Average on all rows)
    case 14:    // PNG prediction (on encoding, PNG Paeth on all rows)
    case 15: 	// PNG prediction (on encoding, PNG optimum)
        applyPNGPredictor();
        break;

    default:
        throw Error(QString("Unknown FlateDecode Predictor '%d'.").arg(mPredictor));
    }
}


/************************************************
 *
 ************************************************/
void FlateDecodeStream::unCompress(const QByteArray &source)
{
    // More typical zlib compression ratios are on the order of 2:1 to 5:1.
    resize(source.size() * 2);

    while (true)
    {
        uchar * dest = (uchar*)data();
        uLongf destSize = size();

        int ret = uncompress(dest, &destSize, reinterpret_cast<const uchar*>(source.data()), source.size());
        switch (ret)
        {
        case Z_OK:
            resize(destSize);
            break;

        case Z_MEM_ERROR:
            throw Error("Z_MEM_ERROR: Not enough memory");

        case Z_DATA_ERROR:
            throw Error("Z_DATA_ERROR: Input data is corrupted");

        case Z_BUF_ERROR:
            resize(size() * 2);
            continue;
        }

        break;
    }
}


/************************************************
 *
 ************************************************/
inline void pngPredictor0Row(const char *src, char *dest, int len)
{
    for (int i=0; i<len; ++i)
        dest[i] = src[i];
}


/************************************************
 *
 ************************************************/
inline void pngPredictor2Row(const char *src, const char *prev, char *dest, int len)
{
    for (int i=0; i<len; ++i)
        dest[i] = src[i] + prev[i];
}


/************************************************
 * https://www.w3.org/TR/2003/REC-PNG-20031110/#9Filters
 *
 * The postprediction data for each PNG-predicted
 * row begins with an explicit algorithm tag;
 ************************************************/
void FlateDecodeStream::applyPNGPredictor()
{
    char *data = this->data();
    int rowLen = mColors * mBitsPerComponent * mColumns / 8;

    // First row;
    for (int pos=1; pos<rowLen+1; ++pos)
        data[pos-1] = data[pos];

    int size = this->size();
    int w = rowLen;
    int prev = 0;
    for (int r=rowLen+1; r<size-rowLen;)
    {
        switch (data[r++])
        {
        case 0:
            pngPredictor0Row(data+r, data+w, rowLen);
            break;

        case 1:
            throw Error("PNG Predictor 1 not implemented yet.");

        case 2:
            pngPredictor2Row(data+r, data+prev, data+w, rowLen);
            break;

        case 3:
            throw Error("PNG Predictor 3 not implemented yet.");

        case 4:
            throw Error("PNG Predictor 4 not implemented yet.");

        default:
            throw Error("Unknown PNG predictor type");
        }

        r+=rowLen;
        w+=rowLen;
        prev+=rowLen;
    }
    resize(w);
}



/************************************************
 *
 ************************************************/
Object::Object(ObjNum objNum, GenNum genNum, const Value &value):
    mObjNum(objNum),
    mGenNum(genNum),
    mValue(value),
    mPos(0),
    mLen(0)
{
}


/************************************************
 *
 ************************************************/
Object::Object(const Object &other):
    mObjNum( other.mObjNum),
    mGenNum( other.mGenNum),
    mValue(  other.mValue),
    mStream( other.mStream),
    mPos(    other.mPos),
    mLen(    other.mLen)
{
}


/************************************************
 *
 ************************************************/
Object &Object::operator =(const Object &other)
{
    mObjNum  = other.mObjNum;
    mGenNum  = other.mGenNum;
    mValue   = other.mValue;
    mStream  = other.mStream;
    mPos     = other.mPos;
    mLen     = other.mLen;
    return *this;
}


/************************************************
 *
 ************************************************/
Object::~Object()
{

}


/************************************************
 *
 ************************************************/
void Object::setObjNum(ObjNum value)
{
    mObjNum = value;
}


/************************************************
 *
 ************************************************/
void Object::setGenNum(GenNum value)
{
    mGenNum = value;
}


/************************************************
 *
 ************************************************/
void Object::setValue(const Value &value)
{
    mValue = value;
}


/************************************************
 *
 ************************************************/
void Object::setStream(const QByteArray &value)
{
    mStream = value;
}


/************************************************
 *
 ************************************************/
QByteArray Object::decodedStream() const
{
    try
    {
        QStringList filters;
        const PDF::Value &v = dict().value("Filter");
        if (v.isName())
        {
            filters << v.asName().value();
        }
        else if (v.isArray())
        {
            const PDF::Array &arr = v.asArray();
            for (int i=0; i<arr.count(); ++i)
                filters << arr.at(i).asName().value();
        }

        QByteArray res = stream();
        foreach (const QString &filter, filters)
        {
            if (filter == "FlateDecode")
            {
                res = FlateDecodeStream(dict().value("DecodeParms").asDict(), res);
                continue;
            }

            if (filter == "ASCIIHexDecode"  ||
                    filter == "ASCII85Decode"   ||
                    filter == "LZWDecode"       ||
                    filter == "FlateDecode"     ||
                    filter == "RunLengthDecode" ||
                    filter == "CCITTFaxDecode"  ||
                    filter == "JBIG2Decode"     ||
                    filter == "DCTDecode"       ||
                    filter == "JPXDecode"       ||
                    filter == "Crypt"           )
            {
                throw Error(QString("Unsupported filter '%1'").arg(filter));
            }

            throw Error(QString("Unknown filter '%1'").arg(filter));
        }

        return res;

    }
    catch (PDF::Error &err)
    {
        throw ObjectError(err.what(), mObjNum, mGenNum);
    }

}


/************************************************
 *
 ************************************************/
QString Object::type() const
{
    return dict().value("Type").asName().value();
}


/************************************************
 *
 ************************************************/
QString Object::subType() const
{
    QString s = dict().value("Subtype").asName().value();
    if (s.isEmpty())
        return dict().value("S").asName().value();
    else
        return s;
}


/************************************************
 *
 ************************************************/
QByteArray Object::streamFlateDecode(const QByteArray &source) const
{
    QByteArray res;
    // More typical zlib compression ratios are on the order of 2:1 to 5:1.
    res.resize(source.size() * 2);

    while (true)
    {
        uchar * dest = (uchar*)res.data();
        uLongf destSize = res.size();

        int ret = uncompress(dest, &destSize, reinterpret_cast<const uchar*>(source.data()), source.length());
        switch (ret)
        {
        case Z_OK:
            res.resize(destSize);
            break;

        case Z_MEM_ERROR:
            qWarning("Z_MEM_ERROR: Not enough memory");
            return QByteArray();

        case Z_DATA_ERROR:
            qWarning("Z_DATA_ERROR: Input data is corrupted");
            return QByteArray();

        case Z_BUF_ERROR:
            res.resize(res.length() * 2);
            continue;
        }

        break;
    }


    return res;
}


/************************************************
 *
 ************************************************/
QDebug operator<<(QDebug dbg, const Object &obj)
{
    dbg.nospace() << obj.objNum() << " " << obj.genNum() << " obj\n";
    dbg.nospace() << obj.value();
    if (obj.stream().length())
        dbg.nospace() << "\n" << "stream length " << obj.stream().length();
    else
        dbg.nospace() << "\n" << "no stream";
    dbg.nospace() << "\nendobj\n";
    return dbg;
}
