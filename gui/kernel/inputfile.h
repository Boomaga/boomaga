#ifndef INPUTFILE_H
#define INPUTFILE_H

#include <QString>
#include <QVector>

class ProjectPage;
class Job;

class InputFile
{
public:
    explicit InputFile(const QString &fileName, int pageCount);
    explicit InputFile(const Job &job, int pageCount);
    virtual ~InputFile();

    QString fileName() const { return mFileName; }

    QString title() const { return mTitle; }
    void setTile(const QString &value) { mTitle = value; }

    int pageCount() const { return mPages.count(); }
    QVector<ProjectPage*> pages() { return mPages; }
    QVector<ProjectPage*> pages() const { return mPages; }

    bool autoRemove() const {return mAutoRemove;}
    void setAutoRemove(bool value) { mAutoRemove = value; }

protected:
    QVector<ProjectPage*> mPages;

private:
    QString mFileName;
    QString mTitle;
    bool mAutoRemove;
};


#endif // INPUTFILE_H
