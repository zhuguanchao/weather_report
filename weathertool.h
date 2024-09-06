#ifndef WEATHERTOOL_H
#define WEATHERTOOL_H

#include <QString>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMap>
#include <QDebug>
#include <QTextCodec>
#include <QTextStream>
class WeatherTool
{
private:
    static QMap<QString,QString> mCityMap;
  public:
    static void initCityMap()
    {
        //1.读取文件
        QString filepath = ":/citycode.txt";
        QFile file(filepath);
        if(file.open(QIODevice::ReadOnly|QIODevice::Text))
        {
            qDebug()<<"打开文件成功";
        }
        QTextStream in(&file);
           while (!in.atEnd())
           {
               QString line = QString(in.readLine());
               if(!line.isEmpty())// 防止对空白行进行操作
               {
               QString  citycode = line.split("=").at(0);
               QString  cityname = line.split("=").at(1);
//             qDebug()<<cityname;
               mCityMap.insert(cityname,citycode);
               }
           }
        file.close();
}
public:
    static QString getCityCode(QString cityName)
{
    if(mCityMap.isEmpty())
    {
        initCityMap();

    }
    QMap<QString,QString>::iterator it = mCityMap.find(cityName);

    if(it == mCityMap.end())
    {
        it = mCityMap.find(cityName+"市");
    }
    if(it!=mCityMap.end())
    {
        return  it.value();
    }
    return "";
}


};

QMap<QString, QString> WeatherTool::mCityMap;

#endif // WEATHERTOOL_H
