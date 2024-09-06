#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QContextMenuEvent>
#include <QDebug>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QPainter>
#include "weathertool.h"
#include <QPen>

#define INCREMENT 3 // 温度每升高/降低1°，y轴坐标的增量
#define POINT_RADIUS 3 // 曲线描点的大小
#define TEXT_OFFSET_X 12
#define TEXT_OFFSET_Y 12

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowFlag(Qt::FramelessWindowHint);//设置窗口无边框
    this->setFixedSize(width(),height());//设置固定窗口大小
    //构建右键菜单
    mExitMenu = new QMenu(this);
    mExitAct = new QAction();
    mExitAct->setText("退出");
    mExitAct->setIcon(QIcon(":/res/close.png"));
    mExitAct->setIconVisibleInMenu(true);//设置上面的图标是否显示
    mExitMenu->addAction(mExitAct);
    connect(mExitAct,&QAction::triggered,this,[=]{
        qApp->exit(0);
//        close();
    });
    ui->lblganmao->setWordWrap(true);
    //将控件添加到控件数组里面，方便使用循环进行处理
    // 星期和日期
    mWeekList<<ui->lblWeek0_2<<ui->lblWeek1<<ui->lblWeek2<<ui->lblWeek3<<ui->lblWeek4<<ui->lblWeek5;
    mDateList<<ui->lblDate0<<ui->lblDate1<<ui->lblDate2<<ui->lblDate3<<ui->lblDate4<<ui->lblDate5;
    // 天气和天气图标
    mTypeList<<ui->lblType0<<ui->lblType1<<ui->lblType2<<ui->lblType3<<ui->lblType4<<ui->lblType5;
    mTypeIconList<<ui->lblTypeIcon0<<ui->lblTypeIcon1<<ui->lblTypeIcon2<<ui->lblTypeIcon3<<ui->lblTypeIcon4<<ui->lblTypeIcon5;
    // 天气指数
    mAqiList<<ui->lblQuality0<<ui->lblQuality1<<ui->lblQuality2<<ui->lblQuality3<<ui->lblQuality4<<ui->lblQuality5;
    // 风向和风力
    mFlList<<ui->lblFl0<<ui->lblFl1<<ui->lblFl2<<ui->lblFl3<<ui->lblFl4<<ui->lblFl5;
    mFxList<<ui->lblFx0<<ui->lblFx1<<ui->lblFx2<<ui->lblFx3<<ui->lblFx4<<ui->lblFx5;
    mTypeMap.insert("暴雪",":/type/res/type/BaoXue.png");
    mTypeMap.insert("暴雨",":/type/res/type/BaoYu.png");
    mTypeMap.insert("暴雨到大暴雨",":/type/res/type/BaoYuDaoDaBaoYu.png");
    mTypeMap.insert("大暴雨",":/type/res/type/DaBaoYu.png");
    mTypeMap.insert("大暴雨到特大暴雨",":/type/res/type/DaBaoYuDaoTeDaBaoYu.png");
    mTypeMap.insert("大雪",":/type/res/type/DaXue.png");
    mTypeMap.insert("大雨",":/type/res/type/DaYu.png");
    mTypeMap.insert("冻雨",":/type/res/type/DongYu.png");
    mTypeMap.insert("多云",":/type/res/type/DuoYun.png");
    mTypeMap.insert("浮沉",":/type/res/type/FuChen.png");
    mTypeMap.insert("雷阵雨",":/type/res/type/LeiZhenYu.png");
    mTypeMap.insert("雷阵雨伴有冰雹",":/type/res/type/LeiZhenYuBanYouBingBao.png");
    mTypeMap.insert("霾",":/type/res/type/Mai.png");
    mTypeMap.insert("强沙尘暴",":/type/res/type/QiangShaChenBao.png");
    mTypeMap.insert("晴",":/type/res/type/Qing.png");
    mTypeMap.insert("沙尘暴",":/type/res/type/ShaChenBao.png");
    mTypeMap.insert("特大暴雨",":/type/res/type/TeDaBaoYu.png");
    mTypeMap.insert("undefined",":/type/res/type/undefined.png");
    mTypeMap.insert("雾",":/type/res/type/Wu.png");
    mTypeMap.insert("小到中雪",":/type/res/type/XiaoDaoZhongXue.png");
    mTypeMap.insert("小到中雨",":/type/res/type/XiaoDaoZhongYu.png");
    mTypeMap.insert("小雪",":/type/res/type/XiaoXue.png");
    mTypeMap.insert("小雨",":/type/res/type/XiaoYu.png");
    mTypeMap.insert("阴",":/type/res/type/Yin.png");
    mTypeMap.insert("雨",":/type/res/type/Yu.png");
    mTypeMap.insert("雨夹雪",":/type/res/type/YuJiaXue.png");
    mTypeMap.insert("阵雨",":/type/res/type/ZhenYu.png");
    mTypeMap.insert("阵雪",":/type/res/type/ZhenXue.png");
    mTypeMap.insert("中到大雪",":/type/res/type/ZhongDaoDaXue.png");
    mTypeMap.insert("中到大雨",":/type/res/type/ZhongDaoDaYu.png");
    mTypeMap.insert("中雪",":/type/res/type/ZhongXue.png");
    mTypeMap.insert("中雨",":/type/res/type/ZhongYu.png");
    //网络请求
    manger = new QNetworkAccessManager(this);
    getweatherinfocity("海淀");//直接在构造中，请求天气数据
    connect(manger,&QNetworkAccessManager::finished,this,&MainWindow::onReplied);

      // 给标签添加事件过滤器 参数指定为this即当前的窗口
    ui->lblHighCurve->installEventFilter(this);
    ui->lblLowCurve->installEventFilter(this);

}

