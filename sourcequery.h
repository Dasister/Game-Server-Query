#ifndef SOURCEQUERY_H
#define SOURCEQUERY_H

#include <QObject>
#include <QUdpSocket>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QVector>
#ifdef QT_DEBUG
#include <QDebug>
#endif

enum ServerType
{
    Type_Source = 0,
    Type_GoldSrc,
    Type_TheShip,
};

const QByteArray A2S_INFO = "\xFF\xFF\xFF\xFF\x54Source Engine Query";
const QByteArray A2S_PLAYERS = "\xFF\xFF\xFF\xFF\x55";
const QByteArray A2S_RULES = "\xFF\xFF\xFF\xFF\x56";
const QByteArray A2S_CHALLENGE = "0xFFFFFFFF";

const QByteArray S2A_PLAYERS = "\x44";
const QByteArray S2A_RULES = "\x45";

struct Players
{
    QString name, timef;
    int id, frags;
    float time;
};

class SourceQuery : public QObject
{
    Q_OBJECT
public:
    explicit SourceQuery(QObject *parent = 0);
    ~SourceQuery();

    void connectTo(QHostAddress addr, quint16 port);
    void disconnectFrom();
    QMap <QString, QVariant> getInfo();
    QVector <Players> getPlayers();
    QMap <QString, QString> getRules();
    
signals:
    
public slots:

private:
    QVariant getChallange(QByteArray header, QByteArray expected);
    int getByte(QByteArray &packet);
    QString getString(QByteArray &packet);
    int getShort(QByteArray &packet);
    long long getLong(QByteArray &packet);
    long long getLongLong(QByteArray &packet);
    float getFloat(QByteArray &packet);

    QByteArray _challenge;
    QHostAddress _addr;
    quint16 _port;
    QUdpSocket *_sock;
    
};

#endif // SOURCEQUERY_H
