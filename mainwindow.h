#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QNetworkAccessManager>
#include <QMainWindow>
#include <QNetworkReply>
#include <QLabel>
#include "weatherdata.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    //获取天气数据、解析JSON、更新UI
    void getweatherinfocity(QString citycode);
    void onReplied(QNetworkReply* reply);
    void parseJson(QByteArray &byteArray);
    void updataUI();
    //重写父类的eventfileter方法
    bool eventFilter(QObject* watched,QEvent* event) override;
    //绘制高低温曲线
    void paintHighCurve();
    void paintLowCurve();


private slots:
    void on_toolButton_clicked();

private:
    Ui::MainWindow *ui;
    QMenu* mExitMenu; //右键退出的菜单
    QAction* mExitAct;//退出的行为 - 菜单项
    QPoint offset;
    QNetworkAccessManager* manger;
    QNetworkReply* reply;
    // 今天和6天的天气
    Today mToday;
    Day mDay[6];

    // 星期和日期
    QList<QLabel*> mWeekList;
    QList<QLabel*> mDateList;

    // 天气和天气图片
    QList<QLabel*> mTypeList;
    QList<QLabel*> mTypeIconList;

    // 天气污染指数
    QList<QLabel*> mAqiList;

    // 风力和风向
    QList<QLabel*> mFxList;
    QList<QLabel*> mFlList;
    // 由于在添加文件中，添加中文名称的图标，会报错，而请求回来的天气类型又都是中文
    //，如晴、小雨等。因此定义一个map，用于英文到中文的转换。以天气类型为key，以图标资源的路径为value
    QMap<QString,QString> mTypeMap;

};
#endif // MAINWINDOW_H