MainWindow::~MainWindow()
{
    delete ui;
}

//重写父类的虚函数
//父类中默认的实现 是忽略右键菜单事件
//重写之后，处理右键菜单事件
void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    qDebug()<<QCursor::pos();
    mExitMenu->exec(QCursor::pos());
    // 调用accept 表示，这个事件已经处理，不需要向上传递了
    event->accept();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    offset=event->globalPos()-this->pos();
    event->accept();
//    qDebug()<<offset;
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    this->move(event->globalPos()-offset);
//     qDebug()<<event->globalPos();
    event->accept();
}

void MainWindow::getweatherinfocity(QString cityName)
{
   QString cityCode =  WeatherTool::getCityCode(cityName);
   if(cityCode.isEmpty())
   {
        QMessageBox::warning(this,"天气","请检查是否输入正确！",QMessageBox::Ok);
   }
    QUrl url(tr("http://weather.itboy.net/api/weather/city/")+cityCode);
    manger->get(QNetworkRequest(url));
}

void MainWindow::onReplied(QNetworkReply *reply)
{
    qDebug()<<"on Replied success";
    int status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug()<<"operation: " << reply->operation();
    qDebug()<<"status_code: " << status_code;
    qDebug()<<"url: " << reply->url();
    qDebug()<<"raw header: " << reply->rawHeaderList();
    // 如果指定的城市编码不存在，就会报错：
    if(reply->error()!=QNetworkReply::NoError||status_code!=200)
    {
        qDebug()<<reply->errorString().toLatin1().data();
        QMessageBox::warning(this,"天气","请求数据失败",QMessageBox::Ok);
    }
    else
    {
    //获取相应信息
        QByteArray byteArray = reply->readAll();
        parseJson(byteArray);
        qDebug()<< "read all:" << byteArray.data();
    }
    reply->deleteLater();
}
     //数据的解析
