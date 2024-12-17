#include "detectdistro.h"

#include <QStringList>
#include <QFile>
#include <QMap>
#include <QProcess>


/**
  * @brief Tries to detect the distro the system is running on.
  */
void detectDistro(DistroType *type, QString *distroDesc)
{
    QString machine = "";
    QString distroName = "";

    distroName = "Unknown OS";
    if(type)
        *type = DISTRO_UNKNOWN;

    // Check for Debian
    QFile file1("/etc/debian_version");
    if(file1.open(QIODevice::ReadOnly))
    {
        if(type)
            *type = DISTRO_DEBIAN;

        QString version = file1.readLine().trimmed();
        distroName = "Debian " + version;
    }

    // Parse lsb-release file
    QFile file2("/etc/lsb-release");
    if(file2.open(QIODevice::ReadOnly))
    {
        QMap<QString,QString> fields;
        // Parse ini-like structure
        while (!file2.atEnd())
        {
            QString line = file2.readLine().trimmed();
            QStringList tokens = line.split("=");
            if(tokens.size() == 2)
            {
                QString name = tokens[0].trimmed();
                QString data = tokens[1].trimmed();
                if(data.startsWith('"'))
                    data = data.mid(1, data.length()-2);
                fields[name] = data;


            }

        }
        
        if(fields.contains("DISTRIB_ID"))
        {
            QString distribId = fields["DISTRIB_ID"];
            if(type)
            {
                if(distribId == "Ubuntu")
                    *type = DISTRO_UBUNTU;
            }
            distroName = distribId;
            if(fields.contains("DISTRIB_RELEASE"))
                distroName += " " + fields["DISTRIB_RELEASE"];
        }
        if(fields.contains("DISTRIB_DESCRIPTION"))
            distroName = fields["DISTRIB_DESCRIPTION"];

    }


    // Detect x64/x86 
    QString versionStr;
    QProcess process;
    process.start("uname", 
        QStringList("-m"),
        QIODevice::ReadOnly | QIODevice::Text);
    if(!process.waitForFinished(2000))
    {
    }
    else
    {
        machine = process.readAllStandardOutput().trimmed();
    }


    if(distroDesc)
    {
        *distroDesc = distroName;
        if(!machine.isEmpty())
            *distroDesc += " " + machine;
    }
}



