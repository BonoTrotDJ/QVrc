#include <QFile>
#include <QDir>
#include <QStringList>
#include <QCoreApplication>

#include "qlcconfig.h"
#include "qvrcinfo.h"

QString QVrcInfo::appVersion()
{
    QStringList paths;
    paths << QDir::current().absoluteFilePath("qvrc.opz");
    paths << QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("qvrc.opz");

    for (const QString& filePath : paths)
    {
        QFile file(filePath);
        if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;

        while (!file.atEnd())
        {
            QString line = QString::fromUtf8(file.readLine()).trimmed();
            if (line.isEmpty())
                continue;

            if (line.startsWith("version=", Qt::CaseInsensitive))
            {
                const QString version = line.section('=', 1).trimmed();
                if (!version.isEmpty())
                    return version;
            }
            else
            {
                return line;
            }
        }
    }

    return QString(APPVERSION);
}