void MainWindow::parseJson(QByteArray &byteArray)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(byteArray, &err);
    if(err.error!=QJsonParseError::NoError)
    {
        return;
    }
    QJsonObject rootobj = doc.object();
    qDebug() << rootobj.value("message").toString();

    // 1. 解析日期和城市
    mToday.date = rootobj.value("date").toString();
    mToday.city = rootobj.value("cityInfo").toObject().value("city").toString();
    // 2. 解析yesterday
    QJsonObject objData = rootobj.value("data").toObject();
    QJsonObject objYesterday = objData.value("yesterday").toObject();
    mDay[0].week = objYesterday.value("week").toString();
    mDay[0].date = objYesterday.value("ymd").toString();
    mDay[0].type = objYesterday.value("type").toString();
    QString s;
    s = objYesterday.value("high").toString().split(" ").at(1);
    s = s.left(s.length()-1);;
    mDay[0].high = s.toInt();

    s = objYesterday.value("low").toString().split(" ").at(1);
    s = s.left(s.length()-1);
    mDay[0].low = s.toInt();

    // 风向风力
    mDay[0].fx = objYesterday.value("fx").toString();
    mDay[0].fl = objYesterday.value("fl").toString();
    // 污染指数
    mDay[0].aqi = objYesterday.value("aqi").toDouble();
    // 3. 解析forcast中5天的数据
    QJsonArray forcastArr = objData.value("forecast").toArray();
    for (int i=0;i<5;++i)
    {
        QJsonObject objForecast = forcastArr[i].toObject();
        mDay[i+1].week = objForecast.value("week").toString();
        mDay[i+1].date = objForecast.value("ymd").toString();
        //天气类型
        mDay[i+1].type = objForecast.value("type").toString();
        QString s;
        // 高温低温
        s = objForecast.value("high").toString().split(" ").at(1);
        s = s.left(s.length()-1);
        mDay[i+1].high = s.toInt();

        s = objForecast.value("low").toString().split(" ").at(1);
        s = s.left(s.length()-1);
        mDay[i+1].low = s.toInt();

        // 风向风力
        mDay[i+1].fx = objForecast.value("fx").toString();
        mDay[i+1].fl = objForecast.value("fl").toString();
        // 污染指数
        mDay[i+1].aqi = objForecast.value("aqi").toDouble();

    }

    // 4. 解析今天的数据
    mToday.ganmao = objData.value("ganmao").toString();
    mToday.wendu = objData.value("wendu").toString().toInt();
    mToday.shidu = objData.value("shidu").toString();
    mToday.pm25 = objData.value("pm25").toDouble();
    mToday.quality = objData.value("quality").toString();

    // 5. forcast中第一个数组元素，也是今天的数据
    mToday.type = mDay[1].type;
    mToday.fx = mDay[1].fx;
    mToday.fl = mDay[1].fl;
    mToday.high = mDay[1].high;
    mToday.low = mDay[1].low;

    //6.更新ui
    updataUI();
}

void MainWindow::updataUI()
{
    //  1.更新日期和城市
    ui->lblDate->setText(QDateTime::fromString(mToday.date,"yyyyMMdd").toString("yyyy/MM/dd")+" "+mDay[1].week);
    ui->lblcity->setText(mToday.city);

    // 2.更新今天
    ui->lblTypeIcon->setPixmap(mTypeMap[mToday.type]);
    ui->lblPM25->setText(QString::number(mToday.pm25));
    ui->lblQuality->setText(mToday.quality);
    ui->lblShidu->setText(mToday.shidu);
    ui->lblWind->setText(mToday.fl);
    ui->lblWindTitle->setText(mToday.fx);
    ui->lblwendu->setText(QString::number(mToday.wendu)+"°");
    ui->lblType->setText(mToday.type);
    ui->lblLowHigh->setText(QString::number(mToday.low) + "~" + QString::number(mToday.high) + "℃");
    ui->lblganmao->setText("感冒指数："+ mToday.ganmao);
    // 3.更新六天
  for(int i=0; i<6; ++i)
  {
      // 3.1 更新日期和时间
      mDateList[i]->setText("周" + mDay[i].week.right(1));
      ui->lblDate1->setText("今天");
      ui->lblDate2->setText("明天");
      mWeekList[i]->setText(mDay[i].date.split("-").at(1)+"/"+mDay[i].date.split("-").at(2));

      // 3.2 更新天气类型
       mTypeList[i]->setText(mDay[i].type);
       mTypeIconList[i]-> setPixmap(mTypeMap[mDay[i].type]);
//      mAqiList[i]->setText(mDay[i].quality);

      if( mDay[i].aqi<=50)
      {
          mAqiList[i]->setText("优");
          mAqiList[i]->setStyleSheet("background-color:rgb(0, 185, 89);");
      }
      else if (mDay[i].aqi>50 && mDay[i].aqi<=100)
      {
          mAqiList[i]->setText("良");
          mAqiList[i]->setStyleSheet("background-color: rgb(255, 170, 0);");
      }
      else if (mDay[i].aqi>100 && mDay[i].aqi<=150)
      {
          mAqiList[i]->setText("轻度污染");
          mAqiList[i]->setStyleSheet("background-color: rgb(255, 85, 127);");
      }
      else if (mDay[i].aqi>150 && mDay[i].aqi<=200)
      {
          mAqiList[i]->setText("中度污染");
          mAqiList[i]->setStyleSheet("background-color: rgb(255, 0, 0);");
      }
      else if (mDay[i].aqi>200 && mDay[i].aqi<=300)
      {
          mAqiList[i]->setText("重度污染");
          mAqiList[i]->setStyleSheet("background-color: rgb(170, 0, 0);");
      }
      else
      {
          mAqiList[i]->setText("严重污染");
          mAqiList[i]->setStyleSheet("background-color: rgb(100, 0, 0);");
      }
      // 更新风力和风级
      mFlList[i]->setText(mDay[i].fl);
      mFxList[i]->setText(mDay[i].fx);

  }
     this->update();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->lblHighCurve&& event->type()==QEvent::Paint)
    {
        paintHighCurve();
    }
    if(watched == ui->lblLowCurve&& event->type()==QEvent::Paint)
    {
        paintLowCurve();
    }
    return QWidget::eventFilter(watched,event);
}

