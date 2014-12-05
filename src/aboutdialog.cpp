#include "aboutdialog.h"
#include "version.h"


AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    
    m_ui.setupUi(this);

    //
    QString verStr;
    verStr.sprintf("Version: v%d.%d.%d", GD_MAJOR, GD_MINOR, GD_PATCH);
    m_ui.label_version->setText(verStr);

    //
    QString buildStr;
    buildStr = __DATE__;
    buildStr += " ";
    buildStr += __TIME__;
    m_ui.label_buildDate->setText("Built: " + buildStr);

}


    
