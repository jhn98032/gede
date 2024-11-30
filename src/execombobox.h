#ifndef FILE__EXECOMBOBOX_H
#define FILE__EXECOMBOBOX_H

#include <QComboBox>


class ExeComboBox : public QComboBox
{
    Q_OBJECT
    public:

    enum SearchAreas
    {
        UseEnvPath = (0x1<<0),
        UseCurrentDir = (0x1<<1),
    };
    
    ExeComboBox(QWidget *parent = NULL);
    virtual ~ExeComboBox();

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    void setFilter(QRegularExpression filter) { m_filter = filter; };
#else
    void setFilter(QRegExp filter) { m_filter = filter; };
#endif

    void setSearchAreas(int areas) { m_areas = areas; };

private:
    void fillIn();
    void showPopup();
    
private:
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    QRegularExpression m_filter;
#else
    QRegExp m_filter;
#endif
    int m_areas;
};



#endif // FILE__EXECOMBOBOX_H
