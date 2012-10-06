#include "sourcequery.h"

SourceQuery::SourceQuery(QObject *parent) :
    QObject(parent)
{
}

SourceQuery::~SourceQuery()
{
    disconnectFrom();
    _sock->deleteLater();
}

void SourceQuery::connectTo(QHostAddress addr, quint16 port)
{
    //if (_sock)
    //{
    //    disconnectFrom();
    //    _sock->deleteLater();
    //}
    _addr = addr;
    _port = port;

    _sock = new QUdpSocket(this);
    _sock->connectToHost(addr, port);

}

QMap <QString, QVariant> SourceQuery::getInfo()
{
    _sock->write(A2S_INFO.data(), A2S_INFO.size() + 1);
    QByteArray buf, allPackets;
    QMap <QString, QVariant> result;

    _sock->waitForBytesWritten(1000);
    _sock->waitForReadyRead(1000);

    buf = _sock->read(4096);
    buf.remove(0, 4);
    if (buf[0] == '\x49')
    {
        allPackets.push_back(buf.mid(1));
        while ((buf = _sock->read(4096)) != "")
        {
            allPackets.push_back(buf);
        }

        result["ServerType"] = Type_Source;
        result["Protocol"] = getByte(allPackets);
        result["HostName"] = getString(allPackets);
        result["Map"] = getString(allPackets);
        result["GameDir"] = getString(allPackets);
        result["GameDesc"] = getString(allPackets);
        result["AppID"] = getShort(allPackets);
        result["Players"] = getByte(allPackets);
        result["MaxPlayers"] = getByte(allPackets);
        result["Bots"] = getByte(allPackets);
        char type = (char)getByte(allPackets);
        if (type == 'l')
            result["Dedicated"] = "Listen";
        else if (type == 'd')
            result["Dedicated"] = "Dedicated";
        else
            result["Dedicated"] = "SourceTV";
        if ((char)getByte(allPackets) == 'w')
            result["OS"] = "Windows";
        else
            result["OS"] = "Linux";

        result["Password"] = (bool)getByte(allPackets);
        result["Secured"] = (bool)getByte(allPackets);
        if (result["AppID"].toInt() == 2400)
        {
            result["ServerType"] = Type_TheShip;
            result["GameMode"] = getByte(allPackets);
            result["WitnessCount"] = getByte(allPackets);
            result["WitnessTime"] = getByte(allPackets);
        }
        result["Version"] = getString(allPackets);

        if (allPackets.size() > 0)
        {
            int edf = getByte(allPackets);

            if (edf & 0x80)
                result["GamePort"] = getShort(allPackets);

            if (edf & 0x10)
                result["ServerSteamID"] = getLongLong(allPackets);

            if (edf & 0x40)
            {
                result["SpecPort"] = getShort(allPackets);
                result["SpecName"] = getString(allPackets);
            }

            if (edf & 0x20)
                result["Tags"] = getString(allPackets);
        }
    }
    else if (buf[0] == '\x6D')
    {
        allPackets.push_back(buf.mid(1));
        result["ServerType"] = Type_GoldSrc;
        result["GameIP"] = getString(allPackets);
        result["HostName"] = getString(allPackets);
        result["Map"] = getString(allPackets);
        result["GameDir"] = getString(allPackets);
        result["GameDesc"] = getString(allPackets);
        result["Players"] = getByte(allPackets);
        result["MaxPlayers"] = getByte(allPackets);
        result["Version"] = getByte(allPackets);
        char type = (char)getByte(allPackets);
        if (type == 'l')
            result["Dedicated"] = "Listen";
        else if (type == 'd')
            result["Dedicated"] = "Dedicated";
        else
            result["Dedicated"] = "HLTV";
        if ((char)getByte(allPackets) == 'w')
            result["OS"] = "Windows";
        else
            result["OS"] = "Linux";

        result["Password"] = (bool)getByte(allPackets);
        result["IsMod"] = (bool)getByte(allPackets);
        if (result["IsMod"].toBool())
        {
            result["URLInfo"] = getString(allPackets);
            result["URLDownload"] = getString(allPackets);
            getByte(allPackets); // NULL-Byte
            result["ModVersion"] = getLong(allPackets);
            result["ModSize"] = getLong(allPackets);
            result["ServerSideOnly"] = (bool)getByte(allPackets);
            result["CustomDLL"] = (bool)getByte(allPackets);
        }
        result["Secure"] = (bool)getByte(allPackets);
        result["Bots"] = getByte(allPackets);
    }
    else
    {
        result["Error"] = true;
    }

    return result;
}

