#include "opendialog.h"
#include "version.h"
#include <QFileDialog>
#include "log.h"
#include "util.h"


OpenDialog::OpenDialog(QWidget *parent)
    : QDialog(parent)
{
    
    m_ui.setupUi(this);

    connect(m_ui.pushButton_selectFile, SIGNAL(clicked()), SLOT(onSelectProgram()));
}



QString OpenDialog::getProgram()
{
    return m_ui.lineEdit_program->text();
}

QString OpenDialog::getArguments()
{
    return m_ui.lineEdit_arguments->text();
}
    

void OpenDialog::setProgram(QString program)
{
    m_ui.lineEdit_program->setText(program);

}
    

void OpenDialog::setArguments(QString arguments)
{
    m_ui.lineEdit_arguments->setText(arguments);

}

void OpenDialog::onSelectProgram()
{

    // Get start dir
    QString startPath = m_ui.lineEdit_program->text();
    if(!startPath.isEmpty())
    {
        dividePath(startPath, NULL, &startPath);
    }
    else
        startPath = "/";
        
    // Open dialog
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Select Program"), startPath, tr("All Files (*.*)"));

    // Fill in the selected path
    m_ui.lineEdit_program->setText(fileName);
}
