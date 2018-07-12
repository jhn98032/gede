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

    void setFilter(QRegExp filter) { m_filter = filter; };

    void setSearchAreas(int areas) { m_areas = areas; };

private:
    void fillIn();
    void showPopup();
    
private:
    QRegExp m_filter;
    int m_areas;
};



#endif // FILE__EXECOMBOBOX_H
