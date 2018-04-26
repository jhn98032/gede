#ifndef FILE__RUSTTAGS_H
#define FILE__RUSTTAGS_H

#include "tagscanner.h"
#include "syntaxhighlighter.h"


class RustTagScanner
{
public:
    
    RustTagScanner();
    virtual ~RustTagScanner();

    int scan(QString filepath, QList<Tag> *taglist);
       




    void tokenize(QString text);

    //unsigned int getRowCount() { return m_rows.size(); };
    void reset();

    bool isKeyword(QString text) const;
    bool isSpecialChar(char c) const;
    bool isSpecialChar(TextField *field) const;
    void setConfig(Settings *cfg);

private:
    class Token
    {
        public:
        Token(int lineNr) : m_lineNr(lineNr) {};

        typedef enum {STRING, COMMENT, NUMBER,WORD, KEYWORD} Type;
        Type m_type;
        QString text;
        int m_lineNr;
    };

    void parse(QList<Tag> *taglist);
    QString tokenToDesc(Token *tok);

private:
    Token* popToken();
    Token* peekToken();
    Token* pushToken(QString text, Token::Type type, int lineNr);
    Token* pushToken(char text, Token::Type type, int lineNr);
    void clearTokenList();
    
private:
    Settings *m_cfg;
    //QVector <Row*> m_rows;
    QHash <QString, bool> m_keywords;
    QHash <QString, bool> m_cppKeywords;
    QString m_filepath;
    QList<Token*> m_tokens;
};

#endif // FILE__RUSTTAGS_H
