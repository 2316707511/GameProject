#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QString>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QMessageBox>
#include <QProcess>
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//创房登录
void MainWindow::Req_Login(QString _opt)
{

    //1 获取用户名和密码
    QString username = ui->LE_UserName->text();
    QString password = ui->LE_PassWord->text();

    //2 对密码进行加密
    QCryptographicHash md5_hash(QCryptographicHash::Md5);
    md5_hash.addData(password.toUtf8());
    QString password_md5(md5_hash.result().toHex());

    //3 拼接登录请求串
    QJsonObject json;
    json["username"] = username;
    json["password"] = password_md5;
    json["opt"] = _opt;

    qDebug() << "username : " << username;
    qDebug() << "password : " << password_md5;
    qDebug() << "opt : " << _opt;

    //{"opt":"create_room","password":"c4ca4238a0b923820dcc509a6f75849b","username":"123"}

    QJsonDocument json_doc(json);
    QString output =  json_doc.toJson();

    //4 登录请求串发送给服务器
    QNetworkRequest req(QUrl("http://122.51.68.107/login/"));
    //QNetworkRequest req(QUrl("http://182.61.146.65/login/"));

    req.setHeader(QNetworkRequest::ContentLengthHeader , output.size());
    req.setHeader(QNetworkRequest::ContentTypeHeader , "application/json");
    m_pReply = m_NetMng.post(req , output.toUtf8());

    connect(m_pReply , SIGNAL(finished()) , this , SLOT(on_login_reply()));
}

//跟房请求
void MainWindow::Req_Login(QString _opt , QString _room_no)
{
    qDebug() << "follow";
    //1 获取用户名和密码
    QString username = ui->LE_UserName->text();
    QString password = ui->LE_PassWord->text();

    //2 对密码进行加密
    QCryptographicHash md5_hash(QCryptographicHash::Md5);
    md5_hash.addData(password.toUtf8());
    QString password_md5(md5_hash.result().toHex());

    //3 拼接登录请求串
    QJsonObject json;
    json["username"] = username;
    json["password"] = password_md5;
    json["opt"] = _opt;
    json["room_no"] = _room_no;

    //{"opt":"create_room","password":"c4ca4238a0b923820dcc509a6f75849b","username":"123"}

    QJsonDocument json_doc(json);
    QString output =  json_doc.toJson();

    //4 登录请求串发送给服务器
    QNetworkRequest req(QUrl("http://122.51.68.107/login/"));
    //QNetworkRequest req(QUrl("http://182.61.146.65/login/"));

    req.setHeader(QNetworkRequest::ContentLengthHeader , output.size());
    req.setHeader(QNetworkRequest::ContentTypeHeader , "application/json");
    m_pReply = m_NetMng.post(req , output.toUtf8());

    connect(m_pReply , SIGNAL(finished()) , this , SLOT(on_login_reply()));
}

//登录按钮
void MainWindow::on_Btn_Login_clicked()
{
    // 登录并且创建房间
    m_cur_opt = create_room;
    Req_Login("create_room");
}

//接受服务器回传的登录json对象
void MainWindow::on_login_reply()
{
    qDebug() << "reply ";
    QJsonDocument doc;
    doc = QJsonDocument::fromJson(m_pReply->readAll());

    QJsonObject json;
    json = doc.object();
    QString ret = json["login_result"].toString();

    if(ret == "success")
    {
        //创建房间
        if(m_cur_opt == create_room)
        {
            //调用游戏
            QString room_IP = json["room_IP"].toString();
            QString room_port = json["room_Port"].toString();
            QString room_no = json["room_no"].toString();

            qDebug() << "IP : " << room_IP ;
            qDebug() << "port : " << room_port ;
            qDebug() << "room no : " << room_no ;

            //消息盒子实现显示房间号
            QString Msg = "房间号 ： " + room_no;
            QMessageBox::information(this , "登录成功" , Msg);

            QStringList argv;
            argv.push_back("-h");
            argv.push_back(room_IP);
            argv.push_back("-p");
            argv.push_back(room_port);

            QProcess proc;
            proc.startDetached("Client.exe" ,  argv , ".");

            exit(0);
        }
        //进入房间
        else if(m_cur_opt == follow_room)
        {
            QString res = json["follow_result"].toString();
            if(res == "success")
            {
                //调用游戏
                QString room_no = ui->LE_RoomID->text();
                //消息盒子实现显示房间号
                QString Msg = "房间号 ： " + room_no;
                QMessageBox::information(this , "登录成功" , Msg);

                QString room_IP = json["room_IP"].toString();
                QString room_port = json["room_Port"].toString();

                qDebug() << room_IP << "  "<< room_port;

                QStringList argv;
                argv.push_back("-h");
                argv.push_back(room_IP);
                argv.push_back("-p");
                argv.push_back(room_port);

                //执行外部程序
                QProcess proc;
                proc.startDetached("Client.exe" ,  argv , ".");

                exit(0);
            }
            else
            {
                QMessageBox::information(this , "登录成功" , "跟房失败");
            }
        }
    }
    else
    {
        //登陆失败
        QMessageBox::information(this , "登录失败" , "用户名或密码错");
    }
}

//注册
void MainWindow::on_Btn_Reg_clicked()
{
    QDesktopServices::openUrl(QUrl("http://122.51.68.107"));
}

//进入房间
void MainWindow::on_Btn_Join_clicked()
{
    //跟随房间
    m_cur_opt = follow_room;
    QString room_no = ui->LE_RoomID->text();
    Req_Login("follow_room" , room_no);
}
