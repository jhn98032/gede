#include "execombobox.h"

#include <QDir>

#include <stdlib.h>


ExeComboBox::ExeComboBox(QWidget *parent)
 : QComboBox(parent)
  ,m_filter(".*")
  ,m_areas(UseEnvPath)
{

}

ExeComboBox::~ExeComboBox()
{

}

void ExeComboBox::showPopup ()
{
    if(count() == 0)
    {
        fillIn();
    }

    QComboBox::showPopup();
}



/**
 * @brief Searches for and fills in any gdb instances found.
 */
void ExeComboBox::fillIn()
{
    QStringList pathList;
    if((m_areas & UseEnvPath) == UseEnvPath)
    {
        const char *pathStr = getenv("PATH");
        if(pathStr)
            pathList = QString(pathStr).split(":");
    }
    if((m_areas & UseCurrentDir) == UseCurrentDir)
    {
        pathList.append("./");
    }
    for(int i = 0;i < pathList.size();i++)
    {
        QString exePath = pathList[i];
        
        QDir dir(exePath);
        dir.setFilter(QDir::Files | QDir::Executable);
        QFileInfoList list = dir.entryInfoList();
        for (int i = 0; i < list.size(); ++i)
        {
            QFileInfo fileInfo = list.at(i);
            QString fileName = fileInfo.fileName();
            if(fileName.contains(m_filter))
            {
                addItem(fileInfo.filePath());
            }
        }
    }
    
}
