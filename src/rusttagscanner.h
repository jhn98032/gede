#ifndef FILE__RUSTTAGS_H
#define FILE__RUSTTAGS_H

#include "tagscanner.h"
#include "syntaxhighlighter.h"


/**
 * @brief Tag scanner for the Rust language.
 *
 * This class scans a rust language file and extract function definitions from it.
 */
class RustTagScanner
{
public:
    
    RustTagScanner();
    virtual ~RustTagScanner();

    int scan(QString filepath, QList<Tag> *taglist);
       

    void setConfig(Settings *cfg);

private:

    void tokenize(QString text);

    void reset();

    bool isKeyword(QString text) const;
    bool isSpecialChar(char c) const;
    bool isSpecialChar(TextField *field) const;

private:
    class Token
    {
    public:
        typedef enum {STRING, COMMENT, NUMBER,WORD, KEYWORD} Type;

        Token(int lineNr) : m_lineNr(lineNr) {};
        Token(int lineNr, Type t) : m_type(t), m_lineNr(lineNr) {};

        QString getText() const { return text;};
        void setText(QString txt) { text = txt;};

        QString toDesc();
        
        void setType(Type t) { m_type = t; };
        Type getType() const { return m_type; };

        int getLineNr() const { return m_lineNr; };

    private:
        Type m_type;
        QString text;
        int m_lineNr;
    };

    void parse(QList<Tag> *taglist);

private:
    bool eatToken(QString text);
    Token* popToken();
    Token* peekToken();
    Token* pushToken(QString text, Token::Type type, int lineNr);
    Token* pushToken(char text, Token::Type type, int lineNr);
    void clearTokenList();
    
private:
    Settings *m_cfg;
    QHash <QString, bool> m_keywords;
    QString m_filepath;
    QList<Token*> m_tokens;
};

#endif // FILE__RUSTTAGS_H
