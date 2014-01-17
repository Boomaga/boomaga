#ifndef JOB_H
#define JOB_H

#include <QList>
#include <QObject>
#include <QString>
#include "inputfile.h"

class ProjectPage;

class Job : public QObject
{
    Q_OBJECT
public:
    explicit Job(const InputFile &inputfile, QObject *parent=0);
    explicit Job(const Job *other, QObject *parent=0);
    virtual ~Job();

    int pageCount() const { return mPages.count(); }
    ProjectPage *page(int index) const { return mPages[index]; }

    int indexOfPage(ProjectPage *page, int from = 0) const { return mPages.indexOf(page, from); }
    void addPage(ProjectPage *page);
    void removePage(ProjectPage *page) { takePage(page); }
    ProjectPage *takePage(ProjectPage *page);

    QString title() const { return mTitle; }
    void setTitle(const QString &title);

    InputFile inputFile() const { return mInputFile; }
private:
    QList<ProjectPage*> mPages;
    QString mTitle;
    InputFile mInputFile;
};


class JobList: public QList<Job*>
{
public:
    JobList();
    JobList(const QList<Job*> & other);

    QList<InputFile> inputFiles() const;
    int indexOfInputFile(const InputFile &inputFile, int from = 0) const;
};
#endif // JOB_H
