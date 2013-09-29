# include "mainwindow.h"
# include "versioninfo.h"

# include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName(APPNAME);
    a.setApplicationVersion(APPVERSION);
    a.setOrganizationDomain(ORGDOMAIN);
    a.setOrganizationName(ORGNAME);

    MainWindow w;
    w.show();
    
    return a.exec();
}
