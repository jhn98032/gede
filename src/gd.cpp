#include <QtGui/QApplication>
#include "mainwindow.h"
#include "core.h"
#include "log.h"
#include "util.h"
#include <QMessageBox>
#include "tree.h"
#include "opendialog.h"


static int dumpUsage()
{
    /*
    QMessageBox::information ( NULL, "Unable to start",
                    "Usage: gd --args PROGRAM_NAME",
                    QMessageBox::Ok, QMessageBox::Ok);
      */
    printf("Usage: gd --args PROGRAM_NAME [PROGRAM_ARGUMENTS...]\n"
           "\n"
           );
    
    return -1;  
}

/**
 * @brief Main program entry.
 */
int main(int argc, char *argv[])
{
    /*
    Tree tree;
    tree.fromString("^done,bkpt={number=\"1\",type=\"breakpoint\",disp=\"keep\",enabled=\"y\","
        "addr=\"0x000000000040051b\",func=\"main\",file=\"test.c\","
        "fullname=\"/work2/priv/gd_wip/testapp/test.c\",line=\"5\",times=\"0\",original-location=\"main\"}");
    tree.dump();


    const char *path = "/bkpt/number";
    infoMsg("%s='%s'\n", path, stringToCStr(tree.getString(path)));
    return 0;

*/
    QStringList argumentList;
    
    for(int i = 1;i < argc;i++)
    {
        const char *curArg = argv[i];
        if(strcmp(curArg, "--args") == 0)
        {
            for(int u = i+1;u < argc;u++)
                argumentList.push_back(argv[u]);
            argc = i;
        }
        else if(strcmp(curArg, "--help") == 0)
        {
            return dumpUsage();
        }
    }

    QApplication a(argc, argv);

    if(argumentList.size() < 1)
    {
        OpenDialog dlg(NULL);
        if(dlg.exec() != QDialog::Accepted)
            return 1;
        argumentList.clear();
        argumentList += dlg.getProgram();
        argumentList += dlg.getArguments().split(' ');
    }

    Core &core = Core::getInstance();

    
    MainWindow w(NULL);

    core.init(argumentList);
    w.insertSourceFiles();
    
    w.show();

    return a.exec();

}