QVector <Players> SourceQuery::getPlayers()
{
    QVector <Players> result;
    QVariant _chel = getChallange(A2S_PLAYERS + A2S_CHALLENGE, S2A_PLAYERS);
    QByteArray packets;

    if ((_chel.type() == QVariant::Bool) && (_chel.toBool()))
    {
        packets = A2S_PLAYERS + _challenge;
        _sock->write(packets.data(), packets.size());
        _sock->waitForBytesWritten(1000);
        _sock->waitForReadyRead(1000);

        QByteArray buf = _sock->read(4096);
        packets = buf;

        while ((buf = _sock->read(4096)) != "")
        {
            packets.push_back(buf);
        }

        packets.remove(0, 4);
    }
    else
    {
        packets = _chel.toByteArray();
    }

    packets.replace("\xFF\xFF\xFF\xFF\x54", "");
    int num = getByte(packets);

    for (int i = 0; i < num; i++)
    {
        Players player;
        player.id = getByte(packets);
        player.name = getString(packets);
        player.frags = getLong(packets);
        player.time = (int)getFloat(packets);

        result.push_back(player);
    }

    return result;
}

QMap <QString, QString> SourceQuery::getRules()
{
    QMap <QString, QString> result;
    QVariant _chel = getChallange(A2S_RULES + A2S_CHALLENGE, S2A_RULES);
    QByteArray packets;

    if ((_chel.type() == QVariant::Bool) && (_chel.toBool()))
    {
        packets = A2S_RULES + _challenge;
        _sock->write(packets.data(), packets.size());
        _sock->waitForBytesWritten(1000);
        _sock->waitForReadyRead(1000);

        QByteArray buf = _sock->read(4096);
        packets = buf;

        while ((buf = _sock->read(4096)) != "")
        {
            packets.push_back(buf);
        }

        packets.remove(0, 8);
    }
    else
    {
        packets = _chel.toByteArray();
    }

    int num_rules = getShort(packets);

    // Server sends incomplete packets. Ignore "num_rules" value
    while (packets.size() > 0)
    {
        result[getString(packets)] = getString(packets);
    }

    return result;

}

int SourceQuery::getByte(QByteArray &packet)
{
    int ans = (int)packet[0];
    packet.remove(0, 1);
    return ans;
}

QString SourceQuery::getString(QByteArray &packet)
{
    QByteArray ans = "";
    int it = 0;
    while (packet[it] != '\0')
    {
        ans.append(packet[it]);
        it++;
    }
    packet.remove(0, it + 1);
    return ans;
}

int SourceQuery::getShort(QByteArray &packet)
{
    QByteArray ans = packet.mid(0, sizeof(short));
    packet.remove(0, sizeof(short));
    return *((short*)ans.data());
}

long long SourceQuery::getLong(QByteArray &packet)
{
    QByteArray ans = packet.mid(0, sizeof(long));
    packet.remove(0, sizeof(long));
    return *((long*)ans.data());
}

long long SourceQuery::getLongLong(QByteArray &packet)
{
    QByteArray ans = packet.mid(0, sizeof(long long));
    packet.remove(0, sizeof(long long));
    return *((long long*)ans.data());
}

float SourceQuery::getFloat(QByteArray &packet)
{
    QByteArray ans = packet.mid(0, sizeof(float));
    packet.remove(0, sizeof(float));
    return *((float*)ans.data());
}

QVariant SourceQuery::getChallange(QByteArray header, QByteArray expected)
{
    if (_challenge != "")
        return true;

    _sock->write(header.data(), header.size());
    _sock->waitForBytesWritten(1000);

    QByteArray ans;
    while (_sock->waitForReadyRead(1000))
    {

        QByteArray buf = _sock->read(4096);
        ans.push_back(buf);
        while ((buf = _sock->read(4096)) != "")
        {
            ans.push_back(buf);
        }
    }

    ans.remove(0, 4);

    if (ans[0] == '\x41')
    {
        _challenge = ans.mid(1);
        return true;
    }
    else if (ans.at(0) == expected.at(0))
        return ans.mid(1);

    return false;

}

void SourceQuery::disconnectFrom()
{
    if (_sock->isOpen())
    {
        _sock->disconnectFromHost();
        _challenge = "";
    }
}
