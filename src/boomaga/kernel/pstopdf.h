#ifndef PSTOPDF_H
#define PSTOPDF_H

#include <QObject>
#include <QProcess>

class PsToPdf: public QObject
{
    Q_OBJECT
public:
    PsToPdf(QObject *parent=nullptr);
    virtual ~PsToPdf();
    void execute(const std::string &psFile, const std::string &pdfFile);
    void execute(std::ifstream &psStream, const std::string &pdfFile);

public slots:
    void terminate();

private:
    QProcess mProcess;
};

#endif // PSTOPDF_H