void MainWindow::paintHighCurve()
{
    QPainter painter(ui->lblHighCurve);
    //抗锯齿
    painter.setRenderHint(QPainter::Antialiasing,true);
    //1 获取x坐标
    int pointX[6] = {0};
    for(int i=0;i<6;++i)
    {
        pointX[i]= mAqiList[i]->pos().x() + mAqiList[i]->width()/2;
    }

    //2 获取y坐标
    int tempSum = 0;
    int tempAverage = 0;
    for(int i=0;i<6;i++)
    {
        tempSum+=mDay[i].high;
    }
    tempAverage = tempSum /6; // 最高温的平均值

    //3 计算y轴坐标
        int pointY[6] = {0};
        int yCenter = ui->lblHighCurve->height()/2;
        for(int i =0;i<6;++i)
        {
            pointY[i] = yCenter - ((mDay[i].high-tempAverage)*INCREMENT);

        }
   //4 开始绘制
        QPen pen = painter.pen();
        pen.setWidth(1);
        pen.setColor(QColor(255,170,0));
        painter.setPen(pen);
        painter.setBrush(QColor(255,170,0));//设置画刷的颜色-内部填充的颜色
        //画点、写文本
        for(int i=0;i<6;++i)
        {
            //显示点
            painter.drawEllipse(QPoint(pointX[i],pointY[i]), POINT_RADIUS, POINT_RADIUS);
            //显示温度文本
            painter.drawText(pointX[i]-TEXT_OFFSET_X,pointY[i]-TEXT_OFFSET_Y,QString::number(mDay[i].high)+"°");
        }
        // 绘制曲线
        for(int i=0;i<6;i++)
        {
            if(i==0)
            {
                pen.setStyle(Qt::DotLine);
                painter.setPen(pen);
            }
            else
            {
                pen.setStyle(Qt::SolidLine);
                painter.setPen(pen);
            }

            painter.drawLine(pointX[i],pointY[i],pointX[i+1],pointY[i+1]);
        }
        painter.restore();

}

void MainWindow::paintLowCurve()
{
    QPainter painter(ui->lblLowCurve);
    painter.setRenderHint(QPainter::Antialiasing,true);//抗锯齿
    //1 获取x坐标
    int pointX[6] ;
    for(int i=0;i<6;++i)
    {
        pointX[i]= mAqiList[i]->pos().x() +mAqiList[i]->width()/2;
    }
    //2 获取y坐标
        int tempSum = 0;
        int tempAverage = 0;
        for(int i=0;i<6;i++)
        {
            tempSum+=mDay[i].high;
        }
        tempAverage = tempSum /6; // 最高温的平均值

    //3 计算y轴坐标
        int pointY[6] = {0};
        int yCenter = ui->lblLowCurve->height()/2;
        for(int i =0;i<6;++i)
        {
            pointY[i] = yCenter - ((mDay[i].high-tempAverage)*INCREMENT);
        }
        // 开始绘制
        QPen pen = painter.pen();
        pen.setWidth(1);
        pen.setColor(QColor(0,255,255));
        painter.setPen(pen);
        painter.setBrush(QColor(0,255,255));//设置画刷的颜色-内部填充的颜色

        // 画点、写文本
        for(int i=0;i<6;++i)
        {
            //显示点
            painter.drawEllipse(QPoint(pointX[i],pointY[i]), POINT_RADIUS, POINT_RADIUS);
            //显示温度文本
            painter.drawText(pointX[i]-TEXT_OFFSET_X,pointY[i]-TEXT_OFFSET_Y,QString::number(mDay[i].high)+"°");
        }

        // 绘制曲线
        for(int i=0;i<5;i++)
        {
            if(i==0)
            {
                pen.setStyle(Qt::DotLine);
                painter.setPen(pen);
            }
            else
            {
                pen.setStyle(Qt::SolidLine);
                painter.setPen(pen);
            }

            painter.drawLine(pointX[i],pointY[i],pointX[i+1],pointY[i+1]);
        }
        painter.restore();

}


void MainWindow::on_toolButton_clicked()
{
      QString cityName = ui->lineEdit->text();
      getweatherinfocity(cityName);
}
